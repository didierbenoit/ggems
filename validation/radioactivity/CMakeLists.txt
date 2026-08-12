# F-18 spectrum exporter
add_executable(ggems_validate_f18_spectrum GGEMSF18SpectrumCSV.cc
                                           tools/GGEMSValidateF18Spectrum.cc)

target_link_libraries(ggems_validate_f18_spectrum
                      PRIVATE ggems_core GGEMS::CompilerOptions)

target_include_directories(
  ggems_validate_f18_spectrum PRIVATE ${PROJECT_SOURCE_DIR}/include
                                      ${CMAKE_CURRENT_SOURCE_DIR})

ggems_apply_release_optimizations(ggems_validate_f18_spectrum)
