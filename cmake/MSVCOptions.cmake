add_compile_options(
  "$<$<CONFIG:Debug>:/Od;/Zi;/RTC1;/W4;/Wall;/GS;/MDd;/sdl;/permissive->"
  "$<$<CONFIG:Debug>:/Zc:__cplusplus;/Zc:inline;/Zc:forScope;/Zc:wchar_t>"

  "$<$<CONFIG:Release>:/Ox;/GL;/fp:fast;/arch:AVX512;/arch:AVX2;/DNDEBUG;/MD>"

  /external:anglebrackets
  /external:W0
  /EHsc
)

add_link_options(
  "$<$<CONFIG:Release>:/LTCG;/OPT:REF;/OPT:ICF>"
)
