
SET(SOURCES
        crewworker.cpp
        mtnotifier.cpp
        pthreadcond.cpp
        pthreadmutex.cpp
        pthreadworkqueue.cpp
        thread.cpp
        workcrew.cpp
        worker.cpp
)

add_library(thread
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(thread
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(thread
        PUBLIC
            lsr util Threads::Threads
        PRIVATE
            edio log4cxx
)