add_compile_options(
  "$<$<CONFIG:Debug>:-O0;-g;-fno-omit-frame-pointer;-fstack-protector-strong>"
  "$<$<CONFIG:Debug>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Wsign-conversion>"
  "$<$<CONFIG:Debug>:-Wnon-virtual-dtor;-fno-common>"
  "$<$<CONFIG:Debug>:-fsanitize=address,undefined;-Wfloat-equal;-Wcast-align;-Wpointer-arith>"
  "$<$<CONFIG:Debug>:-Wmissing-include-dirs;-Wnon-virtual-dtor>"

  "$<$<CONFIG:Release>:-O3;-DNDEBUG;-march=native;-mtune=native;-ffast-math>"
  "$<$<CONFIG:Release>:-fvectorize;-fslp-vectorize;-funroll-loops;-fstrict-aliasing>"
  "$<$<CONFIG:Release>:-finline-functions;-flto=full;-fuse-ld=lld;-fdata-sections;-ffunction-sections>"
)

add_link_options(
  "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
)
