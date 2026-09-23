#include <array>
#include <cstddef>
#include <format>
#include <span>
#include <string>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutConverter.hh"
#include "GGEMS/processes/GGEMSProductionCutDescription.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"

namespace ggems::core::processes {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
InspectProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                            std::size_t context_index)
  -> GGEMSProductionCutContextInspection {
  auto const provenance = package.GetContextProvenance();

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

  std::string description =
    std::format("Material/Cut context {}"
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

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeProductionCuts() -> std::string {
  auto const &global = GetProductionCutPolicy().global;

  return std::format("Production Cuts"
                     "\n  Gamma    : {}"
                     "\n  Electron : {}"
                     "\n  Positron : {}"
                     "\n  Proton   : {}",
                     units::HumanReadable(*global.gamma),
                     units::HumanReadable(*global.electron),
                     units::HumanReadable(*global.positron),
                     units::HumanReadable(*global.proton));
}

// =============================================================================
// =============================================================================

auto VerboseProductionCuts() -> void {
  GGEMS_INFO("Cuts", "\n{}\n", DescribeProductionCuts());
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
DescribeProductionCutsForMaterial(materials::GGEMSMaterial const &material,
                                  units::Length reference_length)
  -> std::string {
  std::string description = std::format("  Production Cuts at {}",
                                        units::HumanReadable(reference_length));

  if (material.GetElementalConstituents().empty()) {
    description += "\n    not applicable";
    return description;
  }

  materials::GGEMSEMMaterialPackage const package{
    std::span<materials::GGEMSMaterial const>{&material, 1U}};

  for (auto const channel : k_production_cut_channels) {
    try {
      auto const threshold =
        ConvertProductionCutLength(channel, reference_length, package, 0U);

      description +=
        std::format("\n    {:<8} : {}", ProductionCutChannelName(channel),
                    units::HumanReadable(threshold));
    } catch (GGEMSRecoverable const &) {
      description += std::format("\n    {:<8} : outside converter domain",
                                 ProductionCutChannelName(channel));
    }
  }

  return description;
}

} // namespace ggems::core::processes
