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
#include "httpextprocessor.h"
#include <http/httpextconnector.h>


void HttpExtProcessor::setConnector(HttpExtConnector *pConnector)
{
    m_pConnector = pConnector;
    if (pConnector)
        pConnector->setProcessor(this);
}


LOG4CXX_NS::Logger *HttpExtProcessor::getLogger() const
{
    if (m_pConnector)
        return m_pConnector->getLogger();
    else
        return NULL;
}


const char *HttpExtProcessor::getLogId()
{
    if (m_pConnector)
        return m_pConnector->getLogId();
    else
        return "idle";
}





