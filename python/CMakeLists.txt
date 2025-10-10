set(PYTHON_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/python/ggems)

# --- Module tools ---
pybind11_add_module(ggems_tools_py ggems/GGEMSToolsBindings.cc)
set_target_properties(ggems_tools_py PROPERTIES
  OUTPUT_NAME "ggems_tools"
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_OUTPUT_DIR}
)
target_link_libraries(ggems_tools_py PRIVATE ggems_tools)
target_include_directories(ggems_tools_py PRIVATE ${PROJECT_SOURCE_DIR}/include)

# --- Installation step ---
# Copy __init__.py to python install directory
install(FILES ggems/__init__.py DESTINATION ggems)

# Install modules
install(TARGETS ggems_tools_py LIBRARY DESTINATION ggems)
