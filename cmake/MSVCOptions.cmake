# ============================================================================
# @file      MSVCOptions.cmake @brief     Compiler configuration for Microsoft
# Visual C++. @details   Configure warnings, language conformance, UTF-8
# support, and the dynamic MSVC runtime required by Python extension modules.
# ============================================================================

include_guard(GLOBAL)

# ----------------------------------------------------------------------------
# User options
# ----------------------------------------------------------------------------

option(GGEMS_WARNINGS_AS_ERRORS "Treat GGEMS compiler warnings as errors" OFF)

# ----------------------------------------------------------------------------
# MSVC runtime
#
# Debug   -> /MDd Release -> /MD
#
# GGEMS and its Python extension must use the dynamic MSVC runtime.
# ----------------------------------------------------------------------------

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# ----------------------------------------------------------------------------
# MSVC diagnostics and language conformance
# ----------------------------------------------------------------------------

message(STATUS "Configuring Microsoft Visual C++ compiler")

add_compile_options(
  "$<$<COMPILE_LANGUAGE:CXX>:/W4>"
  "$<$<COMPILE_LANGUAGE:CXX>:/EHsc>"
  "$<$<COMPILE_LANGUAGE:CXX>:/permissive->"
  "$<$<COMPILE_LANGUAGE:CXX>:/utf-8>"
  "$<$<COMPILE_LANGUAGE:CXX>:/nologo>"
  "$<$<COMPILE_LANGUAGE:CXX>:/Zc:preprocessor>"
  "$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>")

if(GGEMS_WARNINGS_AS_ERRORS)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/WX>")
endif()

# ----------------------------------------------------------------------------
# Diagnostic information
# ----------------------------------------------------------------------------

message(STATUS "MSVC compiler           : ${CMAKE_CXX_COMPILER}")
message(STATUS "MSVC compiler version   : ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "MSVC runtime            : dynamic (/MDd Debug, /MD Release)")
message(STATUS "Warnings as errors      : ${GGEMS_WARNINGS_AS_ERRORS}")
message(
  STATUS "Optimisation policy     : managed by CMake Debug/Release profiles")
