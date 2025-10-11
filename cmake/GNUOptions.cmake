add_compile_options(
  "$<$<CONFIG:Debug>:-Wall;-Wextra;-Wpedantic;-Wcast-align;-Wcast-qual>"
  "$<$<CONFIG:Debug>:-Wdisabled-optimization;-Wduplicated-branches>"
  "$<$<CONFIG:Debug>:-Wduplicated-cond;-Wformat=2;-Wlogical-op>"
  "$<$<CONFIG:Debug>:-Wmissing-include-dirs;-Wnull-dereference>"
  "$<$<CONFIG:Debug>:-Woverloaded-virtual;-Wpointer-arith;-Wshadow>"
  "$<$<CONFIG:Debug>:-Wvla;-Wswitch-enum>"

  "$<$<CONFIG:Release>:-O3;-DNDEBUG;-march=native;-mtune=native>"
  "$<$<CONFIG:Release>:-funroll-loops;-frename-registers>"
)
