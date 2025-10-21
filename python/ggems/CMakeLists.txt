set(PYTHON_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/python/ggems)

# Get C++ file to bind Python
file(GLOB GGEMS_BINDINGS_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/bindings/*.cc
)

# Build Python module
pybind11_add_module(ggems_py ${GGEMS_BINDINGS_FILES})
target_link_libraries(ggems_py PRIVATE ggems CUDA::OpenCL)
target_include_directories(ggems_py PRIVATE ${PROJECT_SOURCE_DIR}/include)

set_target_properties(ggems_py PROPERTIES
  OUTPUT_NAME "ggems"
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_OUTPUT_DIR}
)

# Installation
install(FILES __init__.py DESTINATION ggems)
install(TARGETS ggems_py LIBRARY DESTINATION ggems)
