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
#ifndef JWORKERCONFIG_H
#define JWORKERCONFIG_H

#include <extensions/extworkerconfig.h>
#include <lsdef.h>



class JWorkerConfig : public ExtWorkerConfig
{
    char   *m_pSecret;
    int     m_secretLen;
public:
    JWorkerConfig(const char *pName);
    JWorkerConfig();
    ~JWorkerConfig();
    void setSecret(const char *pSecret);
    const char *getSecret() const  {   return m_pSecret;   }
    int getSecretLen() const        {   return m_secretLen; }

    LS_NO_COPY_ASSIGN(JWorkerConfig);
};

#endif
