#include <algorithm>
#include <optional>

#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

namespace {

GGEMSProductionCutPolicy g_production_cut_policy{};

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

} // namespace

// =============================================================================
// =============================================================================

auto SetProductionCuts(GGEMSProductionCutLengths const &cuts) -> void {
  if (cuts.gamma.has_value()) {
    g_production_cut_policy.global.gamma = cuts.gamma;
  }
  if (cuts.electron.has_value()) {
    g_production_cut_policy.global.electron = cuts.electron;
  }
  if (cuts.positron.has_value()) {
    g_production_cut_policy.global.positron = cuts.positron;
  }
  if (cuts.proton.has_value()) {
    g_production_cut_policy.global.proton = cuts.proton;
  }
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetProductionCutPolicy() noexcept
  -> GGEMSProductionCutPolicy const & {
  return g_production_cut_policy;
}

// =============================================================================
// =============================================================================

auto ResolveProductionCuts(GGEMSProductionCutPolicy const &policy,
                           GGEMSProductionCutContext const &context)
  -> GGEMSResolvedProductionCuts {
  auto const material_override =
    std::ranges::find(policy.materials, context.material_index,
                      &GGEMSMaterialProductionCuts::material_index);

  auto const *material = material_override != policy.materials.end()
                           ? &material_override->lengths
                           : nullptr;

  GGEMSResolvedProductionCuts resolved{};

  for (auto const channel : k_production_cut_channels) {
    auto const index = ProductionCutChannelIndex(channel);

    auto length = FindLength(context.volume, channel);
    auto scope = GGEMSProductionCutScope::Volume;

    if (!length.has_value() && material != nullptr) {
      length = FindLength(*material, channel);
      scope = GGEMSProductionCutScope::Material;
    }

    if (!length.has_value()) {
      length = FindLength(policy.global, channel);
      scope = GGEMSProductionCutScope::Global;
    }

    resolved.lengths[index] = *length;
    resolved.scopes[index] = scope;
  }

  return resolved;
}

} // namespace ggems::core::processes
