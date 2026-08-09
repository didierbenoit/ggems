# ============================================================================
# ============================================================================

# ----------------------------------------------------------------------------
# F-18 spectrum exporter
# ----------------------------------------------------------------------------
add_executable(ggems_validate_f18_spectrum GGEMSF18SpectrumCSV.cc
                                           tools/GGEMSValidateF18Spectrum.cc)

target_link_libraries(ggems_validate_f18_spectrum PRIVATE ggems_core)

target_include_directories(
  ggems_validate_f18_spectrum PRIVATE ${PROJECT_SOURCE_DIR}/include
                                      ${CMAKE_CURRENT_SOURCE_DIR})

ggems_apply_release_optimizations(ggems_validate_f18_spectrum)

if(GGEMS_BUILD_TESTS)
  include(GoogleTest)

  add_executable(
    ggems_f18_csv_tests
    GGEMSF18SpectrumCSV.cc
    ${PROJECT_SOURCE_DIR}/tests/validation/radioactivity/GGEMSF18SpectrumCSVTest.cc
  )

  if(TARGET GTest::gtest_main)
    set(GGEMS_F18_CSV_GTEST_MAIN_TARGET GTest::gtest_main)
  elseif(TARGET gtest_main)
    set(GGEMS_F18_CSV_GTEST_MAIN_TARGET gtest_main)
  else()
    message(FATAL_ERROR "No usable GoogleTest main target was found.")
  endif()

  target_link_libraries(ggems_f18_csv_tests
                        PRIVATE ggems_core ${GGEMS_F18_CSV_GTEST_MAIN_TARGET})

  target_include_directories(
    ggems_f18_csv_tests PRIVATE ${PROJECT_SOURCE_DIR}/include
                                ${CMAKE_CURRENT_SOURCE_DIR})

  ggems_apply_release_optimizations(ggems_f18_csv_tests)
  gtest_discover_tests(ggems_f18_csv_tests)
endif()
