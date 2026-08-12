find_package(Doxygen QUIET COMPONENTS dot)

# Doxygen configuration
set(DOXYGEN_PROJECT_LOGO
    "${CMAKE_CURRENT_SOURCE_DIR}/images/ggems_logo_128_128.png")

set(DOXYGEN_PROJECT_ICON
    "${CMAKE_CURRENT_SOURCE_DIR}/images/ggems_logo_32_32.png")

set(DOXYGEN_HTML_EXTRA_STYLESHEET "${CMAKE_CURRENT_SOURCE_DIR}/mycustom.css")

set(DOXYGEN_INPUT_DIRS
    "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/kernels"
    "${PROJECT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox")

string(REPLACE ";" " " DOXYGEN_INPUT_DIRS "${DOXYGEN_INPUT_DIRS}")

set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(DOXYGEN_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile" "${DOXYGEN_CONFIG_FILE}"
               @ONLY)

# Doxygen target
if(Doxygen_FOUND)
  add_custom_target(
    doxygen
    COMMAND Doxygen::doxygen "${DOXYGEN_CONFIG_FILE}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating GGEMS documentation with Doxygen"
    VERBATIM)

  message(STATUS "Doxygen version           : ${Doxygen_VERSION}")
  message(STATUS "Doxygen input directories : ${DOXYGEN_INPUT_DIRS}")
  message(STATUS "Doxygen output directory  : ${DOXYGEN_OUTPUT_DIR}")
else()
  add_custom_target(
    doxygen
    COMMAND "${CMAKE_COMMAND}" -E echo
            "Doxygen documentation requires Doxygen and Graphviz 'dot'."
    COMMAND "${CMAKE_COMMAND}" -E false
    COMMENT "Doxygen is unavailable"
    VERBATIM)

  message(
    STATUS
      "Doxygen documentation     : unavailable (Doxygen or Graphviz 'dot' not found)"
  )
endif()
