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

if(TARGET GTest::gtest_main)
  set(GGEMS_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET gtest_main)
  set(GGEMS_GTEST_MAIN_TARGET gtest_main)
else()
  message(FATAL_ERROR "No usable GoogleTest main target was found.")
endif()

# ==============================================================================
# generic function adding test
function(ggems_add_test_suite target_name)
  set(one_value_args SOURCE_DIR)
  set(multi_value_args LINK_LIBRARIES COMPILE_DEFINITIONS INCLUDE_DIRECTORIES)

  cmake_parse_arguments(TEST "" "${one_value_args}" "${multi_value_args}"
                        ${ARGN})

  file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS
       "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE_DIR}/*.cc")

  add_executable(${target_name} ${test_sources})

  target_link_libraries(
    ${target_name} PRIVATE ${TEST_LINK_LIBRARIES} GGEMS::CompilerOptions
                           ${GGEMS_GTEST_MAIN_TARGET})

  target_include_directories(
    ${target_name}
    PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests/include
            ${TEST_INCLUDE_DIRECTORIES})

  if(TEST_COMPILE_DEFINITIONS)
    target_compile_definitions(${target_name}
                               PRIVATE ${TEST_COMPILE_DEFINITIONS})
  endif()

  ggems_apply_release_optimizations(${target_name})

  gtest_discover_tests(${target_name})
endfunction()

# ==============================================================================
# Core unit tests
ggems_add_test_suite(
  ggems_core_tests
  SOURCE_DIR
  core
  LINK_LIBRARIES
  ggems_core
  COMPILE_DEFINITIONS
  GGEMS_TEST_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels")

# ==============================================================================
# Framework unit tests
ggems_add_test_suite(
  ggems_framework_tests
  SOURCE_DIR
  frameworks
  LINK_LIBRARIES
  ggems_core
  COMPILE_DEFINITIONS
  GGEMS_TEST_KERNEL_ROOT="${PROJECT_SOURCE_DIR}/kernels")

# ==============================================================================
# GuiMode unit tests
if(GGEMS_WITH_IMGUI)
  ggems_add_test_suite(
    ggems_gui_tests
    SOURCE_DIR
    ui
    LINK_LIBRARIES
    ggems_gui
    COMPILE_DEFINITIONS
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)
endif()

# ==============================================================================
# Test build target
set(GGEMS_TEST_TARGETS ggems_core_tests ggems_framework_tests)

if(GGEMS_WITH_IMGUI)
  list(APPEND GGEMS_TEST_TARGETS ggems_gui_tests)
endif()

add_custom_target(tests DEPENDS ${GGEMS_TEST_TARGETS})
