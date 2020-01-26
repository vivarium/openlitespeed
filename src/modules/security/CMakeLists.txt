
set(SOURCES mod_security.cpp)

add_library(mod_security
        MODULE
            ${SOURCES}
)

set_target_properties(mod_security PROPERTIES PREFIX "")

target_link_libraries(mod_security
        PRIVATE
            lsr
)

install(TARGETS mod_security
        LIBRARY DESTINATION lib/lshttpd/modules
)