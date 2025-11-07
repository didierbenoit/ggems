# ============================================================================
#  @file      CompilerOptions.cmake
#  @brief     Dispatches compiler-specific configurations for GGEMS.
#  @details   Detects the active C++ compiler and includes the corresponding
#             configuration file (Clang or MSVC). Provides a single entry
#             point for all compiler-related options.
#  @author    Didier Benoit
#  @date      2025-11-05
# ============================================================================

include_guard(GLOBAL)

# ----------------------------------------------------------------------------
# Detect and include compiler-specific configuration
# ----------------------------------------------------------------------------
message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER_ID}")

# --- Clang or clang-cl -------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(${CMAKE_SOURCE_DIR}/cmake/ClangOptions.cmake)

# --- Microsoft Visual C++ ----------------------------------------------------
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  include(${CMAKE_SOURCE_DIR}/cmake/MSVCOptions.cmake)

# --- Fallback ----------------------------------------------------------------
else()
  message(WARNING "Unsupported compiler detected: ${CMAKE_CXX_COMPILER_ID}")
  message(WARNING "Default options will be used; please provide a specific configuration file.")
endif()

if (MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
