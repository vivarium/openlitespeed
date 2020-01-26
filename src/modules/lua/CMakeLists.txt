
set(SOURCES
        edluastream.cpp
        lsluaapi.cpp
        lsluaengine.cpp
        lsluaheader.cpp
        lsluaregex.cpp
        lsluasession.cpp
        lsluashared.cpp
        modlua.cpp
)

set(HEADERS
        edluastream.h
        lsluaapi.h
        lsluadefs.h
        lsluaengine.h
        lsluasession.h
)

add_library(mod_lua
        MODULE
            ${SOURCES}
            ${HEADERS}
)

target_include_directories(mod_lua
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
            "${LUAJIT_INCLUDE_DIR}"
)

set_target_properties(mod_lua PROPERTIES PREFIX "")

target_link_libraries(mod_lua
        PUBLIC
            "${LUAJIT_LIBRARIES}"
        PRIVATE
            lsr edio log4cxx shm socket util ${CMAKE_DL_LIBS}
)

install(TARGETS mod_lua
        LIBRARY DESTINATION lib/lshttpd/modules
)