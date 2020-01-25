// Author: Kevin Fwu
// Date: August 6, 2015


#include "sslticket.h"

#include <lsdef.h>
#include <log4cxx/logger.h>
#include <shm/lsshmhash.h>
#include <util/datetime.h>
#include <util/ni_fio.h>

#include <fcntl.h>
#include <string.h>

#if __cplusplus <= 199711L && !defined(static_assert)
#define static_assert(a, b) _Static_assert(a, b)
#endif

#include <openssl/rand.h>
#include <openssl/ssl.h>

#define SSLTICKET_KEYSIZE 48 // name + aes + hmac
#define SSLTICKET_FILESIZE 48

static const char *s_pSSLShmName = "SSL";
static const char *s_pSTShmHashName = "SSLTicket";
static const char *s_pSTTickets = "SSLTickets";

typedef struct STShmData_s
{
    RotateKeys_t m_keys;
    long      m_tmLastAccess;
} STShmData_t;


static void LOGDBG(const char* format, ...)
{
    char achNewFmt[1024];
    va_list arglist;
    va_start(arglist, format);
    snprintf(achNewFmt, 1023, "[SSLTicket] %s", format);
    LOG4CXX_NS::Logger::s_vlog(LOG4CXX_NS::Level::DBG_LESS, NULL, achNewFmt,
                               arglist, 0);
    va_end(arglist);
}


static void LOGERR(const char* format, ...)
{
    char achNewFmt[1024];
    va_list arglist;
    va_start(arglist, format);
    snprintf(achNewFmt, 1023, "[SSLTicket] %s", format);
    LOG4CXX_NS::Logger::s_vlog(LOG4CXX_NS::Level::ERROR, NULL, achNewFmt,
                               arglist, 0);
    va_end(arglist);
}


// #include <util/stringtool.h>
// static void printKey(const char *prefix, STKey_t *pKey)
// {
//     char name[0x100], aes[0x100], hmac[0x100];
//     int namelen, aeslen, hmaclen;
//     namelen = StringTool::hexEncode((const char *)pKey->aName, 16, name);
//     aeslen = StringTool::hexEncode((const char *)pKey->aAes, 16, aes);
//     hmaclen = StringTool::hexEncode((const char *)pKey->aHmac, 16, hmac);
//     LOGDBG("%s Name %.*s, Aes %.*s, Hmac %.*s, expire %ld",
//            prefix, namelen, name, aeslen, aes, hmaclen, hmac, pKey->expireSec);
// }


static void rotateIndices(short &idxPrev, short &idxCur, short &idxNext)
{
    short idxTmp = idxPrev;
    idxPrev = idxCur;
    idxCur = idxNext;
    idxNext = idxTmp;
}


/**
 * loadKeyFromFile() takes the filename and (optional) stat structure and
 * does all file io related code to extract the key from the file and stores
 * it in pData.
 */
static int loadKeyFromFile(const char *pFileName, STKey_t *pData,
                           struct stat *pStat = NULL)
{
    int fd;
    struct stat st;
    if (pStat == NULL)
    {
        pStat = &st;
        if (stat(pFileName, pStat) == -1)
        {
            LOGERR("Stat file name '%s'failed! error %s",
                   pFileName, strerror(errno));
            return LS_FAIL;
        }
    }
    if (pStat->st_size != SSLTICKET_FILESIZE)
    {
        LOGERR("init file failed!  Incorrect number of bytes, %d",
               pStat->st_size);
        return LS_FAIL;
    }
    if ((fd = nio_open(pFileName, O_RDONLY, 0644)) < 0)
    {
        LOGERR("Open file failed!  File name '%s', error %s",
               pFileName, strerror(errno));
        return LS_FAIL;
    }
    if (nio_read(fd, pData, SSLTICKET_FILESIZE) != SSLTICKET_FILESIZE)
    {
        LOGERR("Read file failed!  File name '%s', error %s",
               pFileName, strerror(errno));
        nio_close(fd);
        return LS_FAIL;
    }
    nio_close(fd);
    return LS_OK;
}


LsShmHash      *SslTicket::m_pKeyStore = NULL;
AutoStr2       *SslTicket::m_pFile     = NULL;
LsShmOffset_t   SslTicket::m_iOff      = 0;
RotateKeys_t    SslTicket::m_keys;
long            SslTicket::m_iLifetime = 0;



