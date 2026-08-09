# ============================================================================
# ============================================================================

include_guard(GLOBAL)

function(add_dear_imgui_target target_name)
  set(GGEMS_IMGUI_SOURCE_DIR "${PROJECT_SOURCE_DIR}/external/imgui")

  if(NOT EXISTS "${GGEMS_IMGUI_SOURCE_DIR}/imgui.cpp")
    message(
      FATAL_ERROR
        "Dear ImGui was not found in '${GGEMS_IMGUI_SOURCE_DIR}'.\n"
        "Initialize the external dependency with:\n"
        "  git submodule update --init --recursive")
  endif()

  add_library(
    ${target_name} STATIC
    ${GGEMS_IMGUI_SOURCE_DIR}/imgui.cpp
    ${GGEMS_IMGUI_SOURCE_DIR}/imgui_demo.cpp
    ${GGEMS_IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${GGEMS_IMGUI_SOURCE_DIR}/imgui_tables.cpp
    ${GGEMS_IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    ${GGEMS_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${GGEMS_IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

  add_library(GGEMS::ImGui ALIAS ${target_name})

  target_include_directories(
    ${target_name} PUBLIC ${GGEMS_IMGUI_SOURCE_DIR}
                          ${GGEMS_IMGUI_SOURCE_DIR}/backends)

  target_link_libraries(${target_name} PUBLIC Vulkan::Vulkan
                                              ${GGEMS_GLFW_TARGET})

  target_compile_features(${target_name} PUBLIC cxx_std_23)
endfunction()
