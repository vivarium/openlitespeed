cmake_minimum_required(VERSION 3.10...3.16)

set(MAJOR_VERSION 1)
set(MINOR_VERSION 5)
set(PATCH_VERSION 11)

set(VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

project(OpenLiteSpeed VERSION ${VERSION}
                      DESCRIPTION "High-performance, lightweight, open source HTTP server"
                      LANGUAGES C CXX
        )

set(CMAKE_MODULE_PATH "${OpenLiteSpeed_SOURCE_DIR}/cmake")

find_package(ZLIB REQUIRED)
find_package(LuaJit REQUIRED)
find_package(Threads REQUIRED)
find_package(PCRE REQUIRED)
find_package(UDNS REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(EXPAT REQUIRED)
find_package(Crypt REQUIRED)
find_package(PHP REQUIRED)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

endif()

set(TMP_DIR
        "/tmp/lshttpd"
        CACHE STRING
        "The default temporary directory."
        FORCE
)

set(PID_FILE
        "/var/run/lshttpd/openlitespeed.pid"
        CACHE STRING
        "The OpenLiteSpeed's pid file."
        FORCE
)

set(LS_ENABLE_SPDY
        1
        CACHE BOOL
        "Choose if enable SPDY"
        FORCE
)

option(ENABLE_GEOIP "Choose whether to enable GeoIp" ON)
option(ENABLE_IP2LOCATION "Choose whether to enable IP2Location" ON)
option(ENABLE_BROTLI "Choose whether to enable Brotli" ON)
option(BUILD_TESTS "Choose whether to build tests." ON)
option(BUILD_DOCS "Choose whether to build docs." ON)

add_subdirectory(src)
add_subdirectory(apps)

if (BUILD_TESTS)
    include(CTest)
    add_subdirectory(test)
endif()

if (BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    add_subdirectory(doc)
endif()