# GUI dependencies
find_package(Vulkan 1.3 REQUIRED GLOBAL)
find_package(glfw3 REQUIRED)

if(TARGET glfw)
  set(GGEMS_GLFW_TARGET glfw)
elseif(TARGET glfw::glfw)
  set(GGEMS_GLFW_TARGET glfw::glfw)
else()
  message(
    FATAL_ERROR
      "GLFW was found, but no supported imported CMake target is available.")
endif()

message(STATUS "Vulkan GUI dependency located.")
message(STATUS "GLFW GUI target          : ${GGEMS_GLFW_TARGET}")

# Dear ImGui
include(AddDearImGui)
add_dear_imgui_target(ggems_imgui)

# GGEMS GuiMode shaders
find_program(GGEMS_SLANGC_EXECUTABLE NAMES slangc slangc.exe)

if(NOT GGEMS_SLANGC_EXECUTABLE)
  message(FATAL_ERROR "The Slang shader compiler 'slangc' was not found. "
                      "It is required to build GGEMS GuiMode shaders.")
endif()

set(GGEMS_UI_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(GGEMS_UI_SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(MAKE_DIRECTORY "${GGEMS_UI_SHADER_OUTPUT_DIR}")

set(GGEMS_UI_SHADER_STAGES vertex fragment)
set(GGEMS_UI_SHADER_ENTRIES VertexMain FragmentMain)
set(GGEMS_UI_SHADER_SUFFIXES vert frag)
set(GGEMS_UI_SHADER_OUTPUTS)

foreach(shader_name IN ITEMS GGEMSAxes GGEMSTraces GGEMSGeometry)
  set(shader_source "${GGEMS_UI_SHADER_SOURCE_DIR}/${shader_name}.slang")
  foreach(shader_stage shader_entry shader_suffix IN ZIP_LISTS
          GGEMS_UI_SHADER_STAGES GGEMS_UI_SHADER_ENTRIES GGEMS_UI_SHADER_SUFFIXES)
    set(shader_output
        "${GGEMS_UI_SHADER_OUTPUT_DIR}/${shader_name}.${shader_suffix}.spv")
    add_custom_command(
      OUTPUT "${shader_output}"
      COMMAND
        "${GGEMS_SLANGC_EXECUTABLE}" "${shader_source}" -target spirv
        -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry
        "${shader_entry}" -stage "${shader_stage}" -o "${shader_output}"
      DEPENDS "${shader_source}"
      VERBATIM)
    list(APPEND GGEMS_UI_SHADER_OUTPUTS "${shader_output}")
  endforeach()
endforeach()

add_custom_target(ggems_gui_shaders DEPENDS ${GGEMS_UI_SHADER_OUTPUTS})

# GGEMS GuiMode library
file(GLOB GGEMS_UI_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
list(SORT GGEMS_UI_SOURCES)
add_library(ggems_gui STATIC ${GGEMS_UI_SOURCES})

add_dependencies(ggems_gui ggems_gui_shaders)
add_library(GGEMS::Gui ALIAS ggems_gui)

target_include_directories(ggems_gui PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(
  ggems_gui PRIVATE VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
                    GGEMS_UI_SHADER_DIRECTORY="${GGEMS_UI_SHADER_OUTPUT_DIR}")

target_link_libraries(
  ggems_gui PRIVATE ggems_core GGEMS::CompilerOptions ggems_imgui
                    Vulkan::Headers ${GGEMS_GLFW_TARGET})

if(WIN32)
  target_link_libraries(ggems_gui PRIVATE dxgi dxguid user32)
endif()

target_compile_features(ggems_gui PUBLIC cxx_std_23)

# Apply optional Release-only LTO and native CPU optimizations.
ggems_apply_release_optimizations(ggems_gui)
