add_executable(ggems_sphere_precision GGEMSSpherePrecision.cc)
target_link_libraries(ggems_sphere_precision PRIVATE ggems_core GGEMS::CompilerOptions)
target_compile_definitions(ggems_sphere_precision PRIVATE
  GGEMS_E1_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels"
  GGEMS_E1_PROBE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/sphere_precision_probe.cl"
  GGEMS_E1_BUILD_CONFIGURATION="$<CONFIG>")
ggems_apply_release_optimizations(ggems_sphere_precision)
