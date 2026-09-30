#pragma once

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <cstdint>

#include <vulkan/vulkan.hpp>

namespace ggems::ui::detail {

enum class GGEMSVulkanDeviceSelectorKind : std::uint8_t {
  Automatic = 0U,
  EnumerationIndex,
  Name,
};

struct GGEMSVulkanDeviceSelector {
  GGEMSVulkanDeviceSelectorKind kind{GGEMSVulkanDeviceSelectorKind::Automatic};
  std::uint32_t enumeration_index{0U};
  std::string name;

  [[nodiscard]] static auto FromString(std::string selection)
    -> GGEMSVulkanDeviceSelector;

  [[nodiscard]] static auto FromIndex(std::uint32_t enumeration_index)
    -> GGEMSVulkanDeviceSelector;
};

struct GGEMSVulkanDisplayAdapter {
  std::string platform_id;
  std::string name;
};

struct GGEMSVulkanDeviceCandidate {
  std::uint32_t enumeration_index{0U};
  vk::PhysicalDevice physical_device{};
  std::string name;
  vk::PhysicalDeviceType type{vk::PhysicalDeviceType::eOther};
  std::uint32_t vendor_id{0U};
  std::uint32_t device_id{0U};
  std::uint32_t api_version{0U};
  std::uint32_t driver_version{0U};
  std::optional<std::uint32_t> graphics_queue_family;
  std::optional<std::uint32_t> presentation_queue_family;
  bool required_extensions_available{false};
  bool required_features_available{false};
  bool swapchain_adequate{false};
  bool suitable{false};
  std::string rejection_reason;
  std::optional<std::string> platform_adapter_id;
};

struct GGEMSVulkanDeviceSelection {
  std::uint32_t enumeration_index{0U};
  std::string reason;
  bool display_adapter_mismatch{false};
};

[[nodiscard]] auto
GetVulkanDeviceFallbackScore(vk::PhysicalDeviceType type) noexcept
  -> std::uint32_t;

[[nodiscard]] auto IsVulkanDisplayAdapterMismatch(
  GGEMSVulkanDeviceCandidate const &candidate,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) noexcept
  -> bool;

[[nodiscard]] auto SelectVulkanDevice(
  GGEMSVulkanDeviceSelector const &selector,
  std::span<GGEMSVulkanDeviceCandidate const> candidates,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter)
  -> std::expected<GGEMSVulkanDeviceSelection, std::string>;

} // namespace ggems::ui::detail
