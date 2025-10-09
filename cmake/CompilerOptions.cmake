# Get last C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Initialize compiler flags
set(CMAKE_CXX_FLAGS "" CACHE STRING "CXX flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "Debug flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "Release flags" FORCE)

# Compiler detection and set flag for each compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  include(cmake/MSVCOptions.cmake)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  include(cmake/ClangOptions.cmake)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  include(cmake/GNUOptions.cmake)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
  include(cmake/IntelOptions.cmake)
endif()
