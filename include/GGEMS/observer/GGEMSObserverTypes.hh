#pragma once

#include <cstdint>
#include <string_view>

namespace ggems::core::observer {

enum class GGEMSObserverRecordKind : std::uint32_t {
  Unknown = 0U,
  Source = 1U,
  Step = 2U,
  SecondaryStep = 3U,
  Terminal = 4U,
  Anomaly = 5U
};

constexpr auto
ToKernelObserverRecordKind(GGEMSObserverRecordKind record_kind) noexcept
    -> std::uint32_t {
  return static_cast<std::uint32_t>(record_kind);
}

constexpr auto FromKernelObserverRecordKind(std::uint32_t record_kind) noexcept
    -> GGEMSObserverRecordKind {
  switch (record_kind) {
  case 1U:
    return GGEMSObserverRecordKind::Source;
  case 2U:
    return GGEMSObserverRecordKind::Step;
  case 3U:
    return GGEMSObserverRecordKind::SecondaryStep;
  case 4U:
    return GGEMSObserverRecordKind::Terminal;
  case 5U:
    return GGEMSObserverRecordKind::Anomaly;
  default:
    return GGEMSObserverRecordKind::Unknown;
  }
}

constexpr auto ToLongName(GGEMSObserverRecordKind record_kind) noexcept
    -> std::string_view {
  switch (record_kind) {
  case GGEMSObserverRecordKind::Unknown:
    return "Unknown";
  case GGEMSObserverRecordKind::Source:
    return "Source";
  case GGEMSObserverRecordKind::Step:
    return "Step";
  case GGEMSObserverRecordKind::SecondaryStep:
    return "SecondaryStep";
  case GGEMSObserverRecordKind::Terminal:
    return "Terminal";
  case GGEMSObserverRecordKind::Anomaly:
    return "Anomaly";
  }

  return "Unknown";
}

} // namespace ggems::core::observer
