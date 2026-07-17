# ============================================================================
# @file      CMakeLists.txt
# @brief     Build configuration for the optional GGEMS graphical mode.
# @details   Resolves Vulkan, GLFW and GLM only when GGEMS_WITH_IMGUI is 
#            enabled, preserving headless builds.
# ============================================================================

# ----------------------------------------------------------------------------
# GUI dependencies
# ----------------------------------------------------------------------------
find_package(Vulkan 1.3 REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Python REQUIRED COMPONENTS Interpreter)

if(TARGET glfw)
  set(GGEMS_GLFW_TARGET glfw)
elseif(TARGET glfw::glfw)
  set(GGEMS_GLFW_TARGET glfw::glfw)
else()
  message(FATAL_ERROR
    "GLFW was found, but no supported imported CMake target is available."
  )
endif()

if(TARGET glm::glm)
  set(GGEMS_GLM_TARGET glm::glm)
elseif(TARGET glm)
  set(GGEMS_GLM_TARGET glm)
else()
  message(FATAL_ERROR
    "GLM was found, but no supported imported CMAKE target is available."
  )
endif()

message(STATUS "Vulkan GUI dependency located.")
message(STATUS "GLFW GUI target          : ${GGEMS_GLFW_TARGET}")
message(STATUS "GLM target               : ${GGEMS_GLM_TARGET}")

# ----------------------------------------------------------------------------
# Dear ImGui
# ----------------------------------------------------------------------------
include(AddDearImGui)
add_dear_imgui_target(ggems_imgui)

# ----------------------------------------------------------------------------
# GGEMS GuiMode shaders
# ----------------------------------------------------------------------------
find_program(GGEMS_SLANGC_EXECUTABLE
  NAMES slangc slangc.exe
)

if(NOT GGEMS_SLANGC_EXECUTABLE)
  message(FATAL_ERROR
    "The Slang shader compiler 'slangc' was not found. "
    "It is required to build GGEMS GuiMode shaders."
  )
endif()

set(GGEMS_UI_SHADER_SOURCE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

set(GGEMS_UI_SHADER_OUTPUT_DIR
  "${CMAKE_CURRENT_BINARY_DIR}/shaders"
)

file(MAKE_DIRECTORY "${GGEMS_UI_SHADER_OUTPUT_DIR}")


set(GGEMS_AXES_SHADER_SOURCE
  "${GGEMS_UI_SHADER_SOURCE_DIR}/GGEMSAxes.slang"
)
set(GGEMS_AXES_VERTEX_SPV
  "${GGEMS_UI_SHADER_OUTPUT_DIR}/GGEMSAxes.vert.spv"
)
set(GGEMS_AXES_FRAGMENT_SPV
  "${GGEMS_UI_SHADER_OUTPUT_DIR}/GGEMSAxes.frag.spv"
)

set(GGEMS_TRACES_SHADER_SOURCE
  "${GGEMS_UI_SHADER_SOURCE_DIR}/GGEMSTraces.slang"
)
set(GGEMS_TRACES_VERTEX_SPV
  "${GGEMS_UI_SHADER_OUTPUT_DIR}/GGEMSTraces.vert.spv"
)
set(GGEMS_TRACES_FRAGMENT_SPV
  "${GGEMS_UI_SHADER_OUTPUT_DIR}/GGEMSTraces.frag.spv"
)

add_custom_command(
  OUTPUT "${GGEMS_AXES_VERTEX_SPV}"
  COMMAND "${GGEMS_SLANGC_EXECUTABLE}"
          "${GGEMS_AXES_SHADER_SOURCE}"
          -target spirv
          -profile spirv_1_4
          -emit-spirv-directly
          -fvk-use-entrypoint-name
          -entry VertexMain
          -stage vertex
          -o "${GGEMS_AXES_VERTEX_SPV}"
  DEPENDS "${GGEMS_AXES_SHADER_SOURCE}"
  VERBATIM
)

add_custom_command(
  OUTPUT "${GGEMS_AXES_FRAGMENT_SPV}"
  COMMAND "${GGEMS_SLANGC_EXECUTABLE}"
          "${GGEMS_AXES_SHADER_SOURCE}"
          -target spirv
          -profile spirv_1_4
          -emit-spirv-directly
          -fvk-use-entrypoint-name
          -entry FragmentMain
          -stage fragment
          -o "${GGEMS_AXES_FRAGMENT_SPV}"
  DEPENDS "${GGEMS_AXES_SHADER_SOURCE}"
  VERBATIM
)

add_custom_command(
  OUTPUT "${GGEMS_TRACES_VERTEX_SPV}"
  COMMAND "${GGEMS_SLANGC_EXECUTABLE}"
          "${GGEMS_TRACES_SHADER_SOURCE}"
          -target spirv
          -profile spirv_1_4
          -emit-spirv-directly
          -fvk-use-entrypoint-name
          -entry VertexMain
          -stage vertex
          -o "${GGEMS_TRACES_VERTEX_SPV}"
  DEPENDS "${GGEMS_TRACES_SHADER_SOURCE}"
  VERBATIM
)

add_custom_command(
  OUTPUT "${GGEMS_TRACES_FRAGMENT_SPV}"
  COMMAND "${GGEMS_SLANGC_EXECUTABLE}"
          "${GGEMS_TRACES_SHADER_SOURCE}"
          -target spirv
          -profile spirv_1_4
          -emit-spirv-directly
          -fvk-use-entrypoint-name
          -entry FragmentMain
          -stage fragment
          -o "${GGEMS_TRACES_FRAGMENT_SPV}"
  DEPENDS "${GGEMS_TRACES_SHADER_SOURCE}"
  VERBATIM
)

add_custom_target(ggems_gui_shaders
  DEPENDS
    "${GGEMS_AXES_VERTEX_SPV}"
    "${GGEMS_AXES_FRAGMENT_SPV}"
    "${GGEMS_TRACES_VERTEX_SPV}"
    "${GGEMS_TRACES_FRAGMENT_SPV}"
)

# ----------------------------------------------------------------------------
# GGEMS GuiMode library
# ----------------------------------------------------------------------------
file(GLOB GGEMS_GUI_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

list(SORT GGEMS_GUI_SOURCES)

add_library(ggems_gui STATIC
  ${GGEMS_GUI_SOURCES}
)

add_dependencies(ggems_gui ggems_gui_shaders)

add_library(GGEMS::Gui ALIAS ggems_gui)

target_include_directories(ggems_gui
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(ggems_gui 
  PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    GGEMS_UI_SHADER_DIRECTORY="${GGEMS_UI_SHADER_OUTPUT_DIR}"
)

target_link_libraries(ggems_gui
  PUBLIC
    Vulkan::Headers

  PRIVATE
    ggems_core
    ggems_imgui
    Vulkan::Vulkan
    ${GGEMS_GLFW_TARGET}
)

if(WIN32)
  target_link_libraries(ggems_gui
    PRIVATE
      dxgi
      dxguid
      user32
  )
endif()

target_compile_features(ggems_gui PUBLIC cxx_std_23)
