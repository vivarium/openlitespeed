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
#ifndef FCGIENV_H
#define FCGIENV_H

#include <lsdef.h>
#include <util/autobuf.h>
#include <util/ienv.h>


class FcgiEnv : public IEnv
{
    AutoBuf  m_buf;
public:
    FcgiEnv() : m_buf(2048) {};
    ~FcgiEnv() {}
    int add(const char *name, const char *value)
    {   return IEnv::add(name, value); }

    int add(const char *name, size_t nameLen,
            const char *value, size_t valLen);
    int add(const char *buf, size_t len)
    {   return m_buf.append(buf, len);   }
    void clear()
    {   m_buf.clear();  }
    const char *get() const {  return m_buf.begin();   }
    int size() const          {  return m_buf.size();     }
    LS_NO_COPY_ASSIGN(FcgiEnv);
};

#endif
