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
#ifndef STATICFILECACHE_H
#define STATICFILECACHE_H


#include <lsdef.h>
#include <http/httpcache.h>
#include <util/tsingleton.h>

class StaticFileCacheData;
class FileCacheDataEx;

class StaticFileCache : public HttpCache,
    public TSingleton<StaticFileCache>
{
    friend class TSingleton<StaticFileCache>;

    CacheElement *allocElement();
    void recycle(CacheElement *pElement);
    StaticFileCacheData * newCache(const char *pPath, int pathLen,
                  const struct stat &fileStat, int fd);
    StaticFileCache();
public:
    ~StaticFileCache();

    int getCacheElement(const char *pPath, int pathLen,
                        const struct stat &fileStat, int fd,
                        StaticFileCacheData **pData);
    void returnCacheElement(StaticFileCacheData *pElement);
    LS_NO_COPY_ASSIGN(StaticFileCache);
};

LS_SINGLETON_DECL(StaticFileCache);
#endif
