#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutConverter.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace processes = ggems::core::processes;
namespace units = ggems::units;

using namespace ggems::units;
using Channel = processes::GGEMSProductionCutChannel;

// Authored Material indices. WaterAlias is scientifically identical to Water
// under a different display name, so both intern to the same dense Material.
constexpr std::uint32_t k_water{0U};
constexpr std::uint32_t k_water_alias{1U};
constexpr std::uint32_t k_aluminum{2U};

static_assert(
    std::is_same_v<decltype(processes::GGEMSMaterialCutCouple::thresholds),
                   std::array<units::Energy, 4U>>);

// =============================================================================
// =============================================================================

auto MakeMaterials() -> std::vector<materials::GGEMSMaterial> {
  auto const water = materials::builtins::BuildBuiltInMaterial("Water");
  return {
      water,
      materials::GGEMSMaterial{
          "WaterAlias",
          water.GetDensity(),
          {{.atomic_number = 1U, .mass_fraction = 0.111898L},
           {.atomic_number = 8U, .mass_fraction = 0.888102L}}},
      materials::builtins::BuildBuiltInMaterial("Aluminum"),
  };
}

// =============================================================================
// =============================================================================

auto MakePolicy() -> processes::GGEMSProductionCutPolicy {
  return {
      .global = {.gamma = 1_mm,
                 .electron = 1_mm,
                 .positron = 1_mm,
                 .proton = 1_mm},
      .materials = {},
  };
}

// =============================================================================
// =============================================================================

auto CoupleOf(processes::GGEMSMaterialCutCouplePackage const &package,
              std::size_t context) -> processes::GGEMSMaterialCutCouple {
  return package.GetCouples()[package.GetContextCoupleIds()[context]];
}

} // namespace

// =============================================================================
// =============================================================================

class GGEMSMaterialCutCouplePackageTest : public ::testing::Test {
protected:
  std::vector<materials::GGEMSMaterial> materials_{MakeMaterials()};
  materials::GGEMSEMMaterialPackage em_package_{materials_};
};

// =============================================================================
// =============================================================================

