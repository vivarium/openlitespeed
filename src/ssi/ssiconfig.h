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
#ifndef SSICONFIG_H
#define SSICONFIG_H


#include <lsdef.h>
#include <util/autostr.h>

#define SSI_BIT_LASTMOD_ON   1
#define SSI_BIT_ETAG_ON      2


class SsiTagConfig
{
public:
    SsiTagConfig() {}

    ~SsiTagConfig() {}

    const AutoStr2 &getStartTag() const {   return m_sStartTag;     }
    const AutoStr2 &getEndTag() const   {   return m_sEndTag;       }
private:
    AutoStr2    m_sStartTag;
    AutoStr2    m_sEndTag;


    LS_NO_COPY_ASSIGN(SsiTagConfig);
};

class SsiConfig
{
public:
    SsiConfig();

    ~SsiConfig();


    void setEchoMsg(const char *pVal, int len)
    {   m_sEchoMsg.setStr(pVal, len);     }
    void setErrMsg(const char *pVal, int len)
    {   m_sErrMsg.setStr(pVal, len);      }
    void setTimeFmt(const char *pVal, int len)
    {   m_sTimeFmt.setStr(pVal, len);     }
    void setSizeFmt(const char *pVal, int len);

    const AutoStr2 *getEchoMsg() const
    {   return &m_sEchoMsg;     }
    const AutoStr2 *getErrMsg() const;
    const AutoStr2   *getTimeFmt() const
    {   return &m_sTimeFmt;     }
    int getSizeFmt() const
    {   return m_iSizeFmt;      }

    void setLastModOn(int on_off)
    {   m_iFlags = (m_iFlags & ~SSI_BIT_LASTMOD_ON) | (on_off ? SSI_BIT_LASTMOD_ON : 0);  }
    char isLastModOn() const
    {   return m_iFlags & SSI_BIT_LASTMOD_ON;    }

    void setEtagOn(int on_off)
    {   m_iFlags = (m_iFlags & ~SSI_BIT_ETAG_ON) | (on_off ? SSI_BIT_ETAG_ON : 0);  }
    char isEtagOn() const
    {   return m_iFlags & SSI_BIT_ETAG_ON;   }


    void copy(const SsiConfig *config);


private:
    AutoStr2    m_sEchoMsg;
    AutoStr2    m_sErrMsg;
    AutoStr2    m_sTimeFmt;
    int         m_iSizeFmt;   // 0: abbrev, 1:bytes
    char        m_iFlags;


    LS_NO_COPY_ASSIGN(SsiConfig);
};

#endif
