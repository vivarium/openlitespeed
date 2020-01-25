/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013 - 2018  LiteSpeed Technologies, Inc.                 *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#include "httpcgitool.h"

#include <http/httpdefs.h>
#include <http/httpextconnector.h>
#include <http/httpmethod.h>
#include <http/httpmime.h>
#include <http/httpreq.h>
#include <http/httpresp.h>
#include <http/httpserverversion.h>
#include <http/httpsession.h>
#include <http/httpstatuscode.h>
#include <http/httpver.h>
#include <http/ip2geo.h>
#include <http/iptoloc.h>
#include <http/clientinfo.h>
#include <http/httpserverconfig.h>
#include <http/requestvars.h>
#include <lsiapi/lsiapi_const.h>
#include <log4cxx/logger.h>
#include <lsr/ls_hash.h>
#include <lsr/ls_str.h>
#include <lsr/ls_strtool.h>
#include <sslpp/hiocrypto.h>
#include <sslpp/sslcert.h>
#include <sslpp/sslutil.h>
#include <util/autostr.h>
#include <util/datetime.h>
#include <util/ienv.h>
#include <util/radixtree.h>
#include <util/stringtool.h>

#include <extensions/fcgi/fcgienv.h>
#include <openssl/x509.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>


int HttpCgiTool::processContentType(HttpSession *pSession,
                                    const char *pValue, int valLen)
{
    HttpResp *pResp = pSession->getResp();
    HttpReq *pReq = pSession->getReq();
    const char *p;
    const AutoStr2 *pCharset = NULL;
    if (pReq->getStatusCode() == SC_304)
        return 0;

    if (HttpMime::needCharset(pValue))
    {
        pCharset = pReq->getDefaultCharset();
        if (pCharset && (p = (char *)memchr(pValue, ';', valLen)) != NULL)
        {
            while (isspace(*(++p)))
                ;
            if (strncmp(p, "charset=", 8) == 0)
            {
                pCharset = NULL;
            }
        }
    }
    return pResp->setContentTypeHeader(pValue, valLen, pCharset);
}


int HttpCgiTool::processHeaderLine(HttpExtConnector *pExtConn,
                                   const char *pName, int nameLen,
                                   const char *pValue, int valLen, int &status)
{
    HttpRespHeaders::INDEX index;

    index = HttpRespHeaders::getIndex(pName);
    if ((index < HttpRespHeaders::H_HEADER_END) &&
        (nameLen == HttpRespHeaders::getHeaderStringLen(index)))
    {
        return processHeaderLine(pExtConn, index, pName, nameLen, pValue,
                                 valLen, status);

    }
//    if (( pName < pValue )&&(pValue - (pName + nameLen) < 5 ))
//        return pExtConn->getHttpSession()->getResp()
//                    ->appendHeaderLine( pName, pValue + valLen );
//    else
    return pExtConn->getHttpSession()->getResp()->appendHeader(
                    pName, nameLen, pValue, valLen);
}


int HttpCgiTool::processHeaderLine(HttpExtConnector *pExtConn,
                                   const char  *pLineBegin,
                                   const char *pLineEnd, int &status)
{
    HttpRespHeaders::INDEX index;
    //int tmpIndex;
    const char *pKeyEnd = NULL;
    const char *pValue = pLineBegin;
    int nameLen = 0;

    index = HttpRespHeaders::getIndex(pValue);
    if (index < HttpRespHeaders::H_HEADER_END)
    {
        pValue += HttpRespHeaders::getHeaderStringLen(index);

        while ((pValue < pLineEnd) && isspace(*pValue))
            ++pValue;
        if (*pValue != ':')
            index = HttpRespHeaders::H_HEADER_END;
        else
        {
            nameLen = HttpRespHeaders::getHeaderStringLen(index);
            do { ++pValue; }
            while ((pValue < pLineEnd) && isspace(*pValue));
        }
    }
    if (index == HttpRespHeaders::H_HEADER_END)
    {
        pKeyEnd = (char *)memchr(pValue, ':', pLineEnd - pValue);
        if (pKeyEnd != NULL)
        {
            pValue = pKeyEnd + 1;
            while ((pValue < pLineEnd) && isspace(*pValue))
                ++pValue;
            while (isspace(pKeyEnd[-1]))
                --pKeyEnd;
            nameLen = pKeyEnd - pLineBegin;
            //ignore empty response header
            //if ( pValue == pLineEnd )
            //    return 0;
            if (pKeyEnd - pLineBegin <= 0)
                return 0;
        }
        else
        {
            if (!isspace(*pLineBegin))
                return 0;
        }
        if (status & HEC_RESP_AUTHORIZED)
        {
            if (strncasecmp(pLineBegin, "Variable-", 9) == 0)
            {
                if (pKeyEnd > pLineBegin + 9)
                    pExtConn->getHttpSession()->getReq()->addEnv(
                        pLineBegin + 9, pKeyEnd - pLineBegin - 9,
                        pValue, pLineEnd - pValue);
            }
            return 0;
        }
    }
    HttpReq *pReq = pExtConn->getHttpSession()->getReq();
    if (pExtConn->getHttpSession()->getState() == HSS_REDIRECT
        && pReq->getRedirHdrs() )
    {
        pReq->appendRedirHdr(pLineBegin, pLineEnd - pLineBegin + 2);
        return 0;
    }
    return processHeaderLine(pExtConn, index, pLineBegin, nameLen, pValue,
                             pLineEnd - pValue, status);
}


