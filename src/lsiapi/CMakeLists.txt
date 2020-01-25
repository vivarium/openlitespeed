
set(SOURCES
        ediohandler.cpp
        envmanager.cpp
        lsiapi.cpp
        lsiapi_const.cpp
        lsiapigd.cpp
        lsiapihooks.cpp
        lsiapilib.cpp
        lsimoduledata.cpp
        modulehandler.cpp
        modulemanager.cpp
        moduletimer.cpp
        prelinkedmods.cpp
)

set(HEADERS
        ediohandler.h
        envmanager.h
        internal.h
        lsiapi.h
        lsiapi_const.h
        lsiapigd.h
        lsiapihooks.h
        lsiapilib.h
        lsimoduledata.h
        modulehandler.h
        modulemanager.h
        moduletimer.h
)

add_library(lsiapi
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(lsiapi
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(lsiapi
        PUBLIC
            lsr http util
        PRIVATE
            edio log4cxx main sslpp thread mod_cache mod_gzip OpenSSL::SSL ${CMAKE_DL_LIBS}
)