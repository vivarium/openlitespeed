
set(SOURCES
        h2connection.cpp
        h2protocol.cpp
        h2stream.cpp
        lshpack.c
        spdyconnection.cpp
        spdydebug.cpp
        spdyprotocol.cpp
        spdystream.cpp
        spdystreampool.cpp
        spdyzlibfilter.cpp
        unpackedheaders.cpp
)

set(HEADERS
        h2connection.h
        h2protocol.h
        h2stream.h
        h2streampool.h
        lshpack.h
        protocoldef.h
        spdyconnection.h
        spdydebug.h
        spdyprotocol.h
        spdystream.h
        spdystreampool.h
        spdyzlibfilter.h
        unpackedheaders.h
)

add_library(spdy
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(spdy
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(spdy
        PUBLIC
            lsr edio http util ZLIB::ZLIB
        PRIVATE
            log4cxx
)