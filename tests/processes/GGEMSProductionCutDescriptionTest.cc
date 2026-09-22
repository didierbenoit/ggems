#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutDescription.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace processes = ggems::core::processes;

using namespace ggems::units;
using Channel = processes::GGEMSProductionCutChannel;
using Scope = processes::GGEMSProductionCutScope;

constexpr std::uint32_t k_water{0U};
constexpr std::uint32_t k_water_alias{1U};
constexpr std::uint32_t k_aluminum{2U};

// =============================================================================
// =============================================================================

auto MakeMaterials() -> std::vector<materials::GGEMSMaterial> {
  auto const water = materials::builtins::BuildBuiltInMaterial("Water");
  return {
    water,
    materials::GGEMSMaterial{
      "WaterAlias",
      water.GetDensity(),
      {
        {.atomic_number = 1U, .mass_fraction = 0.111898L},
        {.atomic_number = 8U, .mass_fraction = 0.888102L},
      },
    },
    materials::builtins::BuildBuiltInMaterial("Aluminum"),
  };
}

// =============================================================================
// =============================================================================

// Aluminum: Gamma from Global, Electron from Material, Positron from Volume.
auto MakePolicy() -> processes::GGEMSProductionCutPolicy {
  return {
    .global =
      {
        .gamma = 1_mm,
        .electron = 1_mm,
        .positron = 1_mm,
        .proton = 1_mm,
      },
    .materials =
      {
        {
          .material_index = k_aluminum,
          .lengths =
            {
              .gamma = std::nullopt,
              .electron = 500_um,
              .positron = std::nullopt,
              .proton = std::nullopt,
            },
        },
      },
  };
}

// =============================================================================
// =============================================================================

auto MakeContexts() -> std::vector<processes::GGEMSProductionCutContext> {
  return {
    {.material_index = k_water, .volume = {}},
    {.material_index = k_water_alias, .volume = {}},
    {
      .material_index = k_aluminum,
      .volume =
        {
          .gamma = std::nullopt,
          .electron = std::nullopt,
          .positron = 2_mm,
          .proton = std::nullopt,
        },
    },
  };
}

} // namespace

// =============================================================================
// =============================================================================

class GGEMSProductionCutDescriptionTest : public ::testing::Test {
protected:
  std::vector<materials::GGEMSMaterial> materials_{MakeMaterials()};
  materials::GGEMSEMMaterialPackage em_package_{materials_};
  std::vector<processes::GGEMSProductionCutContext> contexts_{MakeContexts()};
  processes::GGEMSMaterialCutCouplePackage package_{em_package_, MakePolicy(),
                                                    contexts_};
};

// =============================================================================
// =============================================================================

TEST_F(GGEMSProductionCutDescriptionTest, InspectsTheCompiledContext) {
  auto const inspection = processes::InspectProductionCutContext(package_, 2U);

  auto const couple_id = package_.GetContextCoupleIds()[2];
  auto const &couple = package_.GetCouples()[couple_id];

  EXPECT_EQ(inspection.context_index, 2U);
  EXPECT_EQ(inspection.authoring_material_index, k_aluminum);
  EXPECT_EQ(inspection.snapshot_material_id,
            em_package_.GetMaterialIds()[k_aluminum]);
  EXPECT_EQ(inspection.snapshot_couple_id, couple_id);

  auto const resolved =
    processes::ResolveProductionCuts(MakePolicy(), contexts_[2]);
  EXPECT_EQ(resolved.scopes, (std::array{Scope::Global, Scope::Material,
                                         Scope::Volume, Scope::Global}));

  for (auto const channel : processes::k_production_cut_channels) {
    auto const index = processes::ProductionCutChannelIndex(channel);
    auto const &row = inspection.channels[index];

    EXPECT_EQ(row.channel, channel);
    EXPECT_EQ(row.effective_length, resolved.lengths[index]);
    EXPECT_EQ(row.winning_scope, resolved.scopes[index]);
    EXPECT_EQ(row.production_threshold, couple.thresholds[index]);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSProductionCutDescriptionTest, DescribesLengthScopeAndThreshold) {
  auto const inspection = processes::InspectProductionCutContext(package_, 2U);
  auto const description =
    processes::DescribeProductionCutContext(package_, 2U);

  EXPECT_TRUE(description.contains("Material/Cut context 2"));
  EXPECT_TRUE(description.contains("Material index (authoring)   : 2"));
  EXPECT_TRUE(description.contains(std::format(
    "Material ID (snapshot-local) : {}", inspection.snapshot_material_id)));
  EXPECT_TRUE(description.contains(std::format(
    "Couple ID (snapshot-local)   : {}", inspection.snapshot_couple_id)));

  for (auto const &row : inspection.channels) {
    EXPECT_TRUE(description.contains(
      std::format("{:<8} : length {} ({}) -> production threshold {}",
                  processes::ProductionCutChannelName(row.channel),
                  HumanReadable(row.effective_length),
                  processes::ProductionCutScopeName(row.winning_scope),
                  HumanReadable(row.production_threshold))));
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSProductionCutDescriptionTest,
       DistinguishesAuthoringIndexFromSnapshotMaterialId) {
  auto const water = processes::InspectProductionCutContext(package_, 0U);
  auto const alias = processes::InspectProductionCutContext(package_, 1U);

  EXPECT_EQ(water.authoring_material_index, k_water);
  EXPECT_EQ(alias.authoring_material_index, k_water_alias);
  EXPECT_EQ(water.snapshot_material_id, alias.snapshot_material_id);
  EXPECT_EQ(water.snapshot_couple_id, alias.snapshot_couple_id);

  EXPECT_TRUE(processes::DescribeProductionCutContext(package_, 1U)
                .contains("Material index (authoring)   : 1"));
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSProductionCutDescriptionTest, DescribesProductionNotTracking) {
  for (std::size_t context = 0U; context < contexts_.size(); ++context) {
    auto const description =
      processes::DescribeProductionCutContext(package_, context);

    EXPECT_TRUE(description.contains("production threshold"));
    for (auto const *wording : {"kill", "cutoff", "cut-off", "tracking"}) {
      EXPECT_FALSE(description.contains(wording)) << wording;
    }
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSProductionCutDescriptionTest, RejectsUnknownContext) {
  EXPECT_THROW(static_cast<void>(processes::InspectProductionCutContext(
                 package_, contexts_.size())),
               ggems::core::GGEMSRecoverable);
}
