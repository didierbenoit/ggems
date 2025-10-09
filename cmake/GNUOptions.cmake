add_compile_options(
  "$<$<CONFIG:Debug>:-O0;-g;-fno-omit-frame-pointer;-fstack-protector-strong>"
  "$<$<CONFIG:Debug>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
  "$<$<CONFIG:Debug>:-Wsign-conversion;-Wnon-virtual-dtor;-fno-common>"
  "$<$<CONFIG:Debug>:-Wfloat-equal;-Wcast-align;-Wpointer-arith;-Wmissing-include-dirs>"
  "$<$<CONFIG:Debug>:-Wnon-virtual-dtor;-Wunused;-Wuninitialized;-Wswitch-enum>"
  "$<$<CONFIG:Debug>:-Wduplicated-cond;-Wduplicated-branches;-Wlogical-op>"
  "$<$<CONFIG:Debug>:-Wnull-dereference;-Wdouble-promotion>"

  "$<$<CONFIG:Release>:-O3;-DNDEBUG;-march=native;-mtune=native;-ffast-math>"
  "$<$<CONFIG:Release>:-funroll-loops;-fstrict-aliasing;-frename-registers>"
  "$<$<CONFIG:Release>:-finline-functions;-flto=auto;-fdata-sections;-ffunction-sections>"
)

add_link_options(
  "$<$<CONFIG:Release>:-flto=auto;-Wl,--gc-sections>"
)
