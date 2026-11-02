# ==============================================================================
# Dependencies
include(FetchContent)
include(GoogleTest)

set(GGEMS_GTEST_VERSION
    "v1.18.0"
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

# ==============================================================================
# One executable per functional directory, one CTest entry per Google Test case
function(ggems_add_test_suite test_domain source_directory)
  set(test_target "ggems_${test_domain}_tests")
  file(GLOB test_sources CONFIGURE_DEPENDS
       "${CMAKE_CURRENT_SOURCE_DIR}/${source_directory}/*.cc")
  list(REMOVE_ITEM test_sources "${CMAKE_CURRENT_SOURCE_DIR}/GGEMSTestMain.cc")
  list(SORT test_sources)

  add_executable(${test_target} ${test_sources} GGEMSTestMain.cc)
  target_link_libraries(${test_target} PRIVATE ggems_core GGEMS::CompilerOptions
                                             GTest::gtest)
  target_include_directories(${test_target}
                             PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_compile_definitions(
    ${test_target}
    PRIVATE GGEMS_TEST_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels"
            GGEMS_TEST_PROBE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/kernels"
            GGEMS_TEST_TRACE_ROOT="${CMAKE_CURRENT_BINARY_DIR}/trace-kernels")

  ggems_apply_release_optimizations(${test_target})
  gtest_discover_tests(${test_target} PROPERTIES LABELS "${test_domain}")

  set(GGEMS_TEST_TARGETS ${GGEMS_TEST_TARGETS} ${test_target} PARENT_SCOPE)
endfunction()

# ==============================================================================
# Core tests at the tests/ root and the existing functional directories
set(GGEMS_TEST_TARGETS)
ggems_add_test_suite(core .)

set(GGEMS_CORE_TEST_DIRECTORIES
    geometry
    logging
    materials
    navigation
    observer
    opencl
    particles
    processes
    radioactivity
    random
    render
    sources
    transport
    units
    utf)

foreach(test_directory IN LISTS GGEMS_CORE_TEST_DIRECTORIES)
  ggems_add_test_suite(${test_directory} ${test_directory})
endforeach()

# Private C++ binding registration is tested without importing Python.
target_include_directories(ggems_geometry_tests
                           PRIVATE ${PROJECT_SOURCE_DIR}/python/ggems)

# ==============================================================================
# UI tests retain their optional GUI dependencies.
if(GGEMS_WITH_IMGUI)
  ggems_add_test_suite(ui ui)
  target_link_libraries(ggems_ui_tests PRIVATE ggems_gui Vulkan::Headers)
  target_compile_definitions(
    ggems_ui_tests PRIVATE
    "$<FILTER:$<TARGET_PROPERTY:ggems_gui,COMPILE_DEFINITIONS>,INCLUDE,VULKAN_HPP_>")
endif()

# ==============================================================================
# Test build target
add_custom_target(tests DEPENDS ${GGEMS_TEST_TARGETS})
