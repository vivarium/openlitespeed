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
#ifndef CACHEENTRY_H
#define CACHEENTRY_H

#include <lsdef.h>
#include <modules/cache/ceheader.h>
#include <modules/cache/cachehash.h>
#include <util/autostr.h>
#include <util/refcounter.h>

#define DateTime_s_curTime  ( DateTime::s_curTime )
#define DateTime_s_curTimeMs  ( DateTime::s_curTimeUs / 1000 )

//#define DateTime_s_curTime  ( time(NULL) )

//#define CACHE_RESP_HEADER   1

#define CE_UPDATING     (1<<0)
#define CE_STALE        (1<<1)

class DLinkedObj;
class DLinkQueue;
class HttpRespHeaders;

struct CacheKey
{
    const char     *m_pUri;
    int             m_iUriLen;
    const char     *m_pQs;
    int             m_iQsLen;
    const char     *m_pIP;
    int             m_ipLen;
    AutoStr2        m_sCookie;
    int             m_iCookieVary;
    int             m_iCookiePrivate;

    int getPrivateId(char *pBuf, char *pBufEnd);
    int isPrivate() const
    {   return m_pIP != NULL;   }
};


class CacheEntry : public RefCounter
{
public:
    CacheEntry();

    virtual ~CacheEntry();

    void setLastAccess(long tm)   {   m_lastAccess = tm;  }
    long getLastAccess() const      {   return m_lastAccess;    }

    void setLastPurgrCheck(long tm);
    long getLastPurgrCheck() const  {   return m_lastPurgrCheck;}
    
    void incHits()                  {   ++m_iHits;          }
    void clearHits()                {   m_iHits = 0;        }
    long getHits() const            {   return m_iHits;     }

    
    void setDirty()                 {   m_isDirty = 1;          }
    void setBuilding(int v)         {   m_isBuilding = (v != 0);}
    int  isDirty() const            {   return m_isDirty;       }
    int  isBuilding() const         {   return m_isBuilding;    }

//     void incTestHits()              {   ++m_iTestHits;    }
//     long getTestHits() const        {   return m_iTestHits;    }

    void setFdStore(int fd)         {   m_fdStore = fd;  }
    int getFdStore() const          {   return m_fdStore; }

    void setStartOffset(off_t off) {   m_startOffset = off;    }
    off_t getStartOffset() const    {   return m_startOffset;   }

    void setMaxStale(int age)     {   m_iMaxStale = age;      }
    int  getMaxStale() const        {   return m_iMaxStale;     }

    off_t getHeaderSize() const
    {
        return CACHE_ENTRY_MAGIC_LEN + sizeof(CeHeader)
               + m_header.m_keyLen + m_header.m_tagLen;
    }

    void setPart1Len(int len)
    {   m_header.m_valPart1Len = len;   }
    void setPart2Len(int len)
    {   m_header.m_valPart2Len = len;   }

    int getContentTotalLen() const
    {   return m_header.m_valPart1Len + m_header.m_valPart2Len; }

    int getPart1Len() const
    {   return m_header.m_valPart1Len;      }
    int32_t getPart2Len() const
    {   return m_header.m_valPart2Len;      }

    virtual int loadCeHeader() = 0 ;
    virtual int saveCeHeader() = 0 ;
    virtual int allocate(int size) = 0;
    virtual int releaseTmpResource() = 0;
    virtual int saveRespHeaders(HttpRespHeaders *pHeader) = 0;


    int setKey(const CacheHash &hash, CacheKey *pKey);

    int verifyKey(CacheKey *pKey) const;

    int isUnderConstruct() const
    {   return m_header.m_flag & CeHeader::CEH_IN_CONSTRUCT;    }

    void appendToWaitQ(DLinkedObj *pObj);
    DLinkQueue *getWaitQ() const    {   return m_pWaitQue;      }

    void markReady(int compressed_method)
    {
        m_header.m_flag = (m_header.m_flag & ~CeHeader::CEH_IN_CONSTRUCT);
        m_header.m_flag &= (~CeHeader::CEH_BR & ~CeHeader::CEH_GZIP);
        
        if (compressed_method == 2)
            m_header.m_flag |= CeHeader::CEH_BR;
        else if (compressed_method == 1)
            m_header.m_flag |= CeHeader::CEH_GZIP;
    }

    //Return 0, no compressed, 1 Gzip, 2 Br
    int getCompressType() const
    {
        if (m_header.m_flag & CeHeader::CEH_BR)
            return 2;
        else if (m_header.m_flag & CeHeader::CEH_GZIP)
            return 1;
        else
            return 0;
    }
    

    int isUpdating() const
    {   return m_header.m_flag & CeHeader::CEH_UPDATING;    }
    void setUpdating(int i)
    {   setFlag(CeHeader::CEH_UPDATING, i);  }

    int isStale() const
    {   return m_header.m_flag & CeHeader::CEH_STALE;       }
    void setStale(int i)
    {   setFlag(CeHeader::CEH_STALE, i);  }

    int isPrivate() const
    {   return m_header.m_flag & CeHeader::CEH_PRIVATE;     }

    CeHeader &getHeader()               {   return m_header;            }
    AutoStr  &getKey()                  {   return m_sKey;              }
    int       getKeyLen()               {   return m_header.m_keyLen;   }
    const CacheHash &getHashKey() const {   return m_hashKey;           }

    void setHashKey(const CacheHash &hash)  {   m_hashKey.copy(hash);     }

    int getPart1Offset() const          {   return m_startOffset + getHeaderSize();   }

    int getPart2Offset() const
    {   return m_startOffset + getHeaderSize() + m_header.m_valPart1Len;   }

    time_t getExpireTime() const        {   return m_header.m_tmExpire; }

    void setFlag(int flag, int val)
    {   m_header.m_flag = ((m_header.m_flag & ~flag) | ((val) ? flag : 0));  }

    AutoStr &getTag()                   {   return m_sTag;      }
    void setTag(const char *pTag, int len);

    int  isOlderThan(int32_t tmLast, int32_t iMsecLast)
    {
        return ((m_header.m_tmCreated < tmLast)
                || ((m_header.m_tmCreated == tmLast)
                    && (m_header.m_msCreated < iMsecLast)));
    }

    void setNeedDelay(int v) {   m_needDelay = v;    }
    int  getNeedDelay() {   return m_needDelay;    }

    int getVaryIndexSet(int index)   {  return m_iVaryFlag & (1 << index);   }
    void setVaryIndex(int index)     { m_iVaryFlag |= (1 << index);   }


private:
    long        m_lastAccess;
    long        m_lastPurgrCheck;
   
    int         m_iMaxStale;
    
    /**
     * When this reach 10, then means currrent cache need to change gzip/ungzip
     */
    uint32_t    m_iHits:29;
    uint32_t    m_isDirty:1;
    uint32_t    m_isBuilding:1;
    
    int         m_needDelay; //delay serving if have cache, in URI_MAP instead of recv req header */
    CacheHash   m_hashKey;

    off_t       m_startOffset;
    CeHeader    m_header;
    int         m_fdStore;
    int32_t     m_iVaryFlag;  //each bit indicate a vary req header
    AutoStr     m_sKey;

    AutoStr     m_sTag;
    DLinkQueue *m_pWaitQue;
    LS_NO_COPY_ASSIGN(CacheEntry);
};

#endif
