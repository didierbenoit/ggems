add_executable(ggems_radionuclide_exporter tools/GGEMSRadionuclideExporter.cc)

target_link_libraries(ggems_radionuclide_exporter
                      PRIVATE ggems_core GGEMS::CompilerOptions)

target_compile_definitions(
  ggems_radionuclide_exporter
  PRIVATE
    GGEMS_VALIDATION_COMPILER="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
)

ggems_apply_release_optimizations(ggems_radionuclide_exporter)

add_custom_target(validation_radioactivity DEPENDS ggems_radionuclide_exporter)
