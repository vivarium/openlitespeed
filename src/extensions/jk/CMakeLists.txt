
set(SOURCES
        jconn.cpp
        jkajp13.cpp
        jworker.cpp
        jworkerconfig.cpp
)

set(HEADERS
        jconn.h
        jkajp13.h
        jworker.h
        jworkerconfig.h
)

add_library(extensions-jk
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(extensions-jk
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(extensions-jk
        PUBLIC
            lsr extensions
        PRIVATE
            http log4cxx sslpp util
)