int SslTicket::initShm(int uid, int gid)
{
    LsShm *pShm;
    LsShmPool *pShmPool;

    if (m_pKeyStore != NULL)
    {
        LOGDBG("Hash Already Loaded.");
        return LS_FAIL;
    }
    if ((pShm = LsShm::open(s_pSSLShmName, 0)) == NULL)
    {
        LOGDBG("Open LsShm Failed.");
        return LS_FAIL;
    }
    pShm->chperm(uid, gid, 0600);   
    
    if ((pShmPool = pShm->getGlobalPool()) == NULL)
    {
        LOGDBG("Get Global Pool Failed.");
        return LS_FAIL;
    }
    if ((m_pKeyStore = pShmPool->getNamedHash(s_pSTShmHashName, 4,
                                        LsShmHash::hashXXH32, memcmp,
                                        LSSHM_FLAG_NONE )) == NULL)
    {
        LOGDBG("Get Hash Failed.");
        return LS_FAIL;
    }
    m_pKeyStore->disableAutoLock();
    return LS_OK;
}


int SslTicket::init(const char* pFileName, long int timeout, int uid, int gid)
{
    STShmData_t *pShmData;
    int iValLen;
    STKey_t newKey, *pCur;

    if (initShm(uid, gid) == LS_FAIL)
        return LS_FAIL;
    if (pFileName != NULL)
        m_pFile = new AutoStr2(pFileName);
    m_iLifetime = timeout;
    m_pKeyStore->lock();
    if ((m_iOff = m_pKeyStore->find(s_pSTTickets, strlen(s_pSTTickets),
                                    &iValLen)) == 0)
    {
        //Did not find in shm
        if (pFileName == NULL)
        {
            RAND_bytes((unsigned char *)&m_keys.m_aKeys[0], SSLTICKET_KEYSIZE);
            RAND_bytes((unsigned char *)&m_keys.m_aKeys[1], SSLTICKET_KEYSIZE);
            m_keys.m_aKeys[0].expireSec = DateTime::s_curTime + m_iLifetime;
            m_keys.m_aKeys[1].expireSec = m_keys.m_aKeys[0].expireSec + (m_iLifetime >> 1);
            m_keys.m_aKeys[SSLTICKET_NUMKEYS - 1].expireSec = 0;
            m_keys.m_idxCur = 0;
            m_keys.m_idxNext = 1;
            m_keys.m_idxPrev = SSLTICKET_NUMKEYS - 1;
        }
        else
        {
            if (loadKeyFromFile(pFileName, &m_keys.m_aKeys[0]) == LS_FAIL)
            {
                m_pKeyStore->unlock();
                return LS_FAIL;
            }
            m_keys.m_aKeys[0].expireSec = DateTime::s_curTime + m_iLifetime;
            m_keys.m_idxCur = 0;
            m_keys.m_idxNext = 1;
            m_keys.m_idxPrev = SSLTICKET_NUMKEYS - 1;
        }

        if ((m_iOff = m_pKeyStore->insert(s_pSTTickets, strlen(s_pSTTickets),
                                          NULL, sizeof(STShmData_t))) == 0)
        {
            LOGERR("Failed to insert shm data.");
            m_pKeyStore->unlock();
            return LS_FAIL;
        }
        pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
        memmove(&pShmData->m_keys, &m_keys, sizeof(m_keys) ); 
        pShmData->m_tmLastAccess = DateTime::s_curTime;
        m_pKeyStore->unlock();
        return LS_OK;
    }
    pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
    memmove(&m_keys, &pShmData->m_keys, sizeof(m_keys) ); 
    m_pKeyStore->unlock();
    pCur = &m_keys.m_aKeys[m_keys.m_idxCur];
    if (pCur->expireSec > (DateTime::s_curTime + (timeout >> 1)))
        return LS_OK;

    if (pFileName == NULL)
    {
        m_pKeyStore->lock();
        pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
        checkShmExpire(pShmData);
        memmove(&m_keys, &pShmData->m_keys, sizeof(m_keys) ); 
        m_pKeyStore->unlock();
        return LS_OK;
    }

    if (loadKeyFromFile(pFileName, &newKey) == LS_FAIL)
    {
        LOGDBG("Load key from file failed.");
        return LS_FAIL;
    }

    if (memcmp(&newKey, pCur, SSLTICKET_KEYSIZE) == 0)
    {
        m_pKeyStore->lock();
        pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
    }
    else if (pCur->expireSec < DateTime::s_curTime)
    {
        memmove(pCur, &newKey, SSLTICKET_KEYSIZE);
        m_pKeyStore->lock();
        pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
        memmove(&pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxCur], &newKey,
                SSLTICKET_KEYSIZE);
    }
    else
    {
        rotateIndices(m_keys.m_idxPrev, m_keys.m_idxCur, m_keys.m_idxNext);
        memmove(&m_keys.m_aKeys[m_keys.m_idxCur], &newKey, SSLTICKET_KEYSIZE);
        m_pKeyStore->lock();
        pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
        memmove(&pShmData->m_keys, &m_keys, sizeof(m_keys) ); 
    }
    m_keys.m_aKeys[m_keys.m_idxCur].expireSec = DateTime::s_curTime + timeout;
    pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxCur].expireSec
                                    = m_keys.m_aKeys[m_keys.m_idxCur].expireSec;
    pShmData->m_tmLastAccess = DateTime::s_curTime;
    m_pKeyStore->unlock();
    return LS_OK;
}


