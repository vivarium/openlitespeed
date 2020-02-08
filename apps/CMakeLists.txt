
#########################
##### OpenLiteSpeed #####
#########################

add_executable(openlitespeed
        openlitespeed.cpp
)

target_include_directories(openlitespeed
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(openlitespeed
        PRIVATE
            main
)

#############################
#####   ls_shmhashcmp   #####
#############################
add_executable(shmhashcmp ls_shmhashcmp.cpp)

target_include_directories(shmhashcmp
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(shmhashcmp
        PRIVATE
            shm util
)

#############################
#####   ls_shmhashstat  #####
#############################

add_executable(shmhashstat ls_shmhashstat.cpp)

target_include_directories(shmhashstat
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(shmhashstat
        PRIVATE
            shm
)

#########################
#####   ls_shmstat  #####
#########################

add_executable(shmstat ls_shmstat.cpp)

target_include_directories(shmstat
        PRIVATE
           "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(shmstat
        PRIVATE
            shm
)


install(TARGETS openlitespeed shmhashcmp shmhashstat shmstat
        RUNTIME DESTINATION bin
)

#########################
#####   lswsctrl    #####
#########################

install(FILES "${CMAKE_BINARY_DIR}/apps/lswsctrl"
        DESTINATION bin
)