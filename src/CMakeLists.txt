
configure_file(
        "${OpenLiteSpeed_SOURCE_DIR}/src/config.h.in"
        "${OpenLiteSpeed_SOURCE_DIR}/src/config.h"
)

if(ENABLE_GEOIP)
    find_package(GeoIP REQUIRED)
    find_package(MaxMind REQUIRED)
endif()

if(ENABLE_IP2LOCATION)
    find_package(IP2Location REQUIRED)
endif()

if(ENABLE_BROTLI)
    find_package(Brotli REQUIRED)
endif()

add_subdirectory(adns)
add_subdirectory(edio)
add_subdirectory(extensions)
add_subdirectory(http)
add_subdirectory(log4cxx)
add_subdirectory(lsiapi)
add_subdirectory(lsr)
add_subdirectory(main)
add_subdirectory(modules)
add_subdirectory(shm)
add_subdirectory(socket)
add_subdirectory(spdy)
add_subdirectory(ssi)
add_subdirectory(sslpp)
add_subdirectory(thread)
add_subdirectory(util)
