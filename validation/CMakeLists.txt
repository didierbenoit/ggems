# ------------------------------------------------------------------------------
# Scientific validation

# Global validation build target
add_custom_target(validation)

# ------------------------------------------------------------------------------
# Random number validation PractRand, Dieharder and TestU01 validation is
# supported on Linux only
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_subdirectory(random)
  add_dependencies(validation validation_random)
endif()

# ------------------------------------------------------------------------------
# Source validation
add_subdirectory(source)
add_dependencies(validation validation_source)
