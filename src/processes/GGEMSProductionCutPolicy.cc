#include <algorithm>
#include <format>
#include <iterator>
#include <optional>
#include <string_view>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindLength(GGEMSProductionCutLengths const &lengths,
                              GGEMSProductionCutChannel channel) noexcept
    -> std::optional<units::Length> {
  switch (channel) {
  case GGEMSProductionCutChannel::Gamma:
    return lengths.gamma;
  case GGEMSProductionCutChannel::Electron:
    return lengths.electron;
  case GGEMSProductionCutChannel::Positron:
    return lengths.positron;
  case GGEMSProductionCutChannel::Proton:
    return lengths.proton;
  }
  return std::nullopt;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ChannelName(GGEMSProductionCutChannel channel) noexcept
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

} // namespace

// =============================================================================
// =============================================================================

auto RequireAdmissibleProductionCutPolicy(
    GGEMSProductionCutPolicy const &policy) -> void {
  for (auto const channel : k_production_cut_channels) {
    if (!FindLength(policy.global, channel).has_value()) {
      throw GGEMSRecoverable{
          std::format("Global Production-Cut policy has no {} length.",
                      ChannelName(channel))};
    }
  }

  auto const &materials = policy.materials;
  for (auto first = materials.begin(); first != materials.end(); ++first) {
    for (auto second = std::next(first); second != materials.end(); ++second) {
      if (first->material_index == second->material_index) {
        throw GGEMSRecoverable{std::format(
            "Production-Cut policy defines Material {} more than once.",
            first->material_index)};
      }
    }
  }
}

// =============================================================================
// =============================================================================

auto ResolveProductionCutLengths(GGEMSProductionCutPolicy const &policy,
                                 GGEMSProductionCutContext const &context)
    -> GGEMSResolvedProductionCutLengths {
  RequireAdmissibleProductionCutPolicy(policy);

  auto const material_override =
      std::ranges::find(policy.materials, context.material_index,
                        &GGEMSMaterialProductionCuts::material_index);
  auto const *material = material_override != policy.materials.end()
                             ? &material_override->lengths
                             : nullptr;

  GGEMSResolvedProductionCutLengths resolved{};

  for (auto const channel : k_production_cut_channels) {
    auto length = FindLength(context.volume, channel);

    if (!length.has_value() && material != nullptr) {
      length = FindLength(*material, channel);
    }

    resolved[ProductionCutChannelIndex(channel)] =
        length.value_or(*FindLength(policy.global, channel));
  }

  return resolved;
}

} // namespace ggems::core::processes
