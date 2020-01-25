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

#include "poolalloctest.h"

#include <util/poolalloc.h>
#include <util/misc/profiletime.h>

#include <iostream>
#include <map>
#include <string>
#include "UnitTest++/UnitTest++.h"


class dummy
{
public:
    int i1;
    int i2;
    std::string s1;
    std::string s2;
};

class TestAllocator : public
    std::map< int, dummy, std::less<int>, PoolAllocator<std::pair<const int, dummy>>>
{

};

TEST(PoolAllocTest_test)
{
    dummy d1, d2, d3 ;
    TestAllocator test;
    std::map<int, dummy> test2;
    const char *pPoolAlloc = "PoolAllocator benchmark";
    const char *pStdAlloc = "std::allocator benchmark";
    int i;
    {
        ProfileTime profile1;
        for (int i = 0 ; i < 100000; i++)
            test.insert(TestAllocator::value_type(i, d1));
        test.erase(test.begin(), test.end());
        profile1.printTime(pPoolAlloc, 100000);
    }
    {
        ProfileTime profile1;
        for (i = 0 ; i < 100000; i++)
            test2.insert(std::map<int, dummy>::value_type(i, d1));
        test2.erase(test2.begin(), test2.end());
        profile1.printTime(pStdAlloc, 100000);
    }
}

#endif
