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
#include "jconn.h"

#include <extensions/extworker.h>

#include <http/httpcgitool.h>
#include <http/httpextconnector.h>
#include <http/httpmime.h>
#include <http/httpstatuscode.h>
#include <http/httpsession.h>
#include <log4cxx/logger.h>

JConn::JConn()
    : m_pReqHeaderEnd(NULL)
    , m_pBufEnd(NULL)
    , m_iPendingHeader(0)
    , m_iPacketRemain(0)
    , m_iTotalPending(0)
    , m_curPacketSize(0)
    , m_packetLeft(0)
    , m_chunkLeft(0)
    , m_iNumHeader(0)
    , m_pCurPos(m_respBuf)
    , m_packetType(0)
    , m_iPacketState(PACKET_HEADER)
    , m_pRespBufEnd((unsigned char *) & (m_respBuf[ sizeof(m_respBuf)]))
{
}


JConn::~JConn()
{
}


inline int getInt(unsigned char *&p)
{
    int i = *p++;
    i <<= 8;
    i |= *p++;
    return i;
}
inline int peekInt(unsigned char *p)
{
    return ((((int) * p) << 8) | (*(p + 1)));
}


//inline unsigned long getLong( unsigned chsr * &p )
//{
//
//}


void JConn::init(int fd, Multiplexer *pMplx)
{
    EdStream::init(fd, pMplx, POLLIN | POLLOUT | POLLHUP | POLLERR);
    //Do not call reset(), not necessary
    //reset();
}


int JConn::doWrite()
{
    if (getConnector())
    {
        int state = getConnector()->getState();
        if ((!state) || (state & (HEC_FWD_REQ_HEADER | HEC_FWD_REQ_BODY)))
            return getConnector()->extOutputReady();
    }
    if (m_iTotalPending > 0)
        return flush();
    else
        suspendWrite();
    return 0;
}


int JConn::processPacketHeader(unsigned char *&p)
{
    if ((*p != AJP_RESP_PREFIX_B1) ||
        (*(p + 1) != AJP_RESP_PREFIX_B2))
    {
        LS_ERROR(this, "Invalid AJP response signature %x%x", (int) *p,
                 (int) * (p + 1));
        return LS_FAIL;
    }
    p += 2;
    m_curPacketSize = getInt(p);
    if (m_curPacketSize > AJP_MAX_PKT_BODY_SIZE)
    {
        LS_ERROR(this, "Packet size is too large - %d", m_curPacketSize);
        return LS_FAIL;
    }
    m_packetType = *p++;
    m_packetLeft = m_curPacketSize - 1;
    switch (m_packetType)
    {
    case AJP13_RESP_BODY_CHUNK:
        m_iPacketState = CHUNK_LEN;
        break;
    case AJP13_RESP_HEADERS:
        m_iPacketState = STATUS_CODE;
        break;
    case AJP13_END_RESP:
        if (*p != 1)
        {
            LS_DBG_L(this, "Close connection required by servlet engine %s ",
                     getWorker()->getURL());

            setState(CLOSING);
        }
        p++;
        if (getConnector())
        {
            incReqProcessed();
            if (getState() == ABORT)
                setState(PROCESSING);
            setInProcess(0);
            getConnector()->endResponse(0, 0);
        }
        break;
    case AJP13_MORE_REQ_BODY:
        if (getConnector())
        {
            if ((getConnector()->getState() & HEC_FWD_REQ_BODY) == 0)
            {
                LS_DBG_L(this, "REQ_BODY finished, container asking for more, "
                        "send blank packet for EOF.");
                JkAjp13::buildAjpHeader( m_pBufEnd, 0 );
                write( m_pBufEnd, 4 );
            }
        }
        break;
    default:
        break;
    }
    return 0;
}


int JConn::processPacketData(unsigned char *&p)
{
    if (m_iPacketState == PACKET_HEADER)
    {
        //m_respHeader + m_packetLeft
        if (m_pCurPos - p >= AJP_MIN_PACKET_SIZE)
        {
            int ret = processPacketHeader(p);
            if (ret)
                return LS_FAIL;
        }
        else
            return 1;
    }
    if (p == m_pCurPos)
        return 1;
    int ret = 0;
    unsigned char *pCur = p;
    unsigned char *pEnd = p + m_packetLeft;
    if (pEnd > m_pCurPos)
        pEnd = m_pCurPos;
    if (getConnector())
        ret = processPacketContent(p, pEnd);
    else
        p = pEnd;
    if (ret == -1)
        return ret;
    if (pCur == p)
        return 1;
    m_packetLeft -= p - pCur;
    if (m_packetLeft == 0)
        m_iPacketState = PACKET_HEADER;
    return 0;
}


