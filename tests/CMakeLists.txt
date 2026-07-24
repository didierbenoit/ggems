# ============================================================================
# @file      CMakeLists.txt @brief     Unit test build configuration for GGEMS.
# @details   This file declares the GoogleTest-based test executables used to
# validate deterministic core components of GGEMS.
# ============================================================================

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------
include(FetchContent)
include(GoogleTest)

set(GGEMS_GTEST_VERSION
    "v1.17.0"
    CACHE STRING "GoogleTest version used when FetchContent is required.")

option(
  GGEMS_FETCH_GTEST
  "Download GoogleTest with FetchContent when no installed package is found."
  ON)

find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
  if(NOT GGEMS_FETCH_GTEST)
    message(
      FATAL_ERROR
        "GoogleTest was not found. Install GoogleTest, set GTest_DIR, "
        "or enable GGEMS_FETCH_GTEST.")
  endif()

  message(
    STATUS
      "GoogleTest was not found. Fetching GoogleTest ${GGEMS_GTEST_VERSION}.")

  set(INSTALL_GTEST
      OFF
      CACHE BOOL "" FORCE)
  set(BUILD_GMOCK
      OFF
      CACHE BOOL "" FORCE)

  if(WIN32)
    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE)
  endif()

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG ${GGEMS_GTEST_VERSION}
    GIT_SHALLOW TRUE)

  FetchContent_MakeAvailable(googletest)
endif()

if(TARGET GTest::gtest_main)
  set(GGEMS_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET gtest_main)
  set(GGEMS_GTEST_MAIN_TARGET gtest_main)
else()
  message(FATAL_ERROR "No usable GoogleTest main target was found.")
endif()

# ----------------------------------------------------------------------------
# Core unit tests
# ----------------------------------------------------------------------------
file(GLOB_RECURSE GGEMS_CORE_TEST_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cc")

add_executable(ggems_core_tests ${GGEMS_CORE_TEST_SOURCES})

target_compile_definitions(
  ggems_core_tests
  PRIVATE GGEMS_TEST_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels")

target_link_libraries(ggems_core_tests PRIVATE ggems_core
                                               ${GGEMS_GTEST_MAIN_TARGET})

target_include_directories(
  ggems_core_tests PRIVATE ${PROJECT_SOURCE_DIR}/include
                           ${PROJECT_SOURCE_DIR}/python)

ggems_apply_release_optimizations(ggems_core_tests)

gtest_discover_tests(ggems_core_tests)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(
  NAME GGEMSSourceBindingsPython
  COMMAND
    ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:ggems>"
    "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/GGEMSSourceBindingsTest.py")

# ----------------------------------------------------------------------------
# GuiMode unit tests
# ----------------------------------------------------------------------------
if(GGEMS_WITH_IMGUI)
  file(GLOB_RECURSE GGEMS_GUI_TEST_SOURCES CONFIGURE_DEPENDS
       "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cc")

  add_executable(ggems_gui_tests ${GGEMS_GUI_TEST_SOURCES})

  target_compile_definitions(
    ggems_gui_tests PRIVATE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
                            VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)

  target_link_libraries(ggems_gui_tests PRIVATE ggems_gui
                                                ${GGEMS_GTEST_MAIN_TARGET})

  target_include_directories(
    ggems_gui_tests PRIVATE ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/src/ui)

  ggems_apply_release_optimizations(ggems_gui_tests)

  gtest_discover_tests(ggems_gui_tests)
endif()
