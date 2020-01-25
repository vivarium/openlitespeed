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
#ifndef DATETIME_H
#define DATETIME_H



#include <time.h>

#define RFC_1123_TIME_LEN 29
class DateTime
{
public:
    static time_t s_curTime;
    static int    s_curTimeUs;
    DateTime();
    ~DateTime();

    static time_t parseHttpTime(const char *s, int len);
    static char  *getRFCTime(time_t t, char *buf);
    static char  *getLogTime(time_t lTime, char *pBuf, int bGMT = 0);

};

#endif
