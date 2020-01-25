
SET(SOURCES
        ls_aho.c
        ls_base64.c
        ls_buf.c
        ls_confparser.c
        ls_crc64.c
        ls_dlinkq.c
        ls_fileio.c
        ls_hash.c
        ls_lfqueue.c
        ls_lfstack.c
        ls_link.c
        ls_llmq.c
        ls_llxq.c
        ls_lock.c
        ls_loopbuf.c
        ls_map.c
        ls_md5.c
        ls_mpscq.c
        ls_objarray.c
        ls_objpool.c
        ls_pcreg.c
        ls_pool.c
        ls_ptrlist.c
        ls_sha1.c
        ls_shm.cpp
        ls_stack.c
        ls_str.c
        ls_strlist.c
        ls_strtool.c
        ls_time.c
        ls_tsstack.c
        ls_xpool.c
        offloader.cpp
        xxhash.c
)

set("HEADERS"
        "${OpenLiteSpeed_SOURCE_DIR}/include/ls.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsdef.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_aho.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_atomic.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_base64.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_buf.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_confparser.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_crc64.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_dlinkq.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_edio.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_evtcb.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_fileio.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_hash.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_lfqueue.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_lfstack.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_link.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_llmq.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_llxq.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_lock.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_log.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_loopbuf.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_map.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_md5.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_node.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_objarray.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_objpool.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_pcreg.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_pool.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_ptrlist.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_sha1.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_shm.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_stack.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_str.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_strlist.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_strtool.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_swap.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_time.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_tsstack.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_types.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_xpool.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/xxhash.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/dynamic_annotations.h"
        "${OpenLiteSpeed_SOURCE_DIR}/include/lsr/ls_threadcheck.h"
        ls_internal.h
        ls_memcheck.h
        ls_offload.h
        ls_pooldef.h
        ls_poolint.h
        ls_xpool_int.h
        xxhash.h
)

add_library(lsr
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(lsr
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${OpenLiteSpeed_SOURCE_DIR}/include>
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
            "${PCRE_INCLUDE_DIRS}"
)

target_compile_options(lsr
        PUBLIC
            -fPIC
)

target_link_libraries(lsr
        PRIVATE
            edio log4cxx shm thread util OpenSSL::SSL Threads::Threads ${PCRE_LIBRARIES}
)

install(TARGETS lsr
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)

install(DIRECTORY "${OpenLiteSpeed_SOURCE_DIR}/include/"
        DESTINATION include
        FILES_MATCHING PATTERN *.h
)