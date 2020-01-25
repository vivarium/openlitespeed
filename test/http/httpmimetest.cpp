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
#ifdef RUN_TEST

#include "httpmimetest.h"

#include <http/httpmime.h>
#include <util/autostr.h>
#include <unistd.h>
#include "UnitTest++/UnitTest++.h"
#include <string.h>

extern const char *argv0;

const char *get_server_root(char *achServerRoot, ssize_t sz)
{
    if (*argv0 != '/')
    {
        if (getcwd(achServerRoot, sz))
            lstrncat(achServerRoot, "/", sz);
    }
    else
        achServerRoot[0] = 0;
    lstrncat(achServerRoot, argv0, sz);
    const char *pEnd = strrchr(achServerRoot, '/');
    if (pEnd)
    {
        --pEnd;
        while (pEnd > achServerRoot && *pEnd != '/')
            --pEnd;
        --pEnd;
        while (pEnd > achServerRoot && *pEnd != '/')
            --pEnd;
        ++pEnd;

        lstrncpy(&achServerRoot[pEnd - achServerRoot], "test/serverroot",
                 sz - (pEnd - achServerRoot));
    }
    else
        lstrncpy(achServerRoot, "/", sz);
    return achServerRoot;
}



TEST(HttpMimeTest_runTest)
{
    HttpMime m;
    const char *pOldType ;
    const char *pNewType;
    const char *pOldType2 ;
    const char *pNewType2;
    int ret;
    char achServerRoot[1024];
    char achBuf[256];
    char *p = achBuf;
    lstrncpy(p, get_server_root(achServerRoot, sizeof(achServerRoot)), sizeof(achBuf));
    CHECK(p != NULL);
    char *pEnd = p + strlen(p);

//   CHECK(m.loadMime("/proj/httpd/httpd/serverroot/conf/m2")!=0);
    lstrncpy(pEnd, "/conf/m2", sizeof(achBuf) - (pEnd - achBuf));
    ret = m.loadMime(achBuf);
    CHECK(ret == 0);
    if (ret != 0)
        return;
    //printf( "m.getFileMime(\"as/dadf/abc.doc\") return %s\n", m.getFileMime("as/dadf/abc.doc"));
    CHECK(strcmp(m.getFileMime("as/dadf/abc.doc")->getMIME()->c_str(),
                 "application/msword") == 0);
    CHECK(strcmp(m.getFileMime("as/dadf/abc.html")->getMIME()->c_str(),
                 "text/html") == 0);
    CHECK(strcmp(m.getFileMime("as/dadf/abc.htm")->getMIME()->c_str(),
                 "text/html") == 0);
    CHECK(m.getFileMime("as/dadf/abc") == NULL);
    CHECK(m.getFileMime("as/dadf/abc.") == NULL);
    CHECK(m.getFileMime("as/dadf/abc") == NULL);

    pOldType = m.getFileMime("f/abc.jpg")->getMIME()->c_str();
    CHECK(strcmp(pOldType, "image/jpeg") == 0);
    pOldType2 = m.getFileMime("asdsa/sda3.gzip")->getMIME()->c_str();
    CHECK(strcmp(pOldType2, "application/gzip0") == 0);
    lstrncpy(pEnd, "/conf/m1", sizeof(achBuf) - (pEnd - achBuf));
    m.loadMime(achBuf);
    CHECK(strcmp(m.getFileMime("as/dadf/abc.html")->getMIME()->c_str(),
                 "text/html") == 0);
    CHECK(strcmp(m.getFileMime("as/dadf/abc.htm")->getMIME()->c_str(),
                 "text/html") == 0);
    pNewType = m.getFileMime("f/abc.jpg")->getMIME()->c_str();
    CHECK(pOldType == pNewType);
    pNewType2 = m.getFileMime("asdsa/sda3.gzip")->getMIME()->c_str();
    CHECK(strcmp(pNewType2, "application/gzip") == 0);
    CHECK(pNewType2 != pOldType2);
    CHECK(strcmp(pOldType2, "application/gzip0") == 0);
    lstrncpy(pEnd, "/conf/m2", sizeof(achBuf) - (pEnd - achBuf));
    m.loadMime(achBuf);
    pNewType = m.getFileMime("asdsa/sda3.gzip")->getMIME()->c_str();
    CHECK(strcmp(pNewType, "application/gzip0") == 0);
    CHECK(pNewType != pNewType2);
    CHECK(pNewType == pOldType2);
    CHECK(strcmp(pOldType2, "application/gzip0") == 0);
    CHECK(strcmp(pNewType2, "application/gzip") == 0);
    lstrncpy(pEnd, "/conf/m2", sizeof(achBuf) - (pEnd - achBuf));
    m.loadMime(achBuf);
    pNewType2 = m.getFileMime("asdsa/sda3.gzip")->getMIME()->c_str();
    CHECK(pNewType == pNewType2);

}

#endif
