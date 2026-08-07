#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ggems::ui::detail {
struct GGEMSVulkanDeviceStatus {
  bool initialized{false};
  std::uint32_t enumeration_index{0U};
  std::string name{};
  std::string type{};
  std::string selection_reason{};
};

struct GGEMSComputeDeviceStatus {
  std::size_t context_index{0U};
  std::string name{};
  std::string type{};
  std::string platform{};
  bool show_platform{false};
};

struct GGEMSComputeStatus {
  bool initialized{false};
  std::vector<GGEMSComputeDeviceStatus> devices{};
};

struct GGEMSDeviceStatusSnapshot {
  GGEMSVulkanDeviceStatus renderer{};
  GGEMSComputeStatus compute{};
};
} // namespace ggems::ui::detail