int JConn::processPacketContent(unsigned char *&p, unsigned char *pEnd)
{
    int ret = 0;
    switch (m_iPacketState)
    {
    case CHUNK_LEN:
        if (pEnd - p >= 2)
        {
            m_chunkLeft = getInt(p);
            assert(m_chunkLeft == m_curPacketSize - 4);
            m_iPacketState = CHUNK_DATA;
        }
        else
            break;
    //fall through
    case CHUNK_DATA:
        if (pEnd - p > 0)
        {
            int len = m_chunkLeft;
            if (pEnd - p < len)
                len = pEnd - p;
            if (len > 0)
            {
                if (!((getConnector()->getState() &
                       (HEC_ABORT_REQUEST | HEC_ERROR | HEC_COMPLETE | HEC_REDIRECT))))
                    ret = getConnector()->processRespBodyData((const char *)p, len);
                p += len;
                m_chunkLeft -= len;
            }
            if (p < pEnd)
                ++p;
        }
        break;
    case STATUS_CODE:
        if (pEnd - p >= 2)
        {
            int code = getInt(p);
            code = HttpStatusCode::getInstance().codeToIndex(code);
            if (code != -1)
                getConnector()->getHttpSession()->getReq()->updateNoRespBodyByStatus(code);

            m_iPacketState = STATUS_MSG;
        }
        else
            break;
    //fall through
    case STATUS_MSG:
        if (pEnd - p > 2)
        {
            int strLen = peekInt(p);
            if (strLen + 2 < pEnd - p)
            {
                p += strLen + 3;    //skip status message as we don't use it
                m_iPacketState = NUM_HEADERS;
            }
            else
                break;
        }
        else
            break;
    //fall through
    case NUM_HEADERS:
        if (pEnd - p >= 2)
        {
            m_iNumHeader = getInt(p);
            m_iPacketState = RESP_HEADER;
        }
        else
            break;
    //fall through
    case RESP_HEADER:
        ret = readRespHeader(p, pEnd);
        //fall through
    default:
        p = pEnd;
        break;
    }
    return ret;

}


int JConn::readRespHeader(unsigned char *&p, unsigned char *pEnd)
{
    int &status = getConnector()->getRespState();
    while (m_iNumHeader > 0)
    {
        if (pEnd - p < 4)
            return 0;
        unsigned char id1 = *p;
        unsigned char id2;
        int headerNameLen;
        const char *pHeaderName;
        unsigned char *p1;
        if (id1 == 0xA0)
        {
            id2 = *(p + 1);
            if ((id2 > 0) && (id2 <= AJP_RESP_HEADERS_NUM))
            {
                pHeaderName = JkAjp13::getRespHeaderById(id2);
                headerNameLen = JkAjp13::getRespHeaderLenById(id2);
                p1 = p + 2;
            }
            else
            {
                //invalid header id
                return LS_FAIL;
            }
        }
        else
        {
            headerNameLen = id1 << 8 | *(p + 1);
            if (pEnd - p < headerNameLen + 5)
                return 0;
            pHeaderName = (const char *)p + 2;
            p1 = p + headerNameLen + 3;
        }
        int headerValLen = peekInt(p1);
        if (pEnd - p1 < headerValLen + 3)
            return 0;
        char *pHeaderVal = (char *)p1 + 2;
        p = p1 + headerValLen + 3;
        --m_iNumHeader;
        *(p1 - 1) = ':';
        *p1 = ' ';
        *(p1 + 1) = ' ';
        if (HttpCgiTool::processHeaderLine(
                getConnector(),
                pHeaderName, headerNameLen,
                pHeaderVal, headerValLen, status) == -1)
            return LS_FAIL;

    }
    getConnector()->getRespState() |= HttpReq::HEADER_OK;
    return getConnector()->respHeaderDone();
}


int JConn::processRespData()
{
    int ret;
    unsigned char *p = m_respBuf;
    while (p < m_pCurPos)
    {
        ret = processPacketData(p);
        if (ret == 1)
        {
            if (p != m_pCurPos)
            {
                memmove(m_respBuf, p, m_pCurPos - p);
                m_pCurPos = m_respBuf + (m_pCurPos - p);
                return 0;
            }
        }
        else if (ret == -1)
            return LS_FAIL;
    }
    m_pCurPos = m_respBuf;
    return 0;
}


int JConn::doRead()
{
    int len = 0;
    int ret = 0;
    while (true)
    {
        int toRead = m_pRespBufEnd - m_pCurPos;
        len = read((char *)m_pCurPos, toRead);
        if (len > 0)
        {
            LS_DBG_M(this, "Process STDOUT %d bytes", len);
            //printf( ">>read %d bytes from CGI\n", len );
            //::write( 1, m_pCurPos, len );
            m_pCurPos += len;
            ret = processRespData();
            if (ret == -1)
            {
                errno = EIO;
                len = -1;
                break;
            }
            if (len < toRead)
            {
                if ((m_packetType != AJP13_END_RESP) &&
                    (getConnector()))
                    getConnector()->flushResp();
                break;
            }
        }
        else
            break;
    }
    if (getState() == ABORT)
    {
        if (getConnector())
        {
            incReqProcessed();
            getConnector()->endResponse(0, 0);
        }
    }
    return len;
}
int JConn::doError(int err)
{
    return 0;
}


