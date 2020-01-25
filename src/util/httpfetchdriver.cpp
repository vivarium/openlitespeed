/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013  LiteSpeed Technologies, Inc.                        *
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
#include "httpfetchdriver.h"
#include "httpfetch.h"
#include <edio/multiplexerfactory.h>
#include <edio/multiplexer.h>


HttpFetchDriver::HttpFetchDriver(HttpFetch *pHttpFetch)
{
    m_pHttpFetch = pHttpFetch;
}

int HttpFetchDriver::handleEvents(short int event)
{
    return m_pHttpFetch->processEvents(event);
}

int HttpFetchDriver::onTimer()
{
    if (m_pHttpFetch->getTimeout() < 0)
        return 0;

    if (time(NULL) - m_pHttpFetch->getTimeStart() >= m_pHttpFetch->getTimeout())
    {
        m_pHttpFetch->setTimeout(-1);
        m_pHttpFetch->closeConnection();
        return 1;
    }
    return 0;
}

void HttpFetchDriver::start()
{
    setPollfd();
    Multiplexer *pMpl = getMultiplexer();
    if (pMpl)
        pMpl->add(this,  POLLIN | POLLOUT | POLLHUP | POLLERR);
}

void HttpFetchDriver::stop()
{
    Multiplexer *pMpl = getMultiplexer();
    if (pMpl)
        pMpl->remove(this);
}

void HttpFetchDriver::switchWriteToRead()
{
    getMultiplexer()->switchWriteToRead(this);
}

void HttpFetchDriver::switchReadToWrite()
{
    getMultiplexer()->switchReadToWrite(this);
}

void HttpFetchDriver::continueWrite()
{
    getMultiplexer()->continueWrite(this);
} 
