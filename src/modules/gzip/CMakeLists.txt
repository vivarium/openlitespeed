
set(SOURCES modgzip.cpp)

add_library(mod_gzip
        STATIC
            ${SOURCES}
)

target_link_libraries(mod_gzip
        PRIVATE
            lsr ZLIB::ZLIB
)