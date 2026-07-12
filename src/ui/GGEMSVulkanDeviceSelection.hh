#pragma once

#include <cstdlib>
#include <expected>
#include <optional>
#include <span>
#include <string>

#include <vulkan/vulkan.hpp>

namespace ggems::ui::detail {

enum class GGEMSVulkanDeviceSelectorKind : std::uint8_t {
  Automatic,
  EnumerationIndex,
  Name
};

struct GGEMSVulkanDeviceSelector {
  GGEMSVulkanDeviceSelectorKind kind{GGEMSVulkanDeviceSelectorKind::Automatic};
  std::uint32_t enumeration_index{0U};
  std::string name{};

  [[nodiscard]] static GGEMSVulkanDeviceSelector
  FromString(std::string selection);

  [[nodiscard]] static GGEMSVulkanDeviceSelector
  FromIndex(std::uint32_t enumeration_index);
};

struct GGEMSVulkanDisplayAdapter {
  std::string platform_id{};
  std::string name{};
};

struct GGEMSVulkanDeviceCandidate {
  std::uint32_t enumeration_index{0U};
  vk::PhysicalDevice physical_device{};
  std::string name{};
  vk::PhysicalDeviceType type{vk::PhysicalDeviceType::eOther};
  std::uint32_t vendor_id{0U};
  std::uint32_t device_id{0U};
  std::uint32_t api_version{0U};
  std::uint32_t driver_version{0U};
  std::optional<std::uint32_t> graphics_queue_family{};
  std::optional<std::uint32_t> presentation_queue_family{};
  bool required_extensions_available{false};
  bool required_features_available{false};
  bool swapchain_adequate{false};
  bool suitable{false};
  std::string rejection_reason{};
  std::optional<std::string> platform_adapter_id{};
};

struct GGEMSVulkanDeviceSelection {
  std::uint32_t enumeration_index{0U};
  std::string reason{};
  bool display_adapter_mismatch{false};
};

[[nodiscard]] std::uint32_t
GetVulkanDeviceFallbackScore(vk::PhysicalDeviceType type) noexcept;

[[nodiscard]] bool IsVulkanDisplayAdapterMismatch(
    GGEMSVulkanDeviceCandidate const &candidate,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) noexcept;

[[nodiscard]] std::expected<GGEMSVulkanDeviceSelection, std::string>
SelectVulkanDevice(
    GGEMSVulkanDeviceSelector const &selector,
    std::span<GGEMSVulkanDeviceCandidate const> candidates,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter);

} // namespace ggems::ui::detail
