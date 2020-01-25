
set(SOURCES
        ls_base_fetch.cpp
        ls_caching_headers.cpp
        ls_message_handler.cpp
        ls_rewrite_driver_factory.cpp
        ls_rewrite_options.cpp
        ls_server_context.cpp
        ls_uamatcher.cpp
)

set(HEADERS
        log_message_handler.h
        ls_base_fetch.h
        ls_caching_headers.h
        ls_message_handler.h
        ls_rewrite_driver_factory.h
        ls_rewrite_options.h
        ls_server_context.h
        ls_uamatcher.h
        pagespeed.h
)

add_library(mod_pagespeed
        MODULE
            "${SOURCES}"
            "${HEADERS}"
)

set_target_properties(mod_pagespeed PROPERTIES PREFIX "")

target_link_libraries(mod_pagespeed
        PUBLIC
            Threads::Threads
        PRIVATE
            lsr http util
)