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
#ifndef PROXYCONN_H
#define PROXYCONN_H


#include <lsdef.h>
#include <extensions/extconn.h>
#include <extensions/httpextprocessor.h>
#include <sslpp/sslconnection.h>

#define PCF_IN_DO_READ  1

class ChunkInputStream;
class ProxyConn : public ExtConn
    , public HttpExtProcessor
{
    IOVec       m_iovec;
    int         m_iTotalPending;
    int         m_iReqHeaderSize;
    long        m_lReqBeginTime;
    long        m_lReqSentTime;
    long        m_lLastRespRecvTime;
    int         m_iRespRecv;
    int         m_iRespHeaderRecv;

    int64_t     m_iReqBodySize;
    int64_t     m_iReqTotalSent;

    int64_t     m_iRespBodySize;
    int64_t     m_iRespBodyRecv;

    const char *m_pBufBegin;
    const char *m_pBufEnd;

    ChunkInputStream *m_pChunkIS;

    short           m_iSsl;
    short           m_flag;
    SslConnection   m_ssl;

    char        m_extraHeader[256];  //X-Forwarded-For

    int         processResp();
    int         readRespBody();
    void        setupChunkIS();
    int         connectSSL();

    int         readvSsl(const struct iovec *vector, const struct iovec *pEnd);
    void        setSSLAgain();
    int         addForwardedFor(const char *pBegin);

protected:
    virtual int doRead();
    virtual int doWrite();
    virtual int doError(int err);
    virtual int addRequest(ExtRequest *pReq);
    virtual ExtRequest *getReq() const;
    virtual void init(int fd, Multiplexer *pMplx);
    virtual int onTimer();

    int read(char *pBuf , int size);
    int readv(struct iovec *vector, size_t count);

public:
    virtual int removeRequest(ExtRequest *pReq);
    void continueRead();

public:
    ProxyConn();
    ~ProxyConn();

    virtual void finishRecvBuf();

    virtual bool wantRead();
    virtual bool wantWrite();

    virtual void abort();
    virtual int  begin();
    virtual int  beginReqBody();
    virtual int  endOfReqBody();
    virtual int  sendReqBody(const char *pBuf, int size);
    virtual int  readResp(char *pBuf, int size);
    virtual int  flush();
    virtual void cleanUp();
    virtual void dump();

    virtual int sendReqHeader();
    virtual int close();
    void reset();

    void setUseSsl(int s)       {   m_iSsl = s;       }
    short isUseSsl() const      {   return m_iSsl;    }
    short isInDoRead() const    {   return m_flag & PCF_IN_DO_READ;     }

    LS_NO_COPY_ASSIGN(ProxyConn);
};

#endif
