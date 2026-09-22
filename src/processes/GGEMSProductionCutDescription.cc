#include <array>
#include <cstddef>
#include <format>
#include <string>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutDescription.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace ggems::core::processes {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
InspectProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                            std::size_t context_index)
    -> GGEMSProductionCutContextInspection {
  auto const provenance = package.GetContextProvenance();

  if (context_index >= provenance.size()) {
    throw GGEMSRecoverable{
        std::format("Unknown Production-Cut context {}.", context_index)};
  }

  auto const &context = provenance[context_index];
  auto const couple_id = package.GetContextCoupleIds()[context_index];
  auto const &couple = package.GetCouples()[couple_id];

  std::array<GGEMSProductionCutChannelInspection, 4U> channels{};

  for (auto const channel : k_production_cut_channels) {
    auto const index = ProductionCutChannelIndex(channel);

    channels[index] = {
        .channel = channel,
        .effective_length = context.cuts.lengths[index],
        .winning_scope = context.cuts.scopes[index],
        .production_threshold = couple.thresholds[index],
    };
  }

  return {
      .context_index = context_index,
      .authoring_material_index = context.material_index,
      .snapshot_material_id = couple.material_id,
      .snapshot_couple_id = couple_id,
      .channels = channels,
  };
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
DescribeProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                             std::size_t context_index) -> std::string {
  auto const inspection = InspectProductionCutContext(package, context_index);

  std::string description = std::format(
      "Material/Cut context {}"
      "\n  Material index (authoring)   : {}"
      "\n  Material ID (snapshot-local) : {}"
      "\n  Couple ID (snapshot-local)   : {}",
      inspection.context_index, inspection.authoring_material_index,
      inspection.snapshot_material_id, inspection.snapshot_couple_id);

  for (auto const &channel : inspection.channels) {
    description +=
        std::format("\n  {:<8} : length {} ({}) -> production threshold {}",
                    ProductionCutChannelName(channel.channel),
                    units::HumanReadable(channel.effective_length),
                    ProductionCutScopeName(channel.winning_scope),
                    units::HumanReadable(channel.production_threshold));
  }

  return description;
}

// =============================================================================
// =============================================================================

auto VerboseProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                                 std::size_t context_index) -> void {
  GGEMS_INFO("Cuts", "{}",
             DescribeProductionCutContext(package, context_index));
}

} // namespace ggems::core::processes
