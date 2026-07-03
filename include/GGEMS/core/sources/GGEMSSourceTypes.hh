#pragma once

#include <cstdint>
#include <string_view>

namespace ggems::core::sources {

enum class GGEMSSourceType : std::uint32_t {
  Unknown = 0U,
  Analytic = 1U,
  Voxelized = 2U,
  PhaseSpace = 3U
};

constexpr std::uint32_t
ToKernelSourceType(GGEMSSourceType source_type) noexcept {
  return static_cast<std::uint32_t>(source_type);
}

constexpr GGEMSSourceType
FromKernelSourceType(std::uint32_t const source_type) noexcept {
  switch (source_type) {
  case 1U:
    return GGEMSSourceType::Analytic;
  case 2U:
    return GGEMSSourceType::Voxelized;
  case 3U:
    return GGEMSSourceType::PhaseSpace;
  default:
    return GGEMSSourceType::Unknown;
  }
}

constexpr std::string_view
ToLongName(GGEMSSourceType const source_type) noexcept {
  switch (source_type) {
  case GGEMSSourceType::Unknown:
    return "Unknown";
  case GGEMSSourceType::Analytic:
    return "Analytic";
  case GGEMSSourceType::Voxelized:
    return "Voxelized";
  case GGEMSSourceType::PhaseSpace:
    return "PhaseSpace";
  }

  return "Unknown";
}

} // namespace ggems::core::sources
