
set(SOURCES
        aiooutputstream.cpp
        aiosendfile.cpp
        bufferedos.cpp
        cacheos.cpp
        devpoller.cpp
        ediostream.cpp
        epoll.cpp
        eventnotifier.cpp
        eventprocessor.cpp
        eventreactor.cpp
        evtcbque.cpp
        fdindex.cpp
        flowcontrol.cpp
        inputstream.cpp
        iochain.cpp
        kqueuer.cpp
        lookupfd.cpp
        multiplexer.cpp
        multiplexerfactory.cpp
        outputbuf.cpp
        outputstream.cpp
        poller.cpp
        pollfdreactor.cpp
        reactorindex.cpp
        rtsigio.cpp
        sigeventdispatcher.cpp
)

set(HEADERS
        aioeventhandler.h
        aiooutputstream.h
        aiosendfile.h
        bufferedos.h
        cacheos.h
        callbackqueue.h
        devpoller.h
        ediostream.h
        epoll.h
        eventnotifier.h
        eventprocessor.h
        eventreactor.h
        evtcbque.h
        fdindex.h
        flowcontrol.h
        inputstream.h
        iochain.h
        kqueuer.h
        lookupfd.h
        multiplexer.h
        multiplexerfactory.h
        outputbuf.h
        outputstream.h
        poller.h
        pollfdreactor.h
        reactorindex.h
        rtsigio.h
        sigeventdispatcher.h
)

add_library(edio
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(edio
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(edio
        PUBLIC
            lsr thread util rt
        PRIVATE
            http log4cxx lsiapi socket
)