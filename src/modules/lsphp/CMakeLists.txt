
set(SOURCES mod_lsphp.c)

set(HEADERS mod_lsphp.h)

add_library(mod_lsphp
        MODULE
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(mod_lsphp
        PUBLIC
            /usr/include/php/main
            /usr/include/php/Zend
            /usr/include/php/ext
            /usr/include/php/ext/date/lib
            /usr/include/php/TSRM
            /usr/include/php
)

set_target_properties(mod_lsphp PROPERTIES PREFIX "")

target_link_libraries(mod_lsphp
        PRIVATE
            lsr
)