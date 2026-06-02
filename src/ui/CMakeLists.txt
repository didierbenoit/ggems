# ============================================================================
# @file      CMakeLists.txt
# @brief     Build configuration for the optional GGEMS graphical mode.
# @details   Resolves Vulkan, GLFW and GLM only when GGEMS_WITH_IMGUI is 
#            enabled, preserving headless Terminal and Cluster builds.
# ============================================================================

# ----------------------------------------------------------------------------
# GUI dependencies
# ----------------------------------------------------------------------------
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

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
