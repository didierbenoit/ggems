include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723
  OVERRIDE_FIND_PACKAGE)

if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB TEST_SOURCE_FILES *.cc)

add_executable(tests ${TEST_SOURCE_FILES})
target_link_libraries(tests PRIVATE ggems GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
