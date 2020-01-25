
set(HEADERS adns.h)
set(SOURCES adns.cpp)

add_library(adns
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(adns
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
            "${UDNS_INCLUDE_DIRS}"
)

target_link_libraries(adns
        PUBLIC
            lsr edio util
        PRIVATE
            log4cxx shm ${UDNS_LIBRARIES}
)