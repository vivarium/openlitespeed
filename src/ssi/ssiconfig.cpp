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
#include "ssiconfig.h"
#include <string.h>

SsiConfig::SsiConfig()
    : m_iSizeFmt(0)
    , m_iFlags(0)
{
}


SsiConfig::~SsiConfig()
{
}


void SsiConfig::setSizeFmt(const char *pVal, int len)
{
    if (strncasecmp(pVal, "bytes", 4) == 0)
        m_iSizeFmt = 1;
}


void SsiConfig::copy(const SsiConfig *config)
{
    if (!config)
        return;
    if (config->m_sEchoMsg.c_str())
        m_sEchoMsg.setStr(config->m_sEchoMsg.c_str());
    if (config->m_sErrMsg.c_str())
        m_sErrMsg.setStr(config->m_sErrMsg.c_str());
    if (config->m_sTimeFmt.c_str())
        m_sTimeFmt.setStr(config->m_sTimeFmt.c_str());
    m_iSizeFmt = config->m_iSizeFmt;

}


const AutoStr2 *SsiConfig::getErrMsg() const
{
    static AutoStr2
    s_default("[an error occurred while processing this directive]\n");
    if (m_sErrMsg.c_str())
        return &m_sErrMsg;
    else
        return &s_default;

}

