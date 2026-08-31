# ------------------------------------------------------------------------------
# Minimum required CMake version
cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# ------------------------------------------------------------------------------
# MSVC-compatible debug information
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:Embedded>")

# ------------------------------------------------------------------------------
# Project declaration
project(
  GGEMS
  VERSION 2.0.0
  LANGUAGES CXX
  DESCRIPTION "GPU Geant4-based Monte Carlo Simulations")

if(APPLE)
  enable_language(OBJCXX)
endif()

# ------------------------------------------------------------------------------
# General CMake configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# C++ language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
  set(CMAKE_OBJCXX_STANDARD 23)
  set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
  set(CMAKE_OBJCXX_EXTENSIONS OFF)
endif()

# ------------------------------------------------------------------------------
# Supported build configurations
set(GGEMS_SUPPORTED_BUILD_TYPES Debug Release)

if(CMAKE_CONFIGURATION_TYPES)
  # Multi-configuration generators such as Visual Studio or Ninja Multi-Config
  set(CMAKE_CONFIGURATION_TYPES
      "${GGEMS_SUPPORTED_BUILD_TYPES}"
      CACHE STRING "GGEMS build configurations" FORCE)
else()
  # Single-configuration generators such as Ninja and Unix Makefiles
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        Debug
        CACHE STRING "GGEMS build configuration" FORCE)
  endif()

  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

  if(NOT CMAKE_BUILD_TYPE IN_LIST GGEMS_SUPPORTED_BUILD_TYPES)
    message(FATAL_ERROR "Unsupported GGEMS build type '${CMAKE_BUILD_TYPE}'."
                        "Expected Debug or Release.")
  endif()
endif()

# ------------------------------------------------------------------------------
# Launcher configuration
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerLauncher)

# ------------------------------------------------------------------------------
# Display build context
message(STATUS "CMake version in use: ${CMAKE_VERSION}")
message(STATUS "Configuring project ${PROJECT_NAME} version ${PROJECT_VERSION}")

# ------------------------------------------------------------------------------
# Compiler configuration dispatch
include(CompilerOptions)
include(OptimizationOptions)

# ------------------------------------------------------------------------------
# Optional build features
option(GGEMS_WITH_IMGUI "Enable ImGui / Vulkan interface" OFF)

# ------------------------------------------------------------------------------
# Dependencies
set(OPENCL_HPP_INCLUDE_DIR
    "${PROJECT_SOURCE_DIR}/external/OpenCL-CLHPP/include")

if(APPLE)
  set(OPENCL_HEADERS_INCLUDE_DIR
      "${PROJECT_SOURCE_DIR}/external/OpenCL-Headers")
  set(OPENCL_APPLE_COMPAT_INCLUDE_DIR
      "${PROJECT_SOURCE_DIR}/cmake/apple_opencl_compat")

  if(NOT EXISTS "${OPENCL_HEADERS_INCLUDE_DIR}/CL/opencl.h")
    message(
      FATAL_ERROR
        "The pinned external/OpenCL-Headers dependency is unavailable. "
        "Initialize it before configuring GGEMS on Apple.")
  endif()

  find_library(OpenCL_LIBRARY OpenCL REQUIRED)
  add_library(OpenCL::OpenCL INTERFACE IMPORTED)
  set_target_properties(
    OpenCL::OpenCL
    PROPERTIES INTERFACE_LINK_LIBRARIES "${OpenCL_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${OPENCL_HEADERS_INCLUDE_DIR}")
else()
  find_package(OpenCL REQUIRED)
endif()

message(STATUS "OpenCL library located at: ${OpenCL_LIBRARY}")
message(STATUS "OpenCL include path: ${OPENCL_HPP_INCLUDE_DIR}")

find_package(pybind11 REQUIRED CONFIG)

# ------------------------------------------------------------------------------
# Core library
file(GLOB_RECURSE GGEMS_CORE_SOURCES CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/src/*.cc")

if(APPLE)
  list(APPEND GGEMS_CORE_SOURCES
       "${PROJECT_SOURCE_DIR}/src/opencl/GGEMSOpenCLAppleSVM.mm")
endif()

# GuiMode is built separately as ggems_gui when requested.
list(FILTER GGEMS_CORE_SOURCES EXCLUDE REGEX "/ui/")
list(SORT GGEMS_CORE_SOURCES)

add_library(ggems_core STATIC ${GGEMS_CORE_SOURCES})
add_library(GGEMS::Core ALIAS ggems_core)

target_compile_features(ggems_core PUBLIC cxx_std_23)

target_include_directories(ggems_core PUBLIC ${PROJECT_SOURCE_DIR}/include
                                             ${OPENCL_HPP_INCLUDE_DIR})

if(APPLE)
  target_include_directories(
    ggems_core BEFORE PUBLIC ${OPENCL_APPLE_COMPAT_INCLUDE_DIR}
                             ${OPENCL_HEADERS_INCLUDE_DIR})
endif()

target_compile_definitions(
  ggems_core
  PUBLIC "$<$<CONFIG:Debug>:GGEMS_DEBUG_MODE>"
         "$<$<CONFIG:Release>:GGEMS_RELEASE_MODE>"
  PRIVATE "GGEMS_KERNEL_ROOT=\"${PROJECT_SOURCE_DIR}/kernels\"")

target_link_libraries(
  ggems_core
  PUBLIC OpenCL::OpenCL
  PRIVATE GGEMS::CompilerOptions)

if(UNIX AND NOT APPLE)
  target_link_libraries(ggems_core PRIVATE atomic)
endif()

# ------------------------------------------------------------------------------
# Release optimization policy
ggems_apply_release_optimizations(ggems_core)

# ------------------------------------------------------------------------------
# Optional ImGui / Vulkan interface
if(GGEMS_WITH_IMGUI)
  message(STATUS "GGEMS GuiMode enabled: Vulkan / Dear ImGui layer requested.")
  add_subdirectory(src/ui)
endif()

# Python module (pybind11)
add_subdirectory(python)

# Unit testing
enable_testing()
add_subdirectory(tests EXCLUDE_FROM_ALL)

# Scientific validation
add_subdirectory(validation EXCLUDE_FROM_ALL)

# Documentation
add_subdirectory(docs EXCLUDE_FROM_ALL)

# Build summary
message(STATUS "")
message(STATUS "================= GGEMS Build Summary =================")
message(STATUS "GGEMS version          : ${PROJECT_VERSION}")
message(STATUS "CMake version          : ${CMAKE_VERSION}")
message(STATUS "Generator              : ${CMAKE_GENERATOR}")
message(STATUS "Compiler ID            : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler version       : ${CMAKE_CXX_COMPILER_VERSION}")
message(
  STATUS "Compiler frontend      : ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
message(STATUS "Compiler path          : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard           : C++${CMAKE_CXX_STANDARD}")

if(CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Build configurations   : ${CMAKE_CONFIGURATION_TYPES}")
else()
  message(STATUS "Build type             : ${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "Compiler cache         : ${GGEMS_USE_COMPILER_CACHE}")
message(STATUS "Test targets           : tests")
message(STATUS "ImGui UI enabled       : ${GGEMS_WITH_IMGUI}")
message(STATUS "Validation target      : validation")
message(STATUS "Documentation targets  : doxygen, sphinx, docs")
message(STATUS "========================================================")
message(STATUS "")