TEST_F(GGEMSMaterialCutCouplePackageTest, ResolvesConvertsAndInternsContexts) {
  auto const policy = MakePolicy();
  std::vector<processes::GGEMSProductionCutContext> const contexts{
      {.material_index = k_water, .volume = {}},
      {.material_index = k_aluminum, .volume = {}},
      {.material_index = k_water, .volume = {}},
  };

  processes::GGEMSMaterialCutCouplePackage const package{em_package_, policy,
                                                         contexts};

  ASSERT_EQ(package.GetCouples().size(), 2U);
  ASSERT_EQ(package.GetContextCoupleIds().size(), contexts.size());
  EXPECT_EQ(package.GetContextCoupleIds()[0], package.GetContextCoupleIds()[2]);
  EXPECT_NE(package.GetContextCoupleIds()[0], package.GetContextCoupleIds()[1]);

  auto const water = CoupleOf(package, 0U);
  EXPECT_EQ(water.material_id, em_package_.GetMaterialIds()[k_water]);
  for (auto const channel : processes::k_production_cut_channels) {
    EXPECT_EQ(water.thresholds[processes::ProductionCutChannelIndex(channel)],
              processes::ConvertProductionCutLength(channel, 1_mm, em_package_,
                                                    water.material_id));
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSMaterialCutCouplePackageTest, OneDifferentThresholdGivesNewCouple) {
  auto const policy = MakePolicy();
  std::vector<processes::GGEMSProductionCutContext> const contexts{
      {.material_index = k_water, .volume = {}},
      {.material_index = k_water, .volume = {.positron = 2_mm}},
  };

  processes::GGEMSMaterialCutCouplePackage const package{em_package_, policy,
                                                         contexts};

  ASSERT_EQ(package.GetCouples().size(), 2U);
  auto const first = CoupleOf(package, 0U);
  auto const second = CoupleOf(package, 1U);
  EXPECT_EQ(first.material_id, second.material_id);

  auto const positron = processes::ProductionCutChannelIndex(Channel::Positron);
  for (auto const channel : processes::k_production_cut_channels) {
    auto const index = processes::ProductionCutChannelIndex(channel);
    if (index == positron) {
      EXPECT_LT(first.thresholds[index], second.thresholds[index]);
    } else {
      EXPECT_EQ(first.thresholds[index], second.thresholds[index]);
    }
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSMaterialCutCouplePackageTest,
       DifferentMaterialsWithEqualThresholdsStayDistinct) {
  // Water and Aluminum share their material-independent Proton threshold, and
  // a couple carrying Water thresholds with the Aluminum ID is not Water.
  auto const policy = MakePolicy();
  std::vector<processes::GGEMSProductionCutContext> const contexts{
      {.material_index = k_water, .volume = {}},
      {.material_index = k_aluminum, .volume = {}},
  };

  processes::GGEMSMaterialCutCouplePackage const package{em_package_, policy,
                                                         contexts};

  auto const water = CoupleOf(package, 0U);
  auto const aluminum = CoupleOf(package, 1U);
  auto const proton = processes::ProductionCutChannelIndex(Channel::Proton);
  EXPECT_EQ(water.thresholds[proton], aluminum.thresholds[proton]);
  EXPECT_NE(water.material_id, aluminum.material_id);
  EXPECT_NE(package.GetContextCoupleIds()[0], package.GetContextCoupleIds()[1]);

  processes::GGEMSMaterialCutCouple const same_thresholds{
      .material_id = aluminum.material_id, .thresholds = water.thresholds};
  EXPECT_NE(same_thresholds, water);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSMaterialCutCouplePackageTest,
       DisplayNameAndProvenanceDoNotDefineIdentity) {
  auto policy = MakePolicy();
  policy.global.electron = 3_mm;
  policy.materials.push_back(
      {.material_index = k_water_alias, .lengths = {.electron = 1_mm}});

  std::vector<processes::GGEMSProductionCutContext> const contexts{
      // Water, Electron 1 mm from a Volume override.
      {.material_index = k_water, .volume = {.electron = 1_mm}},
      // WaterAlias, Electron 1 mm from a Material override.
      {.material_index = k_water_alias, .volume = {}},
      // WaterAlias, Global Electron 3 mm overridden back to 1 mm by Volume.
      {.material_index = k_water_alias, .volume = {.electron = 1_mm}},
  };

  processes::GGEMSMaterialCutCouplePackage const package{em_package_, policy,
                                                         contexts};

  ASSERT_EQ(package.GetCouples().size(), 1U);
  EXPECT_EQ(package.GetContextCoupleIds()[0], 0U);
  EXPECT_EQ(package.GetContextCoupleIds()[1], 0U);
  EXPECT_EQ(package.GetContextCoupleIds()[2], 0U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSMaterialCutCouplePackageTest, IdsAreIndependentOfContextOrder) {
  auto policy = MakePolicy();
  policy.materials.push_back(
      {.material_index = k_aluminum, .lengths = {.gamma = 100_um}});

  std::vector<processes::GGEMSProductionCutContext> contexts{
      {.material_index = k_aluminum, .volume = {}},
      {.material_index = k_water, .volume = {.electron = 100_um}},
      {.material_index = k_water, .volume = {}},
      {.material_index = k_aluminum, .volume = {.proton = 10_um}},
      {.material_index = k_water_alias, .volume = {}},
  };

  processes::GGEMSMaterialCutCouplePackage const reference{em_package_, policy,
                                                           contexts};
  std::vector<processes::GGEMSMaterialCutCouple> const couples(
      reference.GetCouples().begin(), reference.GetCouples().end());

  EXPECT_TRUE(std::ranges::is_sorted(couples));
  EXPECT_EQ(couples.size(), 4U);

  std::vector<std::size_t> order{0U, 1U, 2U, 3U, 4U};
  do {
    std::vector<processes::GGEMSProductionCutContext> permuted;
    permuted.reserve(order.size());
    for (auto const index : order) {
      permuted.push_back(contexts[index]);
    }

    processes::GGEMSMaterialCutCouplePackage const package{em_package_, policy,
                                                           permuted};

    EXPECT_TRUE(std::ranges::equal(package.GetCouples(), couples));
    for (std::size_t position = 0U; position < order.size(); ++position) {
      EXPECT_EQ(package.GetContextCoupleIds()[position],
                reference.GetContextCoupleIds()[order[position]]);
    }
  } while (std::ranges::next_permutation(order).found);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSMaterialCutCouplePackageTest, InvalidInputsAreRejected) {
  std::vector<processes::GGEMSProductionCutContext> const unknown_context{
      {.material_index = 3U, .volume = {}}};
  EXPECT_THROW((processes::GGEMSMaterialCutCouplePackage{
                   em_package_, MakePolicy(), unknown_context}),
               ggems::core::GGEMSRecoverable);

  auto unknown_override = MakePolicy();
  unknown_override.materials.push_back(
      {.material_index = 3U, .lengths = {.gamma = 1_mm}});
  std::vector<processes::GGEMSProductionCutContext> const water_context{
      {.material_index = k_water, .volume = {}}};
  EXPECT_THROW((processes::GGEMSMaterialCutCouplePackage{
                   em_package_, unknown_override, water_context}),
               ggems::core::GGEMSRecoverable);

  auto incomplete = MakePolicy();
  incomplete.global.proton.reset();
  EXPECT_THROW((processes::GGEMSMaterialCutCouplePackage{
                   em_package_, incomplete, water_context}),
               ggems::core::GGEMSRecoverable);

  std::vector<processes::GGEMSProductionCutContext> const no_context{};
  EXPECT_THROW((processes::GGEMSMaterialCutCouplePackage{
                   em_package_, incomplete, no_context}),
               ggems::core::GGEMSRecoverable);

  auto duplicate = MakePolicy();
  duplicate.materials.push_back(
      {.material_index = k_aluminum, .lengths = {.gamma = 1_mm}});
  duplicate.materials.push_back(
      {.material_index = k_aluminum, .lengths = {.proton = 1_mm}});
  EXPECT_THROW((processes::GGEMSMaterialCutCouplePackage{em_package_, duplicate,
                                                         no_context}),
               ggems::core::GGEMSRecoverable);

  std::vector<processes::GGEMSProductionCutContext> const below_domain{
      {.material_index = k_water, .volume = {.gamma = 1_nm}}};
  EXPECT_THROW((processes::GGEMSMaterialCutCouplePackage{
                   em_package_, MakePolicy(), below_domain}),
               ggems::core::GGEMSRecoverable);
}
