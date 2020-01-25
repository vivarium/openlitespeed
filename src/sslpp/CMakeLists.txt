
set(SOURCES
        ls_fdbuf_bio.c
        sslasyncpk.cpp
        sslcert.cpp
        sslcertcomp.cpp
        sslconnection.cpp
        sslcontext.cpp
        sslcontextconfig.cpp
        sslengine.cpp
        sslerror.cpp
        sslocspstapling.cpp
        sslsesscache.cpp
        sslticket.cpp
        sslutil.cpp
        ocsp/ocsp.c
)

set(HEADERS
        hiocrypto.h
        ls_fdbuf_bio.h
        sslasyncpk.h
        sslcert.h
        sslcertcomp.h
        sslconnection.h
        sslcontext.h
        sslcontextconfig.cpp
        ssldef.h
        sslengine.h
        sslerror.h
        sslocspstapling.h
        sslsesscache.h
        sslticket.h
        sslutil.h
        ocsp/ocsp.h
)

add_library(sslpp
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(sslpp
        PRIVATE
        "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(sslpp
        PUBLIC
            lsr shm socket util OpenSSL::SSL
        PRIVATE
            log4cxx Threads::Threads
)