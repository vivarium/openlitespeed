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
#include "ls_rewrite_driver_factory.h"

#include <cstdio>

#include "log_message_handler.h"
#include "ls_message_handler.h"
#include "ls_rewrite_options.h"
#include "ls_server_context.h"

#include "net/instaweb/http/public/rate_controller.h"
#include "net/instaweb/http/public/rate_controlling_url_async_fetcher.h"
#include "net/instaweb/http/public/wget_url_fetcher.h"
#include "net/instaweb/rewriter/public/rewrite_driver.h"
#include "net/instaweb/rewriter/public/rewrite_driver_factory.h"
#include "net/instaweb/rewriter/public/server_context.h"
#include "net/instaweb/util/public/property_cache.h"
#include "pagespeed/kernel/base/google_message_handler.h"
#include "pagespeed/kernel/base/null_shared_mem.h"
#include "pagespeed/kernel/base/posix_timer.h"
#include "pagespeed/kernel/base/stdio_file_system.h"
#include "pagespeed/kernel/base/string.h"
#include "pagespeed/kernel/base/string_util.h"
#include "pagespeed/kernel/base/thread_system.h"
#include "pagespeed/kernel/http/content_type.h"
#include "pagespeed/kernel/sharedmem/shared_circular_buffer.h"
#include "pagespeed/kernel/sharedmem/shared_mem_statistics.h"
#include "pagespeed/kernel/thread/pthread_shared_mem.h"
#include "pagespeed/kernel/thread/scheduler_thread.h"
#include "pagespeed/kernel/thread/slow_worker.h"
#include "pagespeed/system/in_place_resource_recorder.h"
//#include "pagespeed/system/serf_url_async_fetcher.h"
#include "pagespeed/system/system_caches.h"
#include "pagespeed/system/system_rewrite_options.h"

