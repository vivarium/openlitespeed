
set(LSIAPI_DOC_DIR "${CMAKE_BINARY_DIR}/doc/liblsr")
set(DOC_DIR "${OpenLiteSpeed_SOURCE_DIR}/doc")

configure_file(
        "${OpenLiteSpeed_SOURCE_DIR}/doc/liblsr/liblsr.doxc.in"
        "${CMAKE_BINARY_DIR}/liblsr.doxc"
)


add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/liblsr.doxc"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
)

install(DIRECTORY "${LSIAPI_DOC_DIR}"
        DESTINATION share/doc
)

install(FILES
            "${DOC_DIR}/liblsr/ModuleDeveloperGuide.odt"
            "${DOC_DIR}/liblsr/ModuleDeveloperGuide.pdf"
        DESTINATION
            share/doc/liblsr
)

install(DIRECTORY "${DOC_DIR}/openlitespeed"
        DESTINATION share/doc
)