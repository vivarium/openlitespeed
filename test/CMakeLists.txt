
set(EDIO_TEST
        edio/bufferedostest.cpp
)

set(HTTP_TEST
        http/chunkistest.cpp
        http/chunkostest.cpp
        #http/contexttreebench.cpp Maybe remove
        http/contexttreetest.cpp
        http/datetimetest.cpp
        http/denieddirtest.cpp
        http/expirestest.cpp
        http/httpbuftest.cpp
        http/httpcgitooltest.cpp
        #http/httpheadertest.cpp
        http/httpiptogeo2test.cpp
        http/httplistenerstest.cpp
        http/httpmimetest.cpp
        #http/httprangetest.cpp
        http/httpreqheaderstest.cpp
        http/httpreqtest.cpp
        http/httprequestlinetest.cpp
        http/httpvhostlisttest.cpp
        http/reqparsertest.cpp
        http/rewritetest.cpp
        http/statusurlmaptest.cpp
)

set(LSIAPI_TEST
        lsiapi/envhandler.cpp
        lsiapi/lsiapihookstest.cpp
        lsiapi/moduleconf.cpp
        lsiapi/moduledata.cpp
        lsiapi/moduletimertest.cpp
)

set(LSR_TEST
        lsr/ls_ahotest.cpp
        lsr/ls_base64test.cpp
        lsr/ls_buftest.cpp
        lsr/ls_confparsertest.cpp
        lsr/ls_dlinkqtest.cpp
        lsr/ls_hashtest.cpp
        lsr/ls_linktest.cpp
        lsr/ls_loopbuftest.cpp
        lsr/ls_maptest.cpp
        lsr/ls_md5test.cpp
        lsr/ls_objarraytest.cpp
        lsr/ls_objpooltest.cpp
        lsr/ls_pcregtest.cpp
        lsr/ls_pooltest.cpp
        lsr/ls_ptrlisttest.cpp
        lsr/ls_sha1test.cpp
        lsr/ls_strlisttest.cpp
        lsr/ls_strtest.cpp
        lsr/ls_strtooltest.cpp
        lsr/ls_xpooltest.cpp
)

set(SHM_TEST
        shm/shmbaselrutest.cpp
        shm/shmxtest.cpp
)

set(SOCKET_TEST
        socket/coresockettest.cpp
        socket/hostinfotest.cpp
        socket/tcpsockettest.cpp
)

set(SPDY_TEST
        spdy/dummiostream.cpp
        spdy/pushtest.cpp
        spdy/spdyconnectiontest.cpp
        spdy/spdyzlibfiltertest.cpp
)

set(THREAD_TEST
        thread/mtnotifiertest.cpp
        thread/pthreadworkqueuetest.cpp
        thread/threadtest.cpp
        thread/workcrewtest.cpp
)

set(UTIL_TEST
        util/accesscontroltest.cpp
        util/ahotest.cpp
        util/brotlibuftest.cpp
        util/dlinkqueuetest.cpp
        util/filtermatchtest.cpp
        util/ghashtest.cpp
        util/gmaptest.cpp
        util/gpathtest.cpp
        util/gpointerlisttest.cpp
        util/gzipbuftest.cpp
        #util/httpfetchtest.cpp
        util/linkedobjtest.cpp
        util/logfiletest.cpp
        util/loopbuftest.cpp
        util/objarraytest.cpp
        util/objpooltest.cpp
        util/partitioninfotest.cpp
        util/pcregextest.cpp
        util/poolalloctest.cpp
        util/radixtreetest.cpp
        util/stringmaptest.cpp
        util/stringtooltest.cpp
        util/vmembuftest.cpp
        util/xmlnodetest.cpp
)

##############################
##### OpenLiteSpeed test #####
##############################

add_executable(ols_unittest
        unittest_main.cpp
        "${EDIO_TEST}"
        "${HTTP_TEST}"
        "${LSIAPI_TEST}"
        "${LSR_TEST}"
        "${SHM_TEST}"
        "${SOCKET_TEST}"
        "${SPDY_TEST}"
        "${THREAD_TEST}"
        "${UTIL_TEST}"
)

target_include_directories(ols_unittest
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
            "${OpenLiteSpeed_SOURCE_DIR}/test"
)

target_compile_definitions(ols_unittest
        PRIVATE
            RUN_TEST #ENABLE_IPTOGEO2
)

target_link_libraries(ols_unittest
        PRIVATE
            http util lsiapi edio UnitTest++
)

##############################
#####     lsr lfqueue    #####
##############################

add_executable(ls_lfqueuetest
        lsr/ls_lfqueuetest.c
)

target_link_libraries(ls_lfqueuetest
        PRIVATE
            lsr thread Threads::Threads ${PCRE_LIBRARIES}
)

##############################
#####     lsr lfstack    #####
##############################

add_executable(ls_lfstacktest
        lsr/ls_lfstacktest.cpp
)

target_include_directories(ls_lfstacktest
        PRIVATE
            ${OpenLiteSpeed_SOURCE_DIR}/src
)

target_link_libraries(ls_lfstacktest
        PRIVATE
            lsr thread util Threads::Threads ${PCRE_LIBRARIES}
)

##############################
#####     lsr llmq       #####
##############################

add_executable(ls_llmqtest
        lsr/ls_llmqtest.c
)

target_link_libraries(ls_llmqtest
        PRIVATE
            lsr thread Threads::Threads ${PCRE_LIBRARIES}
)

##############################
#####     lsr lock       #####
##############################

add_executable(ls_locktest
        lsr/ls_locktest.cpp
)

target_link_libraries(ls_locktest
        PRIVATE
            lsr thread Threads::Threads ${PCRE_LIBRARIES}
)

##############################
#####     lsr stack      #####
##############################

add_executable(ls_stacktest
        lsr/ls_stacktest.cpp
)

target_include_directories(ls_stacktest
        PRIVATE
            ${OpenLiteSpeed_SOURCE_DIR}/src
)

target_link_libraries(ls_stacktest
        PRIVATE
            lsr thread util Threads::Threads ${PCRE_LIBRARIES}
)

##############################
#####     lsr thrsafe    #####
##############################

add_executable(ls_thrsafetest
        lsr/ls_thrsafetest.cpp
)

target_include_directories(ls_thrsafetest
        PRIVATE
            ${OpenLiteSpeed_SOURCE_DIR}/src
)

target_link_libraries(ls_thrsafetest
        PRIVATE
            lsr thread Threads::Threads ${PCRE_LIBRAREIS}
)

add_test(OpenLiteSpeed ols_unittest)
add_test(lsr/lfqueue ls_lfqueuetest)
add_test(lsr/lfstack ls_lfstacktest)
add_test(lsr/llmq ls_llmqtest)
add_test(lsr/lock ls_locktest)
add_test(lsr/stack ls_stacktest)
add_test(lsr/thrsafe ls_thrsafetest)