
set(SOURCES
        ssiconfig.cpp
        ssiengine.cpp
        ssiruntime.cpp
        ssiscript.cpp
)

set(HEADERS
        ssiconfig.h
        ssiengine.h
        ssiruntime.h
        ssiscript.h
)

add_library(ssi
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(ssi
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
            "${PCRE_INCLUDE_DIRS}"
)

target_link_libraries(ssi
        PUBLIC
            lsr http util
        PRIVATE
            log4cxx ${PCRE_LIBRARIES}
)
