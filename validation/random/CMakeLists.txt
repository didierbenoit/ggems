# ============================================================================
# @file      CMakeLists.txt @brief     Random number validation tools for GGEMS.
# @details   This file declares optional executables used to generate
# reproducible random streams for scientific validation.
# ============================================================================

# ----------------------------------------------------------------------------
# Random stream generator
# ----------------------------------------------------------------------------
add_executable(ggems_random_stream_generator
               tools/GGEMSRandomStreamGenerator.cc)

target_link_libraries(ggems_random_stream_generator PRIVATE ggems_core)

target_include_directories(
  ggems_random_stream_generator PRIVATE ${PROJECT_SOURCE_DIR}/include
                                        ${PROJECT_SOURCE_DIR}/src/frameworks)

target_compile_definitions(
  ggems_random_stream_generator
  PRIVATE
    GGEMS_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels"
    GGEMS_VALIDATION_RANDOM_KERNEL_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/kernels"
    GGEMS_VALIDATION_RANDOM_RESULT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/results"
    GGEMS_VALIDATION_RANDOM_STREAM_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/streams")

ggems_apply_release_optimizations(ggems_random_stream_generator)