namespace net_instaweb
{
class FileSystem;
class Hasher;
class MessageHandler;
class Statistics;
class Timer;
class UrlAsyncFetcher;
class UrlFetcher;
class Writer;

class SharedCircularBuffer;

LsRewriteDriverFactory::LsRewriteDriverFactory(
        const ProcessContext &process_context,
        SystemThreadSystem *system_thread_system,
        StringPiece hostname, int port)
    : SystemRewriteDriverFactory(process_context, system_thread_system,
                                 NULL, hostname, port)
    , m_timer(NULL)
    , m_bThreadsStarted(false)
    , m_pLsMessageHandler(new LsMessageHandler(timer(),
                           thread_system()->NewMutex()))
    , m_pHtmlParseLsiMessageHandler(
          new LsMessageHandler(timer(), thread_system()->NewMutex()))
    , m_pSharedCircularBuffer(NULL)
    , m_sHostname(hostname.as_string())
    , m_iPort(port)
{
    InitializeDefaultOptions();
    default_options()->set_beacon_url("/ls_pagespeed_beacon");
    SystemRewriteOptions *system_options =
        dynamic_cast<SystemRewriteOptions *>(default_options());
    if (system_options)
    {
        system_options->set_file_cache_clean_inode_limit(500000);
        system_options->set_avoid_renaming_introspective_javascript(true);
    }
    set_message_handler(m_pLsMessageHandler);
    set_html_parse_message_handler(m_pHtmlParseLsiMessageHandler);
}

LsRewriteDriverFactory::~LsRewriteDriverFactory()
{
    ShutDown();
    m_pSharedCircularBuffer = NULL;
    STLDeleteElements(&uninitialized_server_contexts_);
}

Hasher *LsRewriteDriverFactory::NewHasher()
{
    return new MD5Hasher;
}

UrlAsyncFetcher *LsRewriteDriverFactory::AllocateFetcher(
    SystemRewriteOptions *config)
{
    return SystemRewriteDriverFactory::AllocateFetcher(config);
}

MessageHandler *LsRewriteDriverFactory::DefaultHtmlParseMessageHandler()
{
    return m_pHtmlParseLsiMessageHandler;
}

MessageHandler *LsRewriteDriverFactory::DefaultMessageHandler()
{
    return m_pLsMessageHandler;
}

FileSystem *LsRewriteDriverFactory::DefaultFileSystem()
{
    return new StdioFileSystem();
}

Timer *LsRewriteDriverFactory::DefaultTimer()
{
    return new PosixTimer;
}

NamedLockManager *LsRewriteDriverFactory::DefaultLockManager()
{
    CHECK(false);
    return NULL;
}

RewriteOptions *LsRewriteDriverFactory::NewRewriteOptions()
{
    LsRewriteOptions *options = new LsRewriteOptions(thread_system());
    options->SetRewriteLevel(RewriteOptions::kCoreFilters);
    return options;
}

bool LsRewriteDriverFactory::InitLsiUrlAsyncFetchers()
{
    return true;
}


LsServerContext *LsRewriteDriverFactory::MakeLsServerContext(
    StringPiece hostname, int port, int uninitialized)
{
    LsServerContext *server_context = new LsServerContext(this, hostname,
            port);
    if (uninitialized)
        uninitialized_server_contexts_.insert(server_context);
    return server_context;
}

ServerContext *LsRewriteDriverFactory::NewDecodingServerContext()
{
    ServerContext *sc = new LsServerContext(this, m_sHostname, m_iPort);
    InitStubDecodingServerContext(sc);
    return sc;
}

ServerContext *LsRewriteDriverFactory::NewServerContext()
{
    LOG(DFATAL) << "MakeLsServerContext should be used instead";
    return NULL;
}

void LsRewriteDriverFactory::ShutDownMessageHandlers()
{
    m_pLsMessageHandler->set_buffer(NULL);
    m_pHtmlParseLsiMessageHandler->set_buffer(NULL);

    for (LsMessageHandlerSet::iterator p =
             m_serverContextMessageHandlers.begin();
         p != m_serverContextMessageHandlers.end(); ++p)
        (*p)->set_buffer(NULL);

    m_serverContextMessageHandlers.clear();
}

void LsRewriteDriverFactory::StartThreads()
{
    if (m_bThreadsStarted)
        return;

    SchedulerThread *thread = new SchedulerThread(thread_system(),
            scheduler());
    bool ok = thread->Start();
    CHECK(ok) << "Unable to start scheduler thread";
    defer_cleanup(thread->MakeDeleter());
    m_bThreadsStarted = true;
}

void LsRewriteDriverFactory::LoggingInit()
{
    InstallLogMessageHandler();

    if (install_crash_handler())
        LsMessageHandler::InstallCrashHandler();
}

void LsRewriteDriverFactory::SetCircularBuffer(
    SharedCircularBuffer *buffer)
{
    m_pSharedCircularBuffer = buffer;
    m_pLsMessageHandler->set_buffer(buffer);
    m_pHtmlParseLsiMessageHandler->set_buffer(buffer);
}

void LsRewriteDriverFactory::SetServerContextMessageHandler(
    ServerContext *server_context)
{
    LsMessageHandler *handler = new LsMessageHandler(
        timer(), thread_system()->NewMutex());
    // The lsi_shared_circular_buffer_ will be NULL if MessageBufferSize hasn't
    // been raised from its default of 0.
    handler->set_buffer(m_pSharedCircularBuffer);
    m_serverContextMessageHandlers.insert(handler);
    defer_cleanup(new Deleter<LsMessageHandler> (handler));
    server_context->set_message_handler(handler);
}

void LsRewriteDriverFactory::InitStats(Statistics *statistics)
{
    // Init standard PSOL stats.
    SystemRewriteDriverFactory::InitStats(statistics);
    RewriteDriverFactory::InitStats(statistics);
    RateController::InitStats(statistics);

    // Init Lsi-specific stats.
    LsServerContext::InitStats(statistics);
    InPlaceResourceRecorder::InitStats(statistics);
}

}  // namespace net_instaweb