/**
 * onTimer() checks for expired keys every 30 seconds.
 */
int SslTicket::onTimer()
{
    STShmData_t *pShmData;
    STKey_t *pCur, *pPrev, *pNext;
    if (m_pKeyStore == NULL)
        return LS_OK;
    m_pKeyStore->lock();
    pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
    pPrev = &pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxPrev];
    pCur = &pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxCur];
    pNext = &pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxNext];
    if (pCur->expireSec > (DateTime::s_curTime + (m_iLifetime >> 1)))
    {
//         LOGDBG("Not expired");
        m_pKeyStore->unlock();
        return 0; // Not expired.
    }
    if (m_pFile != NULL)
    {
        struct stat st;
        STKey_t newKey;

        if (stat(m_pFile->c_str(), &st) != 0)
        {
            LOGERR("Stat failed");
            m_pKeyStore->unlock();
            return LS_FAIL;
        }
        if (st.st_mtime < pShmData->m_tmLastAccess)
        {
            pCur->expireSec += (m_iLifetime >> 1);
            LOGDBG("File was not modified");
            m_pKeyStore->unlock();
            return LS_OK;
        }
        if (loadKeyFromFile(m_pFile->c_str(), &newKey, &st) == LS_FAIL)
        {
            LOGDBG("Load key from file failed.");
            m_pKeyStore->unlock();
            return LS_FAIL;
        }
        pShmData->m_tmLastAccess = DateTime::s_curTime;
        if (memcmp(&newKey, pCur, SSLTICKET_KEYSIZE) == 0)
        {
            pCur->expireSec += (m_iLifetime >> 1);
            m_pKeyStore->unlock();
            return LS_OK;
        }
        memmove(pNext, &newKey, SSLTICKET_KEYSIZE);
        pNext->expireSec = pCur->expireSec + (m_iLifetime >> 1);
    }
    else
    {
//         LOGDBG("Rotate!");
        RAND_bytes((unsigned char *)pPrev, SSLTICKET_KEYSIZE);
        pPrev->expireSec = pCur->expireSec + m_iLifetime;
    }
    rotateIndices(pShmData->m_keys.m_idxPrev, pShmData->m_keys.m_idxCur, pShmData->m_keys.m_idxNext);
    m_pKeyStore->unlock();
    return LS_OK;
}


/**
 * checkShmExpire() is called when we know that the keys in shm are expired
 * or in need to be renewed.  Based on certain conditions, it will update
 * up to two keys in shm and set a current expire time.
 * NOTICE: When calling this function, must have lock!
 */
int SslTicket::checkShmExpire(STShmData_t *pShmData)
{
    STKey_t *pKeys = pShmData->m_keys.m_aKeys;
    STKey_t *pPrev = &pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxPrev];
    STKey_t *pCur = &pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxCur];
    STKey_t *pNext = &pShmData->m_keys.m_aKeys[pShmData->m_keys.m_idxNext];

    if (pNext->expireSec < DateTime::s_curTime)
    {
//         LOGDBG("All Keys Expired");
        // The expire makes this key not work.  Prev is just a placeholder.
        pShmData->m_keys.m_idxPrev = SSLTICKET_NUMKEYS - 1;
        pShmData->m_keys.m_idxCur = 0;
        pShmData->m_keys.m_idxNext = 1;
        RAND_bytes((unsigned char *)&pKeys[0], SSLTICKET_KEYSIZE);
        RAND_bytes((unsigned char *)&pKeys[1], SSLTICKET_KEYSIZE);
        pKeys[0].expireSec = DateTime::s_curTime + m_iLifetime;
        pKeys[1].expireSec = pKeys[0].expireSec + (m_iLifetime >> 1);
        return LS_OK;
    }
    else if (pCur->expireSec < DateTime::s_curTime)
    {
//         LOGDBG("cur and prev Expired");
        RAND_bytes((unsigned char *)pNext, SSLTICKET_KEYSIZE);
        pNext->expireSec = pCur->expireSec + (m_iLifetime >> 1);
    }
    else if (pPrev->expireSec >= DateTime::s_curTime)
        return LS_OK;
    // else all but previous key are valid.

    RAND_bytes((unsigned char *)pPrev, SSLTICKET_KEYSIZE);
    pPrev->expireSec = pNext->expireSec + (m_iLifetime >> 1);
    rotateIndices(pShmData->m_keys.m_idxPrev, pShmData->m_keys.m_idxCur, 
                  pShmData->m_keys.m_idxNext);
    return LS_OK;
}


/**
 * ticketCb() is the ticket callback.  When called, if enc is 1,
 * that means I need to create a ticket for the request.  If enc is 0,
 * I need to check the ticket to make sure it is still valid.
 */
int SslTicket::ticketCb(SSL *pSSL, unsigned char aName[16], unsigned char *iv,
                    EVP_CIPHER_CTX *ectx, HMAC_CTX *hctx, int enc)
{
    int ret;
    STShmData_t *pShmData;
    STKey_t *pSessKey = &m_keys.m_aKeys[m_keys.m_idxCur];

    if (pSessKey->expireSec < (DateTime::s_curTime + (m_iLifetime >> 1)))
    {

        // Current key needs renewal.  update current version to shm version.
        // If current key is expired, shm version should be updated.
        // if current key needs renewal, shm version might not be updated yet,
        // but current key still works.
        m_pKeyStore->lock();
        pShmData = (STShmData_t *)m_pKeyStore->offset2ptr(m_iOff);
        memmove(&m_keys, &pShmData->m_keys,
                (sizeof(STKey_t) + sizeof(short)) * SSLTICKET_NUMKEYS);
        m_pKeyStore->unlock();
        pSessKey = &m_keys.m_aKeys[m_keys.m_idxCur];
    }

    if (enc == 1)
    {
        if ((ret = RAND_bytes(iv, EVP_MAX_IV_LENGTH)) == -1)
        {
            LOGDBG("RAND_bytes not supported.");
            return -1;
        }
        else if (ret == 0)
        {
            LOGDBG("RAND_bytes failed.");
            return -1;
        }

        if (pSessKey->expireSec < DateTime::s_curTime)
        {
            LOGERR("ERROR: Current Key already expired!");
            return -1;
        }

        EVP_EncryptInit_ex(ectx, EVP_aes_128_cbc(), NULL, pSessKey->aAes, iv);
        HMAC_Init_ex(hctx, pSessKey->aHmac, 16, EVP_sha256(), NULL);
        memmove(aName, pSessKey->aName, 16);
    }
    else
    {
        if (memcmp(aName, pSessKey->aName, 16) != 0)
        {
            pSessKey = &m_keys.m_aKeys[m_keys.m_idxPrev];
            if (memcmp(aName, pSessKey->aName, 16) != 0)
                return 0;
        }

        if (pSessKey->expireSec < DateTime::s_curTime)
            return 0;

        HMAC_Init_ex(hctx, pSessKey->aHmac, 16, EVP_sha256(), NULL);
        EVP_DecryptInit_ex(ectx, EVP_aes_128_cbc(), NULL, pSessKey->aAes, iv);

        if (pSessKey != &m_keys.m_aKeys[m_keys.m_idxCur])
            return 2;
    }
    return 1;
}


