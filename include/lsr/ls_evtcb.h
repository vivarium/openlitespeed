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
#ifndef LS_EVTCB_H
#define LS_EVTCB_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct evtcbtail_s
{
    struct evtcbnode_s *evtcb_tail;
    struct evtcbtail_s **back_ref_ptr;
}
evtcbtail_t;


/**
 * @typedef evtcb_pf
 * @brief The callback function for scheduling event.
 *
 */
typedef int (*evtcb_pf)(evtcbtail_t *pSession,
                        const long lParam, void *pParam);


#ifdef __cplusplus
}
#endif


#endif
