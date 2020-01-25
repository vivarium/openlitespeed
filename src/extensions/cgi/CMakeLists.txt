
set(SOURCES
        cgidconfig.cpp
        cgidconn.cpp
        cgidreq.cpp
        cgidworker.cpp
        cgroupconn.cpp
        cgroupuse.cpp
        lscgid.cpp
        suexec.cpp
)

set(HEADERS
        cgiconnection.h
        cgidconfig.h
        cgidconn.h
        cgidreq.h
        cgidworker.h
        cgroupconn.h
        cgroupuse.h
        lscgid.h
        lscgiddef.h
        suexec.h
)

add_library(extensions-cgi
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(extensions-cgi
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(extensions-cgi
        PUBLIC
            lsr extensions util
        PRIVATE
            edio extensions-registry http log4cxx main socket OpenSSL::SSL Threads::Threads ${CMAKE_DL_LIBS}
)