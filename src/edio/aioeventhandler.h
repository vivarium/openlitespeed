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
#ifndef AIOEVENTHANDLER_H
#define AIOEVENTHANDLER_H

#define LS_AIO_USE_AIO

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#include <linux/version.h>
#define HS_AIO (SIGRTMIN + 4)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25)
#define LS_AIO_USE_SIGFD
#else
#define LS_AIO_USE_SIGNAL
#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25)
#elif defined(__FreeBSD__ ) || defined(__OpenBSD__)
#define LS_AIO_USE_KQ
#else
#undef LS_AIO_USE_AIO
#endif // defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)


class AioEventHandler
{
public:
    virtual int onAioEvent() = 0;
};

#endif //AIOEVENTHANDLER_H
