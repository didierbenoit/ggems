#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

enum class GGEMSProductionCutChannel : std::uint8_t {
  Gamma = 0U,
  Electron = 1U,
  Positron = 2U,
  Proton = 3U,
};

inline constexpr std::array<GGEMSProductionCutChannel, 4U>
  k_production_cut_channels{
    GGEMSProductionCutChannel::Gamma,
    GGEMSProductionCutChannel::Electron,
    GGEMSProductionCutChannel::Positron,
    GGEMSProductionCutChannel::Proton,
};

[[nodiscard]] constexpr auto
ProductionCutChannelIndex(GGEMSProductionCutChannel channel) noexcept
  -> std::size_t {
  return static_cast<std::size_t>(channel);
}

[[nodiscard]] constexpr auto
ProductionCutChannelName(GGEMSProductionCutChannel channel) noexcept
  -> std::string_view {
  switch (channel) {
  case GGEMSProductionCutChannel::Gamma:
    return "Gamma";
  case GGEMSProductionCutChannel::Electron:
    return "Electron";
  case GGEMSProductionCutChannel::Positron:
    return "Positron";
  case GGEMSProductionCutChannel::Proton:
    return "Proton";
  }
  return "Unknown";
}

enum class GGEMSProductionCutScope : std::uint8_t {
  Global = 0U,
  Material = 1U,
  Volume = 2U,
};

[[nodiscard]] constexpr auto
ProductionCutScopeName(GGEMSProductionCutScope scope) noexcept
  -> std::string_view {
  switch (scope) {
  case GGEMSProductionCutScope::Global:
    return "Global";
  case GGEMSProductionCutScope::Material:
    return "Material";
  case GGEMSProductionCutScope::Volume:
    return "Volume";
  }
  return "Unknown";
}

struct GGEMSProductionCutLengths {
  std::optional<units::Length> gamma;
  std::optional<units::Length> electron;
  std::optional<units::Length> positron;
  std::optional<units::Length> proton;
};

struct GGEMSMaterialProductionCuts {
  std::uint32_t material_index;
  GGEMSProductionCutLengths lengths;
};

struct GGEMSProductionCutPolicy {
  GGEMSProductionCutLengths global;
  std::vector<GGEMSMaterialProductionCuts> materials;
};

struct GGEMSProductionCutContext {
  std::uint32_t material_index;
  GGEMSProductionCutLengths volume;
};

using GGEMSResolvedProductionCutLengths = std::array<units::Length, 4U>;

struct GGEMSResolvedProductionCuts {
  GGEMSResolvedProductionCutLengths lengths;
  std::array<GGEMSProductionCutScope, 4U> scopes;
};

auto RequireAdmissibleProductionCutPolicy(
  GGEMSProductionCutPolicy const &policy) -> void;

[[nodiscard]] auto
ResolveProductionCuts(GGEMSProductionCutPolicy const &policy,
                      GGEMSProductionCutContext const &context)
  -> GGEMSResolvedProductionCuts;

[[nodiscard]] auto
ResolveProductionCutLengths(GGEMSProductionCutPolicy const &policy,
                            GGEMSProductionCutContext const &context)
  -> GGEMSResolvedProductionCutLengths;

} // namespace ggems::core::processes
