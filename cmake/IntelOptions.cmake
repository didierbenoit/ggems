include_guard(GLOBAL)

# IntelLLVM diagnostics
message(STATUS "Configuring Intel oneAPI C++ compiler")

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  message(STATUS "Configuring IntelLLVM MSVC-like frontend")

  target_compile_options(
    ggems_compiler_options
    INTERFACE
      "$<$<COMPILE_LANGUAGE:CXX>:/W4>"
      "$<$<COMPILE_LANGUAGE:CXX>:/EHsc>"
      "$<$<COMPILE_LANGUAGE:CXX>:/permissive->"
      "$<$<COMPILE_LANGUAGE:CXX>:/utf-8>"
      "$<$<COMPILE_LANGUAGE:CXX>:/nologo>"
      "$<$<COMPILE_LANGUAGE:CXX>:/fp:precise>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wshadow>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wconversion>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wsign-conversion>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wfloat-conversion>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wno-language-extension-token>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wno-microsoft-enum-value>"
      "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:/Oy->")

  if(GGEMS_WARNINGS_AS_ERRORS)
    target_compile_options(ggems_compiler_options
                           INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:/WX>")
  endif()

elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  message(STATUS "Configuring IntelLLVM GNU-like frontend")

  target_compile_options(
    ggems_compiler_options
    INTERFACE
      "$<$<COMPILE_LANGUAGE:CXX>:-Wall>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wextra>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wshadow>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wconversion>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wsign-conversion>"
      "$<$<COMPILE_LANGUAGE:CXX>:-Wfloat-conversion>"
      "$<$<COMPILE_LANGUAGE:CXX>:-finput-charset=UTF-8>"
      "$<$<COMPILE_LANGUAGE:CXX>:-fexec-charset=UTF-8>"
      "$<$<COMPILE_LANGUAGE:CXX>:-fdiagnostics-color=always>"
      "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>"
      "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>"
      "$<$<COMPILE_LANGUAGE:CXX>:-fp-model=precise>"
      "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:-fno-omit-frame-pointer>")

  if(GGEMS_WARNINGS_AS_ERRORS)
    target_compile_options(ggems_compiler_options
                           INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:-Werror>")
  endif()

else()
  message(
    FATAL_ERROR
      "Unsupported IntelLLVM frontend: "
      "'${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}'.")
endif()

# Diagnostic information
message(STATUS "Intel compiler          : ${CMAKE_CXX_COMPILER}")
message(STATUS "Intel compiler version  : ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Intel compiler frontend : ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
message(STATUS "Warnings as errors      : ${GGEMS_WARNINGS_AS_ERRORS}")
message(
  STATUS "Optimization policy     : managed by CMake Debug/Release profiles")
