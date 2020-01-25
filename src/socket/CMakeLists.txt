
set(SOURCES
        coresocket.cpp
        gsockaddr.cpp
        hostinfo.cpp
        serversocket.cpp
        streamsocket.cpp
        tcpconnection.cpp
        tcpserversocket.cpp
        tcpsockopt.cpp
)

set(HEADERS
        coresocket.h
        datagramsocket.h
        errorno.h
        filecntl.h
        gsockaddr.h
        hostinfo.h
        ipv4addr.h
        ls_sock.h
        rawsocket.h
        serversocket.h
        sockdef.h
        streamsocket.h
        tcpconnection.h
        tcpserversocket.h
        tcpsockopt.h
        udpsocket.h
)

add_library(socket
        STATIC
            "${SOURCES}"
            "${HEADERS}"
)

target_include_directories(socket
        PRIVATE
        "${OpenLiteSpeed_SOURCE_DIR}/src"
)

target_compile_definitions(socket
        PRIVATE
            USE_UDNS
)

target_link_libraries(socket
        PUBLIC
            lsr
        PRIVATE
            adns util
)