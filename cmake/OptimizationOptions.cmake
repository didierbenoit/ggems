# ============================================================================
# @file      OptimizationOptions.cmake @brief     Configure optional GGEMS
# Release optimizations. @details   Provide target-based support for link-time
# optimization and native CPU code generation.
# ============================================================================

include_guard(GLOBAL)

# ----------------------------------------------------------------------------
# User options
# ----------------------------------------------------------------------------

option(GGEMS_ENABLE_LTO "Enable link-time optimization in Release builds" OFF)

option(GGEMS_ENABLE_NATIVE
       "Optimize Release builds for the CPU used during compilation" OFF)

# ----------------------------------------------------------------------------
# Effective optimization policy
# ----------------------------------------------------------------------------

set(GGEMS_LTO_ENABLED "${GGEMS_ENABLE_LTO}")

if(GGEMS_ENABLE_LTO AND CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
  message(
    WARNING "GGEMS_ENABLE_LTO was requested with IntelLLVM/icpx, but IntelLLVM "
            "LTO is not currently supported by the GGEMS build configuration. "
            "LTO will be disabled. Release and native CPU optimizations remain "
            "available.")

  set(GGEMS_LTO_ENABLED OFF)
endif()

# ----------------------------------------------------------------------------
# Link-time optimization support
# ----------------------------------------------------------------------------

set(GGEMS_LTO_SUPPORTED OFF)

if(GGEMS_LTO_ENABLED)
  include(CheckIPOSupported)

  check_ipo_supported(
    RESULT GGEMS_LTO_SUPPORTED
    OUTPUT GGEMS_LTO_ERROR
    LANGUAGES CXX)

  if(NOT GGEMS_LTO_SUPPORTED)
    message(
      FATAL_ERROR
        "GGEMS LTO is enabled, but interprocedural optimization "
        "is unavailable for the current toolchain:\n" "${GGEMS_LTO_ERROR}")
  endif()
endif()

# ----------------------------------------------------------------------------
# Native CPU optimization support
# ----------------------------------------------------------------------------

set(GGEMS_NATIVE_FLAG "")

if(GGEMS_ENABLE_NATIVE)

  if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang|GNU|IntelLLVM)$")

    set(GGEMS_NATIVE_FLAG "-march=native")

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    message(
      FATAL_ERROR
        "GGEMS_ENABLE_NATIVE is not supported with Microsoft Visual C++. "
        "Configure with -DGGEMS_ENABLE_NATIVE=OFF.")

  else()

    message(FATAL_ERROR "GGEMS_ENABLE_NATIVE is unsupported for compiler "
                        "'${CMAKE_CXX_COMPILER_ID}'.")

  endif()

  include(CheckCXXCompilerFlag)

  check_cxx_compiler_flag("${GGEMS_NATIVE_FLAG}" GGEMS_HAS_NATIVE_FLAG)

  if(NOT GGEMS_HAS_NATIVE_FLAG)
    message(
      FATAL_ERROR
        "GGEMS_ENABLE_NATIVE is ON, but compiler "
        "'${CMAKE_CXX_COMPILER_ID}' does not accept " "'${GGEMS_NATIVE_FLAG}'.")
  endif()

endif()

# ----------------------------------------------------------------------------
# Apply the Release optimization policy to one GGEMS target
# ----------------------------------------------------------------------------

function(ggems_apply_release_optimizations target_name)

  if(NOT TARGET "${target_name}")
    message(FATAL_ERROR "Cannot apply GGEMS optimizations: "
                        "target '${target_name}' does not exist.")
  endif()

  # Debug must never use IPO/LTO.
  set_property(TARGET "${target_name}"
               PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)

  # Release uses IPO/LTO only when effectively enabled.
  set_property(
    TARGET "${target_name}" PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE
                                     "${GGEMS_LTO_ENABLED}")

  # Native CPU optimization is restricted to Release.
  if(GGEMS_ENABLE_NATIVE)
    target_compile_options("${target_name}"
                           PRIVATE "$<$<CONFIG:Release>:${GGEMS_NATIVE_FLAG}>")
  endif()

endfunction()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------

message(STATUS "Release LTO requested   : ${GGEMS_ENABLE_LTO}")
message(STATUS "Release LTO enabled     : ${GGEMS_LTO_ENABLED}")
message(STATUS "Native CPU requested    : ${GGEMS_ENABLE_NATIVE}")

if(GGEMS_LTO_ENABLED)
  message(STATUS "Release LTO support     : ${GGEMS_LTO_SUPPORTED}")
endif()

if(GGEMS_ENABLE_NATIVE)
  message(STATUS "Native CPU flag         : ${GGEMS_NATIVE_FLAG}")
  message(STATUS "Native CPU support      : ${GGEMS_HAS_NATIVE_FLAG}")
endif()
