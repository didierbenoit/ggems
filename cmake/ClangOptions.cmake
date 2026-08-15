include_guard(GLOBAL)

# GNU-like frontend Used by: - clang++ on Windows - clang++ on Linux -
# AppleClang on macOS
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  message(STATUS "Configuring Clang GNU-like frontend")
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
      "$<$<COMPILE_LANGUAGE:CXX>:-fcolor-diagnostics>"
      "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:-fno-omit-frame-pointer>"
  )

  # ELF and Mach-O visibility. Windows symbol visibility is managed through its
  # DLL import/export model.
  if(NOT WIN32)
    target_compile_options(
      ggems_compiler_options
      INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>"
                "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>")
  endif()

  if(GGEMS_WARNINGS_AS_ERRORS)
    target_compile_options(ggems_compiler_options
                           INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:-Werror>")
  endif()

  # MSVC-like frontend Used by: - clang-cl on Windows
elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  message(STATUS "Configuring MSVC-like compiler frontend")
  target_compile_options(
    ggems_compiler_options
    INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:/W4>"
              "$<$<COMPILE_LANGUAGE:CXX>:/EHsc>"
              "$<$<COMPILE_LANGUAGE:CXX>:/permissive->"
              "$<$<COMPILE_LANGUAGE:CXX>:/utf-8>"
              "$<$<COMPILE_LANGUAGE:CXX>:/nologo>"
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

  # Unsupported frontend
else()
  message(FATAL_ERROR "Unsupported Clang frontend variant: "
                      "'${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}'.")
endif()

# Diagnostic information
message(
  STATUS "Clang frontend          : ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
message(STATUS "Warnings as errors      : ${GGEMS_WARNINGS_AS_ERRORS}")
message(
  STATUS "Optimization policy     : managed by CMake Debug/Release profiles")
