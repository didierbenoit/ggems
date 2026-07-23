# ----------------------------------------------------------------------------
# Collect all Python binding sources
# ----------------------------------------------------------------------------
file(GLOB_RECURSE GGEMS_PYBIND_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/bindings.cc
     ${CMAKE_CURRENT_SOURCE_DIR}/ggems/bindings_*.cc)

# ----------------------------------------------------------------------------
# Define the Python extension module
# ----------------------------------------------------------------------------
pybind11_add_module(ggems MODULE ${GGEMS_PYBIND_SOURCES})

# ----------------------------------------------------------------------------
# Ensure the compiled module is placed in the package directory
# ----------------------------------------------------------------------------
set_target_properties(
  ggems
  PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ggems"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ggems"
             ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ggems")

# ----------------------------------------------------------------------------
# Link with the GGEMS core library
# ----------------------------------------------------------------------------
target_link_libraries(ggems PRIVATE ggems_core)

if(GGEMS_WITH_IMGUI)
  target_link_libraries(ggems PRIVATE ggems_gui)
  target_compile_definitions(ggems PRIVATE GGEMS_WITH_IMGUI)
endif()

# ----------------------------------------------------------------------------
# Include GGEMS headers
# ----------------------------------------------------------------------------
target_include_directories(ggems PRIVATE ${PROJECT_SOURCE_DIR}/include)

# ----------------------------------------------------------------------------
# Language and Release optimization policy
# ----------------------------------------------------------------------------

target_compile_features(ggems PRIVATE cxx_std_23)

ggems_apply_release_optimizations(ggems)
