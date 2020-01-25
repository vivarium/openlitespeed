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
#include "contextlist.h"

#include <lsdef.h>
#include <http/httpcontext.h>

#include <errno.h>
#include <string.h>

ContextList::ContextList()
    : TPointerList< HttpContext >(4)
{
    m_sTags.prealloc(capacity());
    memset(m_sTags.buf(), 0, capacity());
}


ContextList::~ContextList()
{
    release();
}


void ContextList::release()
{
    iterator iter;
    iterator iterEnd = end();
    char *p = m_sTags.buf();
    for (iter = begin(); iter != iterEnd; ++iter, ++p)
    {
        if (*p)
            delete(*iter);
    }
    clear();
}


int ContextList::add(HttpContext *pContext, int release)
{
    int n = size();
    int cap = capacity();
    if (cap <= n)
    {
        if (m_sTags.prealloc(cap * 2))
        {
            memset(m_sTags.buf() + cap, 0, cap);
        }
        else
            return LS_FAIL;
    }
    push_back(pContext);
    m_sTags.buf()[n] = release;
    return 0;
}


int ContextList::merge(const ContextList *rhs, int release)
{
    const_iterator iter;
    const_iterator iterEnd;
    if (!rhs)
        return 0;
    iterEnd = rhs->end();
    for (iter = rhs->begin(); iter != iterEnd; ++iter)
    {
        if (add(*iter, 0))
            return LS_FAIL;
    }
    return 0;
}


void ContextList::releaseUnused(long curTime, long timeout)
{
    iterator iter;
    for (iter = begin(); iter != end();)
    {
        if (curTime - (*iter)->getLastMod() > timeout)
        {
            if (m_sTags.buf()[iter - begin()])
                delete(*iter);
            erase(iter);
            m_sTags.buf()[iter - begin()] = m_sTags.buf()[size()];
        }
        else
            ++iter;
    }
}

