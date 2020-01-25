
add_subdirectory(cgi)
add_subdirectory(fcgi)
add_subdirectory(jk)
add_subdirectory(lsapi)
add_subdirectory(proxy)
add_subdirectory(registry)

SET(SOURCES
        extconn.cpp
        extrequest.cpp
        extworker.cpp
        extworkerconfig.cpp
        httpextprocessor.cpp
        iprocessortimer.cpp
        l4conn.cpp
        loadbalancer.cpp
        localworker.cpp
        localworkerconfig.cpp
        pidlist.cpp
)

SET(HEADERS
        detached.h
        extconn.h
        extrequest.h
        extworker.h
        extworkerconfig.h
        httpextprocessor.h
        iprocessortimer.h
        l4conn.h
        loadbalancer.h
        localworker.h
        localworkerconfig.h
        pidlist.h
)

add_library(extensions
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(extensions
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(extensions
        PUBLIC
            lsr edio http log4cxx util
        PRIVATE
            main socket
)
