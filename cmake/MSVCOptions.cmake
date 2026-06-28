# ============================================================================
#  @file      MSVCOptions.cmake
#  @brief     Compiler configuration for Microsoft Visual C++.
#  @details   Sets runtime, warnings, and optimisation options for GGEMS when
#             built under MSVC. Ensures full compatibility with Python and
#             pybind11 modules by enforcing a dynamic runtime (/MD or /MDd).
#             Designed for C++23 and OpenCL integration on Windows systems.
#  @author    Didier Benoit
#  @date      2025-11-05
# ============================================================================

include_guard(GLOBAL)

message(STATUS "Detected compiler: Microsoft Visual C++")

# ------------------------------------------------------------------------------
# MSVC: global compile options
# ------------------------------------------------------------------------------
add_compile_options(
  /std:c++latest    # Use latest C++ standard available
  /W4               # high warning level
  /EHsc             # C++ exceptions (synchronous)
  /permissive-      # Strict standard conformance
  /nologo           # Silence the MSVC banner
  /utf-8            # Source files treated as UTF-8
  /Zc:preprocessor
)

# ------------------------------------------------------------------------------
# MSVC: global compile definitions
# ------------------------------------------------------------------------------
add_compile_definitions(
  _CRT_SECURE_NO_WARNINGS  # Disable warnings for "unsafe" CRT functions
  _SCL_SECURE_NO_WARNINGS  # Disable warnings for "unsafe" STL functions
  NOMINMAX                 # Prevent Windows.h from defining min/max macros
)

# ----------------------------------------------------------------------------
# Debug configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring MSVC Debug build")
    add_compile_options(/Od)
    add_link_options(/DEBUG)
endif()

# ----------------------------------------------------------------------------
# Release configuration
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring MSVC Release build")
    add_compile_options(/O2 /GL)
    add_link_options(/LTCG)
    add_compile_definitions(NDEBUG)
endif()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------
message(STATUS "Build type    : ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler path : ${CMAKE_CXX_COMPILER}")
