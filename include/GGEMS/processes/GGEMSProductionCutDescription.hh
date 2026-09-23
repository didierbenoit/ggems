#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

struct GGEMSProductionCutChannelInspection {
  GGEMSProductionCutChannel channel;
  units::Length effective_length;
  GGEMSProductionCutScope winning_scope;
  units::Energy production_threshold;
};

struct GGEMSProductionCutContextInspection {
  std::size_t context_index;
  std::uint32_t authoring_material_index;
  std::uint32_t snapshot_material_id;
  std::uint32_t snapshot_couple_id;
  std::array<GGEMSProductionCutChannelInspection, 4U> channels;
};

[[nodiscard]] auto
InspectProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                            std::size_t context_index)
  -> GGEMSProductionCutContextInspection;

[[nodiscard]] auto
DescribeProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                             std::size_t context_index) -> std::string;

auto VerboseProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                                 std::size_t context_index) -> void;

[[nodiscard]] auto DescribeProductionCuts() -> std::string;

auto VerboseProductionCuts() -> void;

[[nodiscard]] auto
DescribeProductionCutsForMaterial(materials::GGEMSMaterial const &material,
                                  units::Length reference_length)
  -> std::string;

} // namespace ggems::core::processes
