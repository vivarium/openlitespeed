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

#include "xmlnodetest.h"
#include <util/xmlnode.h>
#include <unistd.h>
#include "UnitTest++/UnitTest++.h"

extern const char *get_server_root(char *achServerRoot, ssize_t sz);

TEST(XmlNodeTest_test)
{
    char achError[1024];
    char achBuf[256];
    char *p = achBuf;
    lstrncpy(p, get_server_root(achError, sizeof(achBuf)), sizeof(achBuf) );
    CHECK(p != NULL);
    strcat(achBuf, "/conf/myconfig.xml");

    XmlTreeBuilder builder;
    XmlNode *pRoot = builder.parse(achBuf, achError, 1024);
    CHECK(pRoot != NULL);
//     pRoot->xmlOutput(stdout, 0);
    delete pRoot;
}

#endif
