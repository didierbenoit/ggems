# ============================================================================
#  @file      IntelOptions.cmake
#  @brief     Compiler configuration for Intel Compiler.
#  @details   Defines warnings, optimisation, and build-type specific flags
#             for Unix
#             Designed for C++23, pybind11 and OpenCL builds in GGEMS.
#  @author    Didier Benoit
#  @date      2025-11-05
# ============================================================================

include_guard(GLOBAL)

message(STATUS "Detected Intel oneAPI compiler (icx/icpx)")

# ----------------------------------------------------------------------------
# Common compilation options
# ----------------------------------------------------------------------------
add_compile_options(
  -std=c++23 -Wall -Wextra -Wpedantic
  -Wshadow -Wnon-virtual-dtor -finput-charset=UTF-8 -fexec-charset=UTF-8
  -Wconversion -Wsign-conversion
)

# Colour diagnostics and visibility control
add_compile_options(
  -fdiagnostics-color=always
  -fcolor-diagnostics
  -fvisibility=hidden
  -fvisibility-inlines-hidden
)
add_compile_definitions(NOMINMAX)

# ----------------------------------------------------------------------------
# Debug configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Configuring Clang Debug build")

  add_compile_options(-O0 -g3 -Wfloat-conversion -fno-omit-frame-pointer)
endif()

# ----------------------------------------------------------------------------
# Release configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Configuring Clang Release build")

  add_compile_options(
    -O3 -march=native
    -qopt-zmm-usage=high
    -finline-functions
    -fstrict-aliasing
  )

  add_compile_definitions(NDEBUG)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  # Enable LINK-TIME optimisation if supported
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
  if (ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    message(STATUS "Intel IPO/LTO enabled")
  else()
    message(WARNING "Intel IPO/LTO not supported: ${ipo_error}")
  endif()
endif()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------
message(STATUS "Intel build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler path    : ${CMAKE_CXX_COMPILER}")
