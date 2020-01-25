
set(HEADERS
        addrmap.h
        lsshm.h
        lsshmhash.h
        lsshmhashobserver.h
        lsshmlock.h
        lsshmpool.h
        lsshmtidmgr.h
)

set(SOURCES
        addrmap.cpp
        lsshm.cpp
        lsshmhash.cpp
        lsshmhashobserver.cpp
        lsshmlock.cpp
        lsshmpool.cpp
        lsshmtidmgr.cpp
)

add_library(shm
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(shm
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(shm
        PUBLIC
            lsr util
        PRIVATE
            log4cxx
)
