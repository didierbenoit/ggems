#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
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

auto RequireAdmissibleProductionCutPolicy(
    GGEMSProductionCutPolicy const &policy) -> void;

[[nodiscard]] auto
ResolveProductionCutLengths(GGEMSProductionCutPolicy const &policy,
                            GGEMSProductionCutContext const &context)
    -> GGEMSResolvedProductionCutLengths;

} // namespace ggems::core::processes
