
set(SOURCES
        proxyconfig.cpp
        proxyconn.cpp
        proxyworker.cpp
)

set(HEADERS
        proxyconfig.h
        proxyconn.h
        proxyworker.h
)

add_library(extensions-proxy
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(extensions-proxy
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(extensions-proxy
        PUBLIC
            lsr extensions
        PRIVATE
            edio http log4cxx sslpp
)