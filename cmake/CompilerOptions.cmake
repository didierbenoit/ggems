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

# ------------------------------------------------------------------------------
# Detect and include compiler-specific configuration
# ------------------------------------------------------------------------------
message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER_ID}")

# --- Windows Rules ------------------------------------------------------------
if (WIN32 AND NOT (MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  message(FATAL_ERROR
    "Unsupported compiler on Windows: ${CMAKE_CXX_COMPILER_ID}\n"
    "Allowed compilers on Windows:\n"
    "  - MSVC\n"
    "  - clang-cl (Clang with MSVC frontend)\n"
    "  - Clang/LLVM (pure clang++)\n"
    "GNU g++ and Intel icpx are NOT supported on Windows.\n"
  )
endif()

# --- Linux Rules --------------------------------------------------------------
if (UNIX AND NOT WIN32)
  if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID MATCHES "Intel"))
    message(FATAL_ERROR
      "Unsupported compiler on Linux: ${CMAKE_CXX_COMPILER_ID}\n"
      "Allowed compilers on Linux:\n"
      " - Clang/LLVM\n"
      " - GNU g++\n"
      " - Intel icpx\n"
    )
  endif()
endif()

# --- Clang or clang-cl --------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(STATUS "Loading Clang Option...")
  include(${CMAKE_CURRENT_LIST_DIR}/ClangOptions.cmake)

# --- Microsoft Visual C++ -----------------------------------------------------
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  message(STATUS "Loading MSVC options...")
  include(${CMAKE_CURRENT_LIST_DIR}/MSVCOptions.cmake)

  # --- GNU GCC / g++ ----------------------------------------------------------
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "Loading GCC options...")
  include(${CMAKE_CURRENT_LIST_DIR}/GNUOptions.cmake)

# --- Intel oneAPI (icpx / classic icc) ----------------------------------------
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  message(STATUS "Loading Intel compiler options...")
  include(${CMAKE_CURRENT_LIST_DIR}/IntelOptions.cmake)

# --- Fallback -----------------------------------------------------------------
else()
  message(WARNING "Unsupported compiler detected: ${CMAKE_CXX_COMPILER_ID}")
  message(WARNING "Default options will be used; please provide a specific configuration file.")
endif()

if (WIN32 AND (MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