int HttpCgiTool::processHeaderLine(HttpExtConnector *pExtConn,
                                   int index, const char *pName, int nameLen,
                                   const char *pValue, int valLen, int &status)
{
    HttpResp *pResp = pExtConn->getHttpSession()->getResp();
    HttpReq *pReq = pExtConn->getHttpSession()->getReq();
    int tmpIndex;

    switch (index)
    {
    case HttpRespHeaders::H_CONTENT_TYPE:
        if (pReq->getStatusCode() == SC_304)
            return 0;
        //HttpCgiTool::processExpires(pReq, pResp, pValue);
        return processContentType(pExtConn->getHttpSession(), pValue, valLen);
    case HttpRespHeaders::H_CONTENT_ENCODING:
        if (pReq->getStatusCode() == SC_304
            || strncasecmp(pValue, "none", 4) == 0)
            return 0;
        if (strncasecmp(pValue, "gzip", 4) == 0)
            pReq->orGzip(UPSTREAM_GZIP);
        else if (strncasecmp(pValue, "deflate", 7) == 0)
            pReq->orGzip(UPSTREAM_DEFLATE);
//             if ( !(pReq->gzipAcceptable() & REQ_GZIP_ACCEPT) )
//                 return 0;
//         }
//         else //if ( strncasecmp( pValue, "deflate", 7 ) == 0 )
//         {
//             pReq->andGzip( ~GZIP_ENABLED );
//         }
        break;
    case HttpRespHeaders::H_CONTENT_DISPOSITION:
        pReq->appendRedirHdr(pName, pValue + valLen - pName + 2);
        break;
    case HttpRespHeaders::H_LOCATION:
        if ((status & HEC_RESP_PROXY) || (pReq->getStatusCode() != SC_200))
            break;
        //fall through
    case HttpRespHeaders::H_LITESPEED_LOCATION:
        if (*pValue != '/')
        {
            //set status code to 307
            pReq->setStatusCode(SC_302);
        }
        else
        {
            if ((pReq->getStatusCode() == SC_404) ||
                (index == HttpRespHeaders::H_LITESPEED_LOCATION))
                pReq->setStatusCode(SC_200);
            if (index == HttpRespHeaders::H_LITESPEED_LOCATION)
            {
                char ch = *(pValue + valLen);
                *((char *)(pValue + valLen)) = 0;
                pReq->locationToUrl(pValue, valLen);
                *((char *)(pValue + valLen)) = ch;
            }
            else
                pReq->setLocation(pValue, valLen);
            pExtConn->getHttpSession()->changeHandler();
            //status |= HEC_RESP_LOC_SET;
            return 0;
        }
        break;
    case HttpRespHeaders::H_CGI_STATUS:
        tmpIndex = HttpStatusCode::getInstance().codeToIndex(pValue);
        if (tmpIndex != -1)
        {
            pReq->updateNoRespBodyByStatus(tmpIndex);
            if ((tmpIndex >= SC_300) && (tmpIndex < SC_400))
            {
                if (pReq->getLocation() != NULL)
                {
                    pResp->appendHeader("location: ", 10,
                                        pReq->getLocation(), pReq->getLocationLen());
                    pReq->clearLocation();
                }
            }
            if ((status & HEC_RESP_AUTHORIZER) && (tmpIndex == SC_200))
                status |= HEC_RESP_AUTHORIZED;
        }
        return 0;
    case HttpRespHeaders::H_TRANSFER_ENCODING:
        pResp->setContentLen(LSI_RSP_BODY_SIZE_CHUNKED);
        return 0;
    case HttpRespHeaders::H_LINK:
        //pExtConn->getHttpSession()->processLinkHeader(pValue, pLineEnd - pValue);
        break;

    case HttpRespHeaders::H_SET_COOKIE:
        //pReq->getRespCacheCtrl().setHasCookie();
        pReq->processSetCookieHeader(pValue, valLen);
        break;

    case HttpRespHeaders::H_PROXY_CONNECTION:
    case HttpRespHeaders::H_CONNECTION:
        if (strncasecmp(pValue, "close", 5) == 0)
            status |= HEC_RESP_CONN_CLOSE;
        return 0;
    case HttpRespHeaders::H_CONTENT_LENGTH:
        if (pResp->getContentLen() == LSI_RSP_BODY_SIZE_UNKNOWN)
        {
            off_t lContentLen = strtoll(pValue, NULL, 10);
            if ((lContentLen >= 0) && (lContentLen != LLONG_MAX))
            {
                if (lContentLen > HttpServerConfig::getInstance().getMaxDynRespLen())
                {
                    pReq->setStatusCode(SC_413);
                    int len = pExtConn->getHttpSession()->createOverBodyLimitErrorPage();
                    pResp->setContentLen(len);
                    pExtConn->getHttpSession()->endResponse(0);
                    return LS_FAIL;
                }
                else
                    pResp->setContentLen(lContentLen);
                status |= HEC_RESP_CONT_LEN;
                pReq->orContextState(RESP_CONT_LEN_SET);
            }
        }
    //fall through
    case HttpRespHeaders::H_KEEP_ALIVE:
    case HttpRespHeaders::H_SERVER:
    case HttpRespHeaders::H_DATE:
        return 0;
    default:
        //"script-control: no-abort" is not supported

        if (11 == nameLen && 0 == strncasecmp(pName, "Lsrecaptcha", 11)
            && 1 == valLen && '1' == *pValue)
        {
            pExtConn->getHttpSession()->checkSuccessfulRecaptcha();
            return 0;
        }
        break;
    }
    if (index != HttpRespHeaders::H_HEADER_END)
        return pResp->appendHeader(index, pName, nameLen, pValue, valLen);
    else
    {
        return pResp->getRespHeaders().addWithUnknownHeader(pName, nameLen,
                                                            pValue, valLen,
                                                            LSI_HEADEROP_ADD);
    }
}


