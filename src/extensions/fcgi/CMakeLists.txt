
set(SOURCES
        fcgiapp.cpp
        fcgiappconfig.cpp
        fcgiconnection.cpp
        fcgienv.cpp
        fcginamevaluepair.cpp
        fcgirecord.cpp
)

set(HEADERS
        fcgiapp.h
        fcgiappconfig.h
        fcgiconnection.h
        fcgidef.h
        fcgienv.h
        fcginamevaluepair.h
        fcgirecord.h
        fcgireqlist.h
        fcgirequest.h
        fcgistarter.h
        samplefcgiextconn.h
)

add_library(extensions-fcgi
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(extensions-fcgi
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(extensions-fcgi
        PUBLIC
            lsr extensions util
        PRIVATE
            edio extensions-registry http log4cxx main
)