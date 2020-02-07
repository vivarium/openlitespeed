
configure_file(
        "${OpenLiteSpeed_SOURCE_DIR}/conf/httpd_config.conf.in"
        "${CMAKE_BINARY_DIR}/conf/httpd_config.conf"
)

configure_file(
        "${OpenLiteSpeed_SOURCE_DIR}/conf/lsws_env.in"
        "${CMAKE_BINARY_DIR}/conf/lsws_env"
)

install(FILES
            "${CMAKE_BINARY_DIR}/conf/httpd_config.conf"
            "${CMAKE_BINARY_DIR}/conf/lsws_env"
            "${OpenLiteSpeed_SOURCE_DIR}/conf/mime.properties"
        DESTINATION etc/lshttpd
)