# ------------------------------------------------------------------------------
# Source validation uses the ordinary production Run and Source path.
add_executable(ggems_source_sample_exporter tools/GGEMSSourceSampleExporter.cc)

target_link_libraries(ggems_source_sample_exporter
                      PRIVATE ggems_core GGEMS::CompilerOptions)

ggems_apply_release_optimizations(ggems_source_sample_exporter)

add_custom_target(validation_source DEPENDS ggems_source_sample_exporter)
