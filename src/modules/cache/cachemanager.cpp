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

#include "cachemanager.h"
#include "cacheentry.h"

#include <util/autobuf.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>



void CacheManager::populatePrivateTag()
{
    const char *pPrivateTags[] =
    {
        "E.formkey",
        "E.cart",
        "E.welcome",
        "E.minicart_head",
        "E.topLinks",
        "E.compare",
        "E.viewed",
        "E.compared",
        "E.poll",
        "E.messages",
        "E.reorder",
        "E.wishlist",
        "E.footer",
        "E.header",
        NULL
    };
    const char **p;
    for (p = pPrivateTags; *p != NULL; p++)
        getTagId(*p, strlen(*p));
}


void CacheManager::generateRpt(const char *name, AutoBuf *pBuf)
{
    CacheInfo *pInfo = getCacheInfo();
    cachestats_t *pPublic = pInfo->getPublicStats();
    cachestats_t *pPrivate = pInfo->getPrivateStats();
    char achBuf[4096];
    int n = snprintf(achBuf, 4096,
                     "[%s] PUB_CREATES:%d, PUB_HITS: %d, PUB_PURGE: %d, "
                     "PUB_EXPIRE: %d, PUB_COLLISION: %d, "
                     "PVT_CREATES:%d, PVT_HITS: %d, PVT_PURGE: %d, "
                     "PVT_EXPIRE: %d, PVT_COLLISION: %d, "
                     "PVT_SESSIONS: %d, PVT_SESSION_PURGE: %d, "
                     "FULLPAGE_HITS: %d, PARTIAL_HITS: %d\n",
                     name, pPublic->created, pPublic->hits, pPublic->purged,
                     pPublic->expired, pPublic->collisions,
                     pPrivate->created, pPrivate->hits, pPrivate->purged,
                     pPrivate->expired, pPrivate->collisions,
                     getPrivateSessionCount(),
                     pInfo->getSessionPurged(),
                     pInfo->getFullPageHits(),
                     pInfo->getPartialPageHits()
                    );
    pBuf->append(achBuf, n);

}

void CacheManager::updateStatsExpireByTracking(shm_objtrack_t *pData)
{
    CacheInfo *pInfo = getCacheInfo();
    ls_atomic_add(&(pInfo->getStats(pData->x_flag & CM_TRACK_PRIVATE)->expired),
                  1);
}

