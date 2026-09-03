# ------------------------------------------------------------------------------
# Optional TestU01 validation dependency
#
# TestU01 is not a GGEMS runtime dependency. If the development files are not
# available, the TestU01 consumer is simply not built.

find_path(TESTU01_INCLUDE_DIR NAMES unif01.h)
find_library(TESTU01_LIBRARY NAMES testu01)
find_library(TESTU01_PROBDIST_LIBRARY NAMES probdist)
find_library(TESTU01_MYLIB_LIBRARY NAMES mylib)
find_library(TESTU01_MATH_LIBRARY NAMES m)

if(NOT TESTU01_INCLUDE_DIR
   OR NOT TESTU01_LIBRARY
   OR NOT TESTU01_PROBDIST_LIBRARY
   OR NOT TESTU01_MYLIB_LIBRARY
   OR NOT TESTU01_MATH_LIBRARY)
  message(STATUS "TestU01 validation       : unavailable")
  return()
endif()

include(CheckLanguage)
check_language(C)
if(NOT CMAKE_C_COMPILER)
  message(STATUS "TestU01 validation       : unavailable (C compiler not found)")
  return()
endif()
enable_language(C)

add_library(TestU01::TestU01 UNKNOWN IMPORTED)
set_target_properties(
  TestU01::TestU01
  PROPERTIES
    IMPORTED_LOCATION "${TESTU01_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${TESTU01_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES
    "${TESTU01_PROBDIST_LIBRARY};${TESTU01_MYLIB_LIBRARY};${TESTU01_MATH_LIBRARY}"
)

message(STATUS "TestU01 validation       : available")

# ------------------------------------------------------------------------------
# TestU01 consumer
add_executable(ggems_testu01_consumer GGEMSTestU01Consumer.c)

set_target_properties(
  ggems_testu01_consumer
  PROPERTIES C_STANDARD 17
             C_STANDARD_REQUIRED ON
             C_EXTENSIONS OFF)

target_compile_definitions(ggems_testu01_consumer
                           PRIVATE _POSIX_C_SOURCE=200809L)

target_compile_options(
  ggems_testu01_consumer
  PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,IntelLLVM>:
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wfloat-conversion>
    $<$<AND:$<BOOL:${GGEMS_WARNINGS_AS_ERRORS}>,$<C_COMPILER_ID:GNU,Clang,IntelLLVM>>:
    -Werror>)

target_link_libraries(ggems_testu01_consumer PRIVATE TestU01::TestU01)
