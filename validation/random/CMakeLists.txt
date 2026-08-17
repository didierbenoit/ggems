# Random stream generator
add_executable(ggems_random_stream_generator
               tools/GGEMSRandomStreamGenerator.cc)

target_link_libraries(ggems_random_stream_generator
                      PRIVATE ggems_core GGEMS::CompilerOptions)

target_include_directories(ggems_random_stream_generator
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(
  ggems_random_stream_generator
  PRIVATE
    GGEMS_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels"
    GGEMS_VALIDATION_RANDOM_KERNEL_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/kernels"
    GGEMS_VALIDATION_RANDOM_RESULT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/results"
    GGEMS_VALIDATION_RANDOM_STREAM_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/streams")

ggems_apply_release_optimizations(ggems_random_stream_generator)
