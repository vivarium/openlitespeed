/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013 - 2015  LiteSpeed Technologies, Inc.                 *
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
#include <log4cxx/logsession.h>
#include <stddef.h>
#include <lsr/ls_atomic.h>

LogSession::LogSession()
    : m_pLogger(NULL)
{
    ls_str_blank(&m_logId);
    ls_spinlock_setup(&m_buildLock);
}


bool LogSession::allocLogId()
{
    if (!ls_str_prealloc(&m_logId, MAX_LOGID_LEN + 1))
    {
        return false;
    }
    m_logId.ptr[MAX_LOGID_LEN] = '\0';
    m_logId.ptr[0] = '\0';
    return true;
}

LogSession::~LogSession()
{
    ls_str_d(&m_logId);
}
