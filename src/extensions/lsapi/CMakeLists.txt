
set(SOURCES
        lsapiconfig.cpp
        lsapiconn.cpp
        lsapireq.cpp
        lsapiworker.cpp
)

set(HEADERS
        lsapiconfig.h
        lsapiconn.h
        lsapidef.h
        lsapireq.h
        lsapiworker.h
)

add_library(extensions-lsapi
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(extensions-lsapi
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(extensions-lsapi
        PUBLIC
            lsr extensions util
        PRIVATE
            edio extensions-registry http log4cxx main
)