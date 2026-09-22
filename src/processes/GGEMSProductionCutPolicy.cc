#include <algorithm>
#include <format>
#include <iterator>
#include <optional>

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

} // namespace

// =============================================================================
// =============================================================================

auto RequireAdmissibleProductionCutPolicy(
    GGEMSProductionCutPolicy const &policy) -> void {
  for (auto const channel : k_production_cut_channels) {
    if (!FindLength(policy.global, channel).has_value()) {
      throw GGEMSRecoverable{
          std::format("Global Production-Cut policy has no {} length.",
                      ProductionCutChannelName(channel))};
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

auto ResolveProductionCuts(GGEMSProductionCutPolicy const &policy,
                           GGEMSProductionCutContext const &context)
    -> GGEMSResolvedProductionCuts {
  RequireAdmissibleProductionCutPolicy(policy);

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

// =============================================================================
// =============================================================================

auto ResolveProductionCutLengths(GGEMSProductionCutPolicy const &policy,
                                 GGEMSProductionCutContext const &context)
    -> GGEMSResolvedProductionCutLengths {
  return ResolveProductionCuts(policy, context).lengths;
}

} // namespace ggems::core::processes
