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
#ifndef MODULEHANDLER_H
#define MODULEHANDLER_H

#include <http/reqhandler.h>
#include <ls.h>

class HttpHandler;
class HttpSession;
class WorkCrew;

class ModuleHandler : public ReqHandler
{
public:
    ModuleHandler();
    ~ModuleHandler();
    virtual const char *getName() const {  return "module"; }
    virtual int cleanUp(HttpSession *pSession);
    virtual int onWrite(HttpSession *pSession);
    virtual int process(HttpSession *pSession, const HttpHandler *pHandler);
    virtual void onTimer() {};
    virtual int onRead(HttpSession *pSession);
    
    static  void initGlobalWorkCrew();
    static  WorkCrew *getGlobalWorkCrew();

private:
    int mt_cleanUp(HttpSession *pSession, const lsi_reqhdlr_t *pModuleHandler);
    int mt_onWrite(HttpSession *pSession, const lsi_reqhdlr_t *pModuleHandler);
    int mt_process(HttpSession *pSession, const lsi_reqhdlr_t *pModuleHandler);
    int mt_onRead(HttpSession *pSession, const lsi_reqhdlr_t *pModuleHandler);
    
};

#endif // MODULEHANDLER_H
