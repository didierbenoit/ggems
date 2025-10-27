add_compile_options(
  "$<$<CONFIG:Debug>:/Od;/Zi;/W4;/GS;/fp:precise>"
  "$<$<CONFIG:Release>:/Ox;/favor:blend;/GL;/fp:fast;/arch:AVX512;/arch:AVX2>"
  "$<$<CONFIG:Release>:/DNDEBUG>"
  /external:anglebrackets
  /external:W0
  /MT
  /EHsc
)

add_link_options(
  "$<$<CONFIG:Release>:/LTCG;/OPT:REF;/OPT:ICF>"
)
