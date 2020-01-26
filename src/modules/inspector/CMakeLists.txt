
set(SOURCES modinspector.cpp)

add_library(mod_inspector
        MODULE
            ${SOURCES}
)

set_target_properties(mod_inspector PROPERTIES PREFIX "")

target_link_libraries(mod_inspector
        PRIVATE
            lsr
)

install(TARGETS mod_inspector
        LIBRARY DESTINATION lib/lshttpd/modules
)