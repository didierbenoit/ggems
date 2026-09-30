#pragma once

#if defined(_WIN32)

#include <expected>
#include <optional>
#include <string>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMSVulkanDeviceSelection.hh"

struct GLFWwindow;

namespace ggems::ui::detail {

[[nodiscard]] auto ResolveWin32DisplayAdapter(GLFWwindow *window)
  -> std::expected<GGEMSVulkanDisplayAdapter, std::string>;

[[nodiscard]] auto
QueryWin32VulkanAdapterId(vk::raii::PhysicalDevice const &physical_device)
  -> std::optional<std::string>;

} // namespace ggems::ui::detail

#endif
