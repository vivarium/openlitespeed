
set(SOURCES modreqparser.cpp)

add_library(mod_reqparser
        MODULE
            ${SOURCES}
)

target_include_directories(mod_reqparser
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

set_target_properties(mod_reqparser PROPERTIES PREFIX "")

target_link_libraries(mod_reqparser
        PRIVATE
            lsr util
)

install(TARGETS mod_reqparser
        LIBRARY DESTINATION lib/lshttpd/modules
)