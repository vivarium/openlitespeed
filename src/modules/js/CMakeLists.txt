
set(SOURCES
        lsjsengine.cpp
        modjs.cpp
)

set(HEADERS
        lsjsengine.h
        modjs.h
)

add_library(mod_js
        MODULE
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(mod_js
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

set_target_properties(mod_js PROPERTIES PREFIX "")

target_link_libraries(mod_js
        PRIVATE
            lsr util
)
