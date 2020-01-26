
set(LSIAPI_DOC_DIR "${CMAKE_BINARY_DIR}/doc/liblsr")
set(OPENLITESPEED_DOC_DIR "${OpenLiteSpeed_SOURCE_DIR}/doc/openlitespeed")

configure_file(
        "${OpenLiteSpeed_SOURCE_DIR}/doc/lsiapi/lsiapi.doxc.in"
        "${CMAKE_BINARY_DIR}/lsiapi.doxc"
)


add_custom_target(lsiapi_doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/lsiapi.doxc"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
)

install(DIRECTORY "${LSIAPI_DOC_DIR}"
        DESTINATION share/doc
)

install(DIRECTORY "${OPENLITESPEED_DOC_DIR}"
        DESTINATION share/doc
)