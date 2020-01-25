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


#ifndef EVENTREACTOR_H
#define EVENTREACTOR_H

#include <lsdef.h>
#include <stddef.h>
#include <poll.h>

#define ERF_UPDATE  1
#define ERF_ADD     2
#define ERF_REMOVE  4

class Multiplexer;

class EventReactor
{
    struct pollfd   m_pollfd;
    struct pollfd  *m_pfd;
    int             m_cntHup;
    unsigned short  m_eventSet;
    unsigned short  m_flags;
public:

    typedef int (*pri_handler)();
    typedef void (*command_fn)(EventReactor *pThis);

    EventReactor() 
        : m_pfd(NULL)
        , m_cntHup(0)
        , m_eventSet(0)
        , m_flags(0)
    {   m_pollfd.fd = -1;   m_pollfd.events = 0; m_pollfd.revents = 0;  }
    explicit EventReactor(int fd)
        : m_pfd(NULL)
        , m_cntHup(0)
        , m_eventSet(0)
        , m_flags(0)
    {   m_pollfd.fd = fd; m_pollfd.events = 0; m_pollfd.revents = 0;  }

    virtual ~EventReactor() {};
    virtual int handleEvents(short event) = 0;
    virtual int onTimer()   {   return 0;   }

    int getfd() const                   {   return m_pollfd.fd;     }

    void setfd(int fd)
    {
        m_pollfd.fd = fd;
        m_pollfd.revents = 0;
        m_cntHup = 0;
        m_eventSet = 0;
        m_flags = 0;
    }

    int getHupCounter() const           {   return m_cntHup;        }
    void incHupCounter()                {   ++m_cntHup;             }

    struct pollfd *getPollfd() const    {   return m_pfd;           }
    void setPollfd()                    {   m_pfd = &m_pollfd;      }
    void setPollfd(struct pollfd *pollfd)
    {   m_pfd = pollfd; }
    short getEvents() const             {   return m_pfd->events;    }
    void setMask2(short mask)           {   m_pfd->events  = mask;   }
    void orMask2(short mask)            {   m_pfd->events |= mask;   }
    void andMask2(short mask)           {   m_pfd->events &= mask;   }
    short getRevents() const            {   return m_pfd->revents;   }
    void setRevent(short event)         {   m_pfd->revents |= event; }
    void resetRevent(short event)       {   m_pfd->revents &= ~event;}
    void clearRevent()                  {   m_pollfd.revents = 0;    }
    void assignRevent(short event)      {   m_pollfd.revents = event;}
    short getAssignedRevent()           {   return m_pollfd.revents; }
    
    void updateEventSet()               {   m_eventSet = m_pfd->events;     }
    int  isApplyEvents() const          {   return m_eventSet != m_pfd->events;  }
    
    void addFlag(unsigned short flag)   {   m_flags |= flag;        }
    void removeFlag(unsigned short flag){   m_flags &= ~flag;       }
    unsigned short getEvtFlag() const   {   return m_flags;         }

    Multiplexer *getMultiplexer() const;

    LS_NO_COPY_ASSIGN(EventReactor);
};

#endif
