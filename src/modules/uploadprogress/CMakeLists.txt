
set(SOURCES
        uploadprogress.cpp
)

add_library(mod_uploadprogress
        MODULE
            "${SOURCES}"
)

set_target_properties(mod_uploadprogress PROPERTIES PREFIX "")

target_link_libraries(mod_uploadprogress
        PRIVATE
            lsr
)

install(TARGETS mod_uploadprogress
        LIBRARY DESTINATION lib/lshttpd/modules
)