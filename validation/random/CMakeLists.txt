# ------------------------------------------------------------------------------
# Random validation build target
add_custom_target(validation_random)

# ------------------------------------------------------------------------------
# Validation-private raw uint32 chunk producer
add_library(ggems_random_uint32_chunk_producer STATIC
            tools/GGEMSRandomUInt32ChunkProducer.cc)

target_link_libraries(
  ggems_random_uint32_chunk_producer
  PUBLIC ggems_core
  PRIVATE GGEMS::CompilerOptions)

target_include_directories(
  ggems_random_uint32_chunk_producer
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tools
  PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(
  ggems_random_uint32_chunk_producer
  PRIVATE
    GGEMS_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels"
    GGEMS_VALIDATION_RANDOM_KERNEL_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/kernels")

ggems_apply_release_optimizations(ggems_random_uint32_chunk_producer)

# ------------------------------------------------------------------------------
# Random stream generator
add_executable(ggems_random_stream_generator
               tools/GGEMSRandomStreamGenerator.cc)

target_link_libraries(
  ggems_random_stream_generator PRIVATE ggems_random_uint32_chunk_producer
                                        ggems_core GGEMS::CompilerOptions)

target_include_directories(ggems_random_stream_generator
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(
  ggems_random_stream_generator
  PRIVATE
    GGEMS_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels"
    GGEMS_VALIDATION_RANDOM_KERNEL_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/kernels")

ggems_apply_release_optimizations(ggems_random_stream_generator)

# ------------------------------------------------------------------------------
# Binary stdout stream producer
add_executable(ggems_random_stream_pipe_producer
               tools/GGEMSRandomStreamPipeProducer.cc)

target_link_libraries(
  ggems_random_stream_pipe_producer PRIVATE ggems_random_uint32_chunk_producer
                                            ggems_core GGEMS::CompilerOptions)

target_include_directories(ggems_random_stream_pipe_producer
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)

ggems_apply_release_optimizations(ggems_random_stream_pipe_producer)

# ------------------------------------------------------------------------------
# Optional TestU01 validation target
add_subdirectory(testu01)

# ------------------------------------------------------------------------------
# Random validation dependencies
add_dependencies(validation_random ggems_random_stream_generator
                 ggems_random_stream_pipe_producer)

if(TARGET ggems_testu01_consumer)
  add_dependencies(validation_random ggems_testu01_consumer)
endif()
