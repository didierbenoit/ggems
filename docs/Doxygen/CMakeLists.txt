# ============================================================================
# ============================================================================

# --- Section: Options and dependencies --------------------------------------
find_package(Doxygen REQUIRED)

# --- Section: Doxygen configuration ----------------------------------------
set(DOXYGEN_PROJECT_LOGO
    "${CMAKE_CURRENT_SOURCE_DIR}/images/ggems_logo_128_128.png")
set(DOXYGEN_PROJECT_ICON
    "${CMAKE_CURRENT_SOURCE_DIR}/images/ggems_logo_32_32.png")

set(DOXYGEN_INPUT_DIRS
    "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/kernels"
    "${PROJECT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox")
string(REPLACE ";" " " DOXYGEN_INPUT_DIRS "${DOXYGEN_INPUT_DIRS}")

set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Configure paths to custom templates and styles
set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/header.html")
set(DOXYGEN_HTML_FOOTER "${CMAKE_CURRENT_SOURCE_DIR}/footer.html")
set(DOXYGEN_HTML_STYLESHEET "${CMAKE_CURRENT_SOURCE_DIR}/customdoxygen.css")
set(DOXYGEN_HTML_EXTRA_STYLESHEET "${CMAKE_CURRENT_SOURCE_DIR}/mycustom.css")

set(DOXYGEN_LATEX_DIR "${DOXYGEN_OUTPUT_DIR}/latex")

# Generate a temporary Doxygen configuration file
set(DOXYGEN_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile" "${DOXYGEN_CONFIG_FILE}"
               @ONLY)

# --- Run Doxygen immediately when docs are built ----------------------------
add_custom_target(
  docs ALL
  COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating GGEMS documentation with Doxygen"
  VERBATIM)

# --- Section: Sphinx (optional future integration) --------------------------
# To enable Sphinx documentation generation (narrative + Python docs), uncomment
# the following lines and ensure Sphinx + Breathe are installed.
#
# find_package(Sphinx) if(Sphinx_FOUND) add_custom_target(sphinx_docs COMMAND
# ${SPHINX_EXECUTABLE} -b html "${CMAKE_CURRENT_SOURCE_DIR}/Sphinx/source"
# "${CMAKE_BINARY_DIR}/docs/Sphinx" COMMENT "Generating Sphinx documentation for
# GGEMS" ) add_dependencies(sphinx_docs docs) endif()

# --- Section: Final messages ------------------------------------------------
message(STATUS "Doxygen input directory : ${DOXYGEN_INPUT_DIRS}")
message(STATUS "Doxygen output directory: ${DOXYGEN_OUTPUT_DIR}")
