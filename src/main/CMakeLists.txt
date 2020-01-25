
set(SOURCES
        configctx.cpp
        httpconfigloader.cpp
        httpserver.cpp
        lshttpdmain.cpp
        mainserverconfig.cpp
        plainconf.cpp
        serverinfo.cpp
        zconfclient.cpp
        zconfmanager.cpp
)

set(HEADERS
        configctx.h
        httpconfigloader.h
        httpserver.h
        lshttpdmain.h
        mainserverconfig.h
        plainconf.h
        serverinfo.h
        timestamp.h
        zconfclient.h
        zconfmanager.h
)

add_library(main
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(main
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(main
        PUBLIC
            lsr extensions http log4cxx util
        PRIVATE
            adns edio extensions-cgi extensions-fcgi extensions-jk extensions-registry
            lsiapi shm socket sslpp OpenSSL::SSL
)
