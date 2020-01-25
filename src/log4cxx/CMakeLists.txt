
set(HEADERS
        appender.h
        appendermanager.h
        fileappender.h
        ilog.h
        layout.h
        level.h
        logger.h
        loggingevent.h
        logrotate.h
        logsession.h
        patternlayout.h
        tmplogid.h
)

set(SOURCES
        appender.cpp
        appendermanager.cpp
        fileappender.cpp
        ilog.cpp
        layout.cpp
        level.cpp
        logger.cpp
        loggingevent.cpp
        logrotate.cpp
        logsession.cpp
        patternlayout.cpp
        tmplogid.cpp
)

add_library(log4cxx
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(log4cxx
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(log4cxx
        PUBLIC
            lsr util
        PRIVATE
            edio
)