int HttpCgiTool::parseRespHeader(HttpExtConnector *pExtConn,
                                 const char *pBuf, int size, int &status)
{
    const char *pEnd = pBuf + size;
    const char *pLineEnd;
    const char *pLineBegin;
    const char *pCur = pBuf;
    const char *pValue;
    while (pCur < pEnd)
    {
        pLineBegin = pCur;
        pLineEnd = (const char *)memchr(pCur, '\n', pEnd - pCur);
        if (pLineEnd == NULL)
            break;
        pCur = pLineEnd + 1;
        while ((pLineEnd > pLineBegin) && (*(pLineEnd - 1) == '\r'))
            --pLineEnd;
        if (pLineEnd == pLineBegin)
        {
            //empty line detected
            status |= HttpReq::HEADER_OK;
            break;
        }
        pValue = pLineBegin;
        while ((pLineEnd > pValue) && (isspace(pLineEnd[-1])))
            --pLineEnd;
        if (pValue == pLineEnd)
            continue;
        int index;
        if ((*(pValue + 4) == '/') && memcmp(pValue, "HTTP/1.", 7) == 0)
        {
            index = HttpStatusCode::getInstance().codeToIndex(pValue + 9);
            if (index != -1)
            {
                pExtConn->getHttpSession()->getReq()->updateNoRespBodyByStatus(index);
                status |= HEC_RESP_NPH2;
                if ((status & HEC_RESP_AUTHORIZER) && (index == SC_200))
                    status |= HEC_RESP_AUTHORIZED;
            }
            continue;
        }
        if (processHeaderLine(pExtConn, pValue,
                              pLineEnd, status) == -1)
            return LS_FAIL;
    }
    return pCur - pBuf;
}