int JConn::addRequest(ExtRequest *pReq)
{
    assert(pReq);
    setConnector((HttpExtConnector *)pReq);
    reset();
    m_pCurPos = m_respBuf;
    m_iPacketState = PACKET_HEADER;
    int ret = buildReqHeader();
    if (ret)
    {
        LS_DBG_L(this, "Request header can't fit into 8K buffer, "
                 "can't forward request to servlet engine");
        ((HttpExtConnector *)pReq)->setProcessor(NULL);
        setConnector(NULL);
        ret = SC_500;
    }
    return ret;
    //return 0;
}


ExtRequest *JConn::getReq() const
{
    return getConnector();
}


int JConn::removeRequest(ExtRequest *pReq)
{
    if (getConnector())
    {
        getConnector()->setProcessor(NULL);
        setConnector(NULL);
    }
    return 0;
}


void JConn::reset()
{
    memset(&m_pReqHeaderEnd, 0, (char *)(&m_iTotalPending + 1) -
           (char *)(&m_pReqHeaderEnd));
}


void JConn::abort()
{
    setState(ABORT);
}


int  JConn::begin()
{
    return 1;
}


int  JConn::beginReqBody()
{
    return 1;
}


// m_iPendingHeader = -1, no half body packet sent
//                    0, no header left
//                    1-6, header need to be sent

int  JConn::sendReqBody(const char *pBuf, int size)
{
    if (m_iTotalPending > 0)
        if (flush() == -1)
            return LS_FAIL;
    if (m_iTotalPending > 0)
        return 0;
    if (size > m_iPacketRemain)
        size = m_iPacketRemain;
    int total = size;
    if (m_iPendingHeader == -1)
    {
        JkAjp13::buildAjpReqBodyHeader(m_pBufEnd, size);
        m_iovec.append(m_pBufEnd, 6);
        m_iPendingHeader = 6;
    }
    if (m_iPendingHeader > 0)
        total += m_iPendingHeader;
    m_iovec.append(pBuf, size);
    int ret = writev(m_iovec, total);
    if (ret >= total)
    {
        m_iPendingHeader = -1;
        m_iovec.clear();
        m_iPacketRemain = AJP_MAX_PKT_BODY_SIZE - 2;
        return size;
    }
    else if (ret > 0)
    {
        if (ret < m_iPendingHeader)
        {
            m_iPendingHeader -= ret;
            m_iovec.pop_back(1);
            m_iovec.finish(ret);
            ret = 0;
        }
        else
        {
            ret -= m_iPendingHeader;
            m_iPendingHeader = 0;
            m_iovec.clear();
        }
    }
    return ret;
}


int  JConn::endOfReqBody()
{
    //if ( m_iPendingBody )
    //    sendReqBodyPacket();
    if (m_iTotalPending)
    {
        int ret = flush();
        if (ret)
            return ret;
    }
    //JkAjp13::buildAjpHeader( m_pBufEnd, 0 );
    //write( m_pBufEnd, 4 );
    suspendWrite();
    return 0;
}


int  JConn::flush()
{
    if (m_iTotalPending)
    {
        int ret = writev(m_iovec, m_iTotalPending);
        if (ret >= m_iTotalPending)
        {
            ret -= m_iTotalPending;
            m_iTotalPending = 0;
            m_iovec.clear();
        }
        else
        {
            if (ret > 0)
            {
                m_iTotalPending -= ret;
                m_iovec.finish(ret);
                return 1;
            }
            return LS_FAIL;
        }
    }
    return 0;
}


int  JConn::readResp(char *pBuf, int size)
{
    return 0;
}


void JConn::cleanUp()
{
    setConnector(NULL);
    reset();
    recycle();
}


void JConn::finishRecvBuf()
{
}


bool JConn::wantRead()
{
    return true;
}


bool JConn::wantWrite()
{
    return true;
}


int JConn::buildReqHeader()
{
    m_pReqHeaderEnd = m_buf + 4;
    int ret = JkAjp13::buildReq(
                  getConnector()->getHttpSession(), m_pReqHeaderEnd,
                  &m_buf[ AJP_MAX_PACKET_SIZE ]);
    if (ret == -1)
        return LS_FAIL;
    m_pBufEnd = m_pReqHeaderEnd;
    ret = JkAjp13::buildWorkerHeader((JWorker *)getWorker(),
                                     m_pBufEnd, &m_buf[ AJP_MAX_PACKET_SIZE]);
    if (ret == -1)
        return LS_FAIL;
    JkAjp13::buildAjpHeader(m_buf, m_pBufEnd - m_buf - 4);
    return 0;
}


int JConn::sendReqHeader()
{
    m_iovec.clear();
    m_iovec.append(m_buf, m_pBufEnd - m_buf);
    m_iTotalPending = m_pBufEnd - m_buf;
    m_iPendingHeader = -1;
    m_iPacketRemain = AJP_MAX_PKT_BODY_SIZE - 2;
    setInProcess(1);
    return 1;
}

