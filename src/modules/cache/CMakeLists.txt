
set(SOURCES
        cache.cpp
        cacheconfig.cpp
        cachectrl.cpp
        cacheentry.cpp
        cachehash.cpp
        cachemanager.cpp
        cachestore.cpp
        ceheader.cpp
        dirhashcacheentry.cpp
        dirhashcachestore.cpp
        shmcachemanager.cpp
)

set(HEADERS
        cacheconfig.h
        cachectrl.h
        cacheentry.h
        cachehash.h
        cachemanager.h
        cachestore.h
        ceheader.h
        dirhashcacheentry.h
        dirhashcachestore.h
        shmcachemanager.h
)

add_library(mod_cache
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(mod_cache
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src/"
)

target_link_libraries(mod_cache
        PRIVATE
            lsr http log4cxx shm util ZLIB::ZLIB
)