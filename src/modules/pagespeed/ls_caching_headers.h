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
#ifndef LSI_CACHING_HEADERS_H_
#define LSI_CACHING_HEADERS_H_

#include "pagespeed.h"
#include <lsdef.h>

#include "pagespeed/kernel/base/basictypes.h"
#include "pagespeed/kernel/http/caching_headers.h"

class LsiCachingHeaders : public CachingHeaders
{
public:
    explicit LsiCachingHeaders(lsi_session_t *session)
        : CachingHeaders(g_api->get_status_code(session)),
          m_session(session)
    {
    }

    virtual bool Lookup(const StringPiece &key, StringPieceVector *values);

    virtual bool IsLikelyStaticResourceType() const
    {
        return false;
    }

    virtual bool IsCacheableResourceStatusCode() const
    {
        return false;
    }

private:
    lsi_session_t *m_session;

    LS_NO_COPY_ASSIGN(LsiCachingHeaders);
};


#endif  // LSI_CACHING_HEADERS_H_
