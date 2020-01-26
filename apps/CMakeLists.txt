
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
add_executable(ls_shmhashcmp ls_shmhashcmp.cpp)

target_include_directories(ls_shmhashcmp
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(ls_shmhashcmp
        PRIVATE
            shm util
)

#############################
#####   ls_shmhashstat  #####
#############################

add_executable(ls_shmhashstat ls_shmhashstat.cpp)

target_include_directories(ls_shmhashstat
        PRIVATE
            "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(ls_shmhashstat
        PRIVATE
            shm
)

#########################
#####   ls_shmstat  #####
#########################

add_executable(ls_shmstat ls_shmstat.cpp)

target_include_directories(ls_shmstat
        PRIVATE
           "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_link_libraries(ls_shmstat
        PRIVATE
            shm
)

install(TARGETS openlitespeed ls_shmhashcmp ls_shmhashstat ls_shmstat
        RUNTIME DESTINATION bin
)