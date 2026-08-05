#pragma once

#if defined(_WIN32)

#include <expected>
#include <optional>
#include <string>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMSVulkanDeviceSelection.hh"

struct GLFWwindow;

namespace ggems::ui::detail {

[[nodiscard]] std::expected<GGEMSVulkanDisplayAdapter, std::string>
ResolveWin32DisplayAdapter(GLFWwindow *window);

[[nodiscard]] std::optional<std::string>
QueryWin32VulkanAdapterId(vk::raii::PhysicalDevice const &physical_device);

} // namespace ggems::ui::detail

#endif
