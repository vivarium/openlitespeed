
set(SOURCES
        appconfig.cpp
        extappregistry.cpp
)

set(HEADERS
        appconfig.h
        extappregistry.h
)

add_library(extensions-registry
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(extensions-registry
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(extensions-registry
        PUBLIC
            lsr util
        PRIVATE
            extensions extensions-cgi extensions-fcgi extensions-jk extensions-lsapi extensions-proxy
            http log4cxx main socket
)