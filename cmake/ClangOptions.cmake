add_compile_options(
  "$<$<CONFIG:Debug>:-Wall;-Wextra;-Wpedantic;-Wshadow>"
  "$<$<CONFIG:Debug>:-Wdouble-promotion;-Wformat=2;-Wformat-truncation>"
  "$<$<CONFIG:Debug>:-Wundef;-fno-common;-Wconversion>"
  "$<$<CONFIG:Debug>:-g3;-O0;-ffunction-sections;-fdata-sections;-Wpadded>"

  "$<$<CONFIG:Release>:-O3;-DNDEBUG;-march=native;-mtune=native>"
  "$<$<CONFIG:Release>:-fvectorize;-funroll-loops>"
)
