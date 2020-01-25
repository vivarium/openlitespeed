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
#ifndef ACCESSCACHE_H
#define ACCESSCACHE_H



#include <lsdef.h>
#include <util/accesscontrol.h>

class AccessCache
{
    IPAccessControl     m_cache;
    //IP6AccessControl     m_cache6;
    AccessControl       m_accessCtrl;

public:
    AccessCache(int initSize);
    ~AccessCache() {};

    int  isAllowed(const struct sockaddr *pAddr);
    void onTimer();
    AccessControl *getAccessCtrl()
    {   return &m_accessCtrl;    }
    const AccessControl *getAccessCtrl() const
    {   return &m_accessCtrl;    }
    LS_NO_COPY_ASSIGN(AccessCache);
};

#endif