static char DEFAULT_PATH[] =
    "/bin:/usr/bin:/usr/ucb:/usr/bsd:/usr/local/bin";
static int DEFAULT_PATHLEN = sizeof(DEFAULT_PATH) - 1;
#define CGI_FWD_HEADERS 12
int HttpCgiTool::buildEnv(IEnv *pEnv, HttpSession *pSession)
{
    HttpReq *pReq = pSession->getReq();
    int n;
    pEnv->add("GATEWAY_INTERFACE", 17, "CGI/1.1", 7);
    if (getenv("PATH") == NULL)
        pEnv->add("PATH", 4, DEFAULT_PATH, DEFAULT_PATHLEN);
    n = pReq->getVersion();
    pEnv->add("SERVER_PROTOCOL", 15,
              HttpVer::getVersionString(n),
              HttpVer::getVersionStringLen(n));
    const char *pServerStr;
    pServerStr = HttpServerVersion::getVersion();
    n = HttpServerVersion::getVersionLen();
    pEnv->add("SERVER_SOFTWARE", 15, pServerStr, n);
    n = pReq->getMethod();
    pEnv->add("REQUEST_METHOD", 14, HttpMethod::get(n),
              HttpMethod::getLen(n));



//    //TODO: do nslookup
//
//    tmp = dnslookup(r->cn->peer.sin_addr, r->c->dns);
//    if (tmp) {
//            ADD_ENV(pEnv, "REMOTE_HOST", tmp);
//            free(tmp);
//    }
//
//    //ADD_ENV(pEnv, "REMOTE_HOST", achTemp );

    addSpecialEnv(pEnv, pReq);
    buildCommonEnv(pEnv, pSession);
    addHttpHeaderEnv(pEnv, pReq);
    pEnv->add(0, 0, 0, 0);
    return 0;
}


static char GISS_ENV[128];

static int GISS_ENV_LEN;
void HttpCgiTool::buildServerEnv()
{
    GISS_ENV_LEN = ls_snprintf(GISS_ENV, sizeof(GISS_ENV) - 1,
                               "\021\007GATEWAY_INTERFACECGI/1.1"
                               "\017%cSERVER_SOFTWARE",
                               HttpServerVersion::getVersionLen());
    memmove(&GISS_ENV[GISS_ENV_LEN], HttpServerVersion::getVersion(),
            HttpServerVersion::getVersionLen());
    GISS_ENV_LEN += HttpServerVersion::getVersionLen();
}


int HttpCgiTool::buildFcgiEnv(FcgiEnv *pEnv, HttpSession *pSession)
{
    static const char *SP_ENVs[] =
    {
        "\017\010SERVER_PROTOCOLHTTP/1.1",
        "\017\010SERVER_PROTOCOLHTTP/1.0",
        "\017\010SERVER_PROTOCOLHTTP/0.9"
    };
    static const char *RM_ENVs[10] =
    {
        "\016\007REQUEST_METHODUNKNOWN",
        "\016\007REQUEST_METHODOPTIONS",
        "\016\003REQUEST_METHODGET",
        "\016\004REQUEST_METHODHEAD",
        "\016\004REQUEST_METHODPOST",
        "\016\003REQUEST_METHODPUT",
        "\016\006REQUEST_METHODDELETE",
        "\016\005REQUEST_METHODTRACE",
        "\016\007REQUEST_METHODCONNECT",
        "\016\004REQUEST_METHODMOVE"

    };

    static int RM_ENV_LEN[10] =
    {   23, 23, 19, 20, 20, 19, 22, 21, 23, 20 };


    HttpReq *pReq = pSession->getReq();
    int n;

    pEnv->add(GISS_ENV, GISS_ENV_LEN);
    n = pReq->getVersion();
    pEnv->add(SP_ENVs[n], 25);
    n = pReq->getMethod();
    if (n < 10)
        pEnv->add(RM_ENVs[n], RM_ENV_LEN[n]);
    else
        pEnv->add("REQUEST_METHOD", 016, HttpMethod::get(n),
                  HttpMethod::getLen(n));

    addSpecialEnv(pEnv, pReq);
    buildCommonEnv(pEnv, pSession);
    addHttpHeaderEnv(pEnv, pReq);
    return 0;
}


int HttpCgiTool::addSpecialEnv(IEnv *pEnv, HttpReq *pReq)
{
    const AutoStr2 *psTemp = pReq->getRealPath();
    if (psTemp)
        pEnv->add("SCRIPT_FILENAME", 15, psTemp->c_str(), psTemp->len());
    pEnv->add("QUERY_STRING", 12, pReq->getQueryString(),
              pReq->getQueryStringLen());
    //const char * pTemp = pReq->getOrgURI();
    const char *pTemp = pReq->getURI();
    pEnv->add("SCRIPT_NAME", 11, pTemp, pReq->getScriptNameLen());
    return 0;
}


static int addEnv(void *pObj, void *pUData, const char *pKey, int iKeyLen)
{
    IEnv *pEnv = (IEnv *)pUData;
    ls_strpair_t *pPair = (ls_strpair_t *)pObj;
    pEnv->add(ls_str_cstr(&pPair->key), ls_str_len(&pPair->key),
              ls_str_cstr(&pPair->val), ls_str_len(&pPair->val));
    return 0;
}


int HttpCgiTool::buildCommonEnv(IEnv *pEnv, HttpSession *pSession)
{
    int count = 0;
    HttpReq *pReq = pSession->getReq();
    int isPython = pReq->isPythonContext();
    const char *pTemp;
    int n;
    char buf[128];
    RadixNode *pNode;

    pTemp = pReq->getAuthUser();
    if (pTemp && *pTemp)
    {
        //NOTE: only Basic is support now
        pEnv->add("AUTH_TYPE", 9, "Basic", 5);
        pEnv->add("REMOTE_USER", 11, pTemp, strlen(pTemp));
        count += 2;
    }
    //ADD_ENV("REMOTE_IDENT", "" )        //TODO: not supported yet
    //extensions of CGI/1.1
    const AutoStr2 *pStr = pReq->getDocRoot();
    if (!isPython)
        pEnv->add("DOCUMENT_ROOT", 13,
                  pStr->c_str(), pStr->len() - 1);


    pEnv->add("REMOTE_ADDR", 11, pSession->getPeerAddrString(),
              pSession->getPeerAddrStrLen());

    n = ls_snprintf(buf, 10, "%hu", pSession->getRemotePort());
    pEnv->add("REMOTE_PORT", 11, buf, n);

    n = pSession->getServerAddrStr(buf, 128);

    pEnv->add("SERVER_ADDR", 11, buf, n);

    pEnv->add("SERVER_NAME", 11, pReq->getHostStr(),  pReq->getHostStrLen());

    pStr = pReq->getVHost()->getAdminEmails();
    if (pStr->c_str())
    {
        pEnv->add("SERVER_ADMIN", 12, pStr->c_str(), pStr->len());
        ++count;
    }
    char *pBuf = buf;
    n = RequestVars::getReqVar(pSession, REF_SERVER_PORT, pBuf, 128);
    pEnv->add("SERVER_PORT", 11, pBuf, n);
    n = RequestVars::getReqVar(pSession, REF_REQ_SCHEME, pBuf, 128);
    pEnv->add("REQUEST_SCHEME", 14, pBuf, n);
    pEnv->add("REQUEST_URI", 11, pReq->getOrgReqURL(),
              pReq->getOrgReqURLLen());
    count += 8 - isPython;

    n = pReq->getPathInfoLen();
    if (n > 0)
    {
        pEnv->add("PATH_INFO", 9, pReq->getPathInfo(), n);
        ++count;
        if (!isPython)
        {
            int m;
            char achTranslated[10240];
            m =  pReq->translatePath(pReq->getPathInfo(), n,
                                    achTranslated, sizeof(achTranslated));
            if (m != -1)
            {
                pEnv->add("PATH_TRANSLATED", 15, achTranslated, m);
                ++count;
            }

            if (pReq->getRedirects() > 0)
            {
                pEnv->add("ORIG_PATH_INFO", 14, pReq->getPathInfo(), n);
                ++count;
            }
        }
    }

    if (!isPython)
    {
        if (pReq->getStatusCode() && (pReq->getStatusCode() != SC_200))
        {
            pTemp = HttpStatusCode::getInstance().getCodeString(pReq->getStatusCode());
            if (pTemp)
            {
                pEnv->add("REDIRECT_STATUS", 15, pTemp, 3);
                ++count;
            }
        }

        if (pReq->getRedirects() > 0)
        {
            pTemp = pReq->getRedirectURL(n);
            if (pTemp && (n > 0))
            {
                pEnv->add("REDIRECT_URL", 12, pTemp, n);
                ++count;
            }
            pTemp = pReq->getRedirectQS(n);
            if (pTemp && (n > 0))
            {
                pEnv->add("REDIRECT_QUERY_STRING", 21, pTemp, n);
                ++count;
            }
        }
    }

    //add geo IP env here
    if (pReq->isGeoIpOn())
    {
        GeoInfo *pInfo = pSession->getClientInfo()->getGeoInfo();
        if (pInfo)
        {
            pEnv->add("GEOIP_ADDR", 10, pSession->getPeerAddrString(),
                      pSession->getPeerAddrStrLen());
            count += pInfo->addGeoEnv(pEnv) + 1;
        }
    }
#ifdef USE_IP2LOCATION
    //add IP2Location env here
    if (pReq->isIpToLocOn())
    {
        LocInfo *pInfo = pSession->getClientInfo()->getLocInfo();
        if (pInfo)
        {
            pEnv->add("IP2LOCATION_ADDR", 16, pSession->getPeerAddrString(),
                      pSession->getPeerAddrStrLen());
            count += pInfo->addLocEnv(pEnv) + 1;
        }
    }
#endif
    n = pReq->getEnvCount();
    count += n;
    if ((pNode = (RadixNode *)pReq->getEnvNode()) != NULL)
        pNode->for_each2(addEnv, pEnv);


    if (pSession->getStream()->isSpdy())
    {
        const char *pProto = HioStream::getProtocolName((HiosProtocol)
                             pSession->getStream()->getProtocol());
        pEnv->add("X_SPDY", 6, pProto, strlen(pProto));
        ++count;
    }

    HioCrypto *pCrypto = pSession->getCrypto();
    if (pCrypto)
    {
        pBuf = buf;
        n = pCrypto->getEnv(HioCrypto::CRYPTO_VERSION, pBuf, 128);
        pEnv->add("SSL_PROTOCOL", 12, pBuf, n);
        ++count ;

        pBuf = buf;
        n = pCrypto->getEnv(HioCrypto::SESSION_ID, pBuf, 128);
        if (n > 0)
        {
            pEnv->add("SSL_SESSION_ID", 14, pBuf, n);
            ++count;
        }


        pBuf = buf;
        n = pCrypto->getEnv(HioCrypto::CIPHER, pBuf, 128);
        pEnv->add("SSL_CIPHER", 10, pBuf, n);

        pBuf = buf;
        n = pCrypto->getEnv(HioCrypto::CIPHER_USEKEYSIZE, pBuf, 128);
        pEnv->add("SSL_CIPHER_USEKEYSIZE", 21, pBuf, n);

        pBuf = buf;
        n = pCrypto->getEnv(HioCrypto::CIPHER_USEKEYSIZE, pBuf, 128);
        pEnv->add("SSL_CIPHER_ALGKEYSIZE", 21, pBuf, n);
        count += 3;

#ifdef HIOS_PROTO_QUIC
        char proto = pSession->getStream()->getProtocol();
        if (proto == HIOS_PROTO_QUIC && pCrypto)
        {
            pBuf = buf;
            n = pCrypto->getEnv(HioCrypto::TRANS_PROTOCOL_VERSION, pBuf, 128);
            pEnv->add("QUIC", 4, pBuf, n);
            ++count;
        }
#endif
        int i = pCrypto->getVerifyMode();
        if (i != 0)
        {
            char achBuf[4096];
            X509 *pClientCert = pCrypto->getPeerCertificate();
            if (pCrypto->isVerifyOk())
            {
                if (pClientCert)
                {
                    //IMPROVE: too many deep copy here.
                    //n = SslCert::PEMWriteCert( pClientCert, achBuf, 4096 );
                    //if ((n>0)&&( n <= 4096 ))
                    //{
                    //    pEnv->add( "SSL_CLIENT_CERT", 15, achBuf, n );
                    //    ++count;
                    //}
                    n = snprintf(achBuf, sizeof(achBuf), "%lu",
                                 X509_get_version(pClientCert) + 1);
                    pEnv->add("SSL_CLIENT_M_VERSION", 20, achBuf, n);
                    ++count;
                    n = SslUtil::lookupCertSerial(pClientCert, achBuf, 4096);
                    if (n != -1)
                    {
                        pEnv->add("SSL_CLIENT_M_SERIAL", 19, achBuf, n);
                        ++count;
                    }
                    X509_NAME_oneline(X509_get_subject_name(pClientCert), achBuf, 4096);
                    pEnv->add("SSL_CLIENT_S_DN", 15, achBuf, strlen(achBuf));
                    ++count;
                    X509_NAME_oneline(X509_get_issuer_name(pClientCert), achBuf, 4096);
                    pEnv->add("SSL_CLIENT_I_DN", 15, achBuf, strlen(achBuf));
                    ++count;
                    if (SslConnection::isClientVerifyOptional(i))
                    {
                        lstrncpy(achBuf, "GENEROUS", sizeof(achBuf));
                        n = 8;
                    }
                    else
                    {
                        lstrncpy(achBuf, "SUCCESS", sizeof(achBuf));
                        n = 7;
                    }
                }
                else
                {
                    lstrncpy(achBuf, "NONE", sizeof(achBuf));
                    n = 4;
                }
            }
            else
                n = pCrypto->buildVerifyErrorString(achBuf, sizeof(achBuf));
            pEnv->add("SSL_CLIENT_VERIFY", 17, achBuf, n);
            ++count;
        }
    }

    char sVer[40];
    n = snprintf(sVer, 40, "Openlitespeed %s", PACKAGE_VERSION);
    pEnv->add("LSWS_EDITION", 12, sVer, n);
    pEnv->add("X-LSCACHE", 9, "on,crawler", 10);
    count += 2;

    return count;
}


int HttpCgiTool::addHttpHeaderEnv(IEnv *pEnv, HttpReq *pReq)
{
    int i, n;
    const char *pTemp;
    LsiApiConst lsiApiConst;
    int headers = lsiApiConst.get_cgi_header_count();
    for (i = 0; i < headers; ++i)
    {
        pTemp = pReq->getHeader(i);
        if (*pTemp)
        {
            //Note: web server does not send authorization info to cgi for
            // security reason
            //pass AUTHORIZATION header only when server does not check it.
            if ((i == HttpHeader::H_AUTHORIZATION)
                && (pReq->getAuthUser()))
                continue;
            pEnv->add(lsiApiConst.get_cgi_header(i),
                      lsiApiConst.get_cgi_header_len(i),
                      pTemp, pReq->getHeaderLen(i));
        }
    }

    n = pReq->getUnknownHeaderCount();
    for (i = 0; i < n; ++i)
    {
        const char *pKey;
        const char *pVal;
        int keyLen;
        int valLen;
        pKey = pReq->getUnknownHeaderByIndex(i, keyLen, pVal, valLen);
        if (pKey)
        {
            char *p;
            const char *pKeyEnd = pKey + keyLen;
            char achHeaderName[256];
            memcpy(achHeaderName, "HTTP_", 5);
            p = &achHeaderName[5];
            if (keyLen > 250)
                keyLen = 250;
            while (pKey < pKeyEnd)
            {
                char ch = *pKey++;
                if (ch == '-')
                    *p++ = '_';
                else
                    *p++ = toupper(ch);
            }
            keyLen += 5;
            pEnv->add(achHeaderName, keyLen, pVal, valLen);
        }
    }
    return 0;
}

int HttpCgiTool::processExpires(HttpReq *pReq, HttpResp *pResp, const char *pValue)
{
    HttpContext *pContext = &(pReq->getVHost()->getRootContext());
    const MimeSetting *pMIMESetting = pContext->lookupMimeSetting((char *)pValue);
    int enbale = pContext->getExpires().isEnabled();
    if (enbale)
    {
        ExpiresCtrl *pExpireDefault = NULL;//&pContext->getExpires();
        if (pMIMESetting)
            pExpireDefault = (ExpiresCtrl *)pMIMESetting->getExpires();

        if (pExpireDefault == NULL)
            pExpireDefault = &pContext->getExpires();

        if (pExpireDefault->getBase())
            pResp->addExpiresHeader(DateTime::s_curTime, pExpireDefault);
    }

    return 0;
}


