#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

namespace builtins = ggems::core::materials::builtins;

using namespace ggems::units;

constexpr std::array<std::string_view, 24U> k_compound_names{{
    "Air",           "Water",         "Adipose",       "Blood", "BloodIodine5",
    "BloodIodine10", "BloodIodine15", "BloodIodine20", "Brain", "Breast",
    "Heart",         "Intestine",     "Kidney",        "Liver", "Lung",
    "RibBone",       "SpineBone",     "Spleen",        "CdTe",  "CsI",
    "GaAs",          "GOS",           "LSO",           "NaI",
}};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, ExposesCanonicalNamesInRegistryOrder) {
  auto const available = builtins::GetAvailableMaterialNames();

  ASSERT_EQ(available.size(), 117U);
  EXPECT_EQ(available.front(), "Vacuum");

  // Vacuum occupies slot 0; the 92 elemental Materials follow in Z order.
  EXPECT_EQ(available[1U], "Hydrogen");
  EXPECT_EQ(available[13U], "Aluminum");
  EXPECT_EQ(available[53U], "Iodine");
  EXPECT_EQ(available[74U], "Tungsten");
  EXPECT_EQ(available[92U], "Uranium");

  constexpr std::size_t k_compound_offset{93U};
  for (std::size_t index = 0U; index < k_compound_names.size(); ++index) {
    EXPECT_EQ(available[k_compound_offset + index], k_compound_names[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsEveryAvailableMaterial) {
  for (auto const name : builtins::GetAvailableMaterialNames()) {
    SCOPED_TRACE(name);
    EXPECT_NO_THROW(static_cast<void>(builtins::BuildBuiltInMaterial(name)));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsVacuum) {
  auto const material = builtins::BuildBuiltInMaterial("Vacuum");

  EXPECT_EQ(material.GetName(), "Vacuum");
  EXPECT_EQ(material.GetDensity(), 0.0_g_cm3);
  EXPECT_TRUE(material.GetConstituents().empty());
  EXPECT_EQ(material.GetTotalAtomDensityPerCubicCentimeter(), 0.0L);
  EXPECT_EQ(material.GetElectronDensityPerCubicCentimeter(), 0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsCommonMedia) {
  auto const water = builtins::BuildBuiltInMaterial("Water");
  EXPECT_EQ(water.GetDensity(), 1.000_g_cm3);
  ASSERT_EQ(water.GetConstituents().size(), 2U);
  EXPECT_EQ(water.GetConstituents()[0U].atomic_number, 1U);
  EXPECT_EQ(water.GetConstituents()[0U].mass_fraction, 0.111898L);
  EXPECT_EQ(water.GetConstituents()[1U].atomic_number, 8U);
  EXPECT_EQ(water.GetConstituents()[1U].mass_fraction, 0.888102L);

  auto const air = builtins::BuildBuiltInMaterial("Air");
  EXPECT_EQ(air.GetDensity(), 1.205e-3_g_cm3);
  ASSERT_EQ(air.GetConstituents().size(), 4U);
  EXPECT_EQ(air.GetConstituents()[0U].atomic_number, 6U);
  EXPECT_EQ(air.GetConstituents()[1U].atomic_number, 7U);
  EXPECT_EQ(air.GetConstituents()[2U].atomic_number, 8U);
  EXPECT_EQ(air.GetConstituents()[3U].atomic_number, 18U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsRepresentativeElementalMaterials) {
  struct ExpectedMaterial {
    std::string_view name;
    std::uint32_t atomic_number;
    Density density;
  };

  constexpr std::array expected{
      ExpectedMaterial{
          .name = "Aluminum", .atomic_number = 13U, .density = 2.699_g_cm3},
      ExpectedMaterial{
          .name = "Silicon", .atomic_number = 14U, .density = 2.330_g_cm3},
      ExpectedMaterial{
          .name = "Copper", .atomic_number = 29U, .density = 8.960_g_cm3},
      ExpectedMaterial{
          .name = "Tungsten", .atomic_number = 74U, .density = 19.30_g_cm3},
      ExpectedMaterial{
          .name = "Lead", .atomic_number = 82U, .density = 11.35_g_cm3},
      ExpectedMaterial{
          .name = "Uranium", .atomic_number = 92U, .density = 18.95_g_cm3},
  };

  for (auto const &expected_material : expected) {
    auto const material =
        builtins::BuildBuiltInMaterial(expected_material.name);

    SCOPED_TRACE(expected_material.name);

    EXPECT_EQ(material.GetName(), expected_material.name);
    EXPECT_EQ(material.GetDensity(), expected_material.density);

    auto const constituents = material.GetConstituents();
    ASSERT_EQ(constituents.size(), 1U);
    EXPECT_EQ(constituents.front().atomic_number,
              expected_material.atomic_number);
    EXPECT_EQ(constituents.front().mass_fraction, 1.0L);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsRepresentativeMedicalMaterials) {
  auto const brain = builtins::BuildBuiltInMaterial("Brain");
  EXPECT_EQ(brain.GetDensity(), 1.03_g_cm3);
  EXPECT_EQ(brain.GetConstituents().size(), 13U);

  auto const lung = builtins::BuildBuiltInMaterial("Lung");
  EXPECT_EQ(lung.GetDensity(), 0.26_g_cm3);
  EXPECT_EQ(lung.GetConstituents().size(), 9U);

  auto const lso = builtins::BuildBuiltInMaterial("LSO");
  EXPECT_EQ(lso.GetDensity(), 7.4_g_cm3);
  EXPECT_EQ(lso.GetConstituents().size(), 3U);

  auto const cdte = builtins::BuildBuiltInMaterial("CdTe");
  EXPECT_EQ(cdte.GetDensity(), 6.200_g_cm3);
  ASSERT_EQ(cdte.GetConstituents().size(), 2U);
  EXPECT_EQ(cdte.GetConstituents()[0U].atomic_number, 48U);
  EXPECT_EQ(cdte.GetConstituents()[0U].mass_fraction, 0.468358L);
  EXPECT_EQ(cdte.GetConstituents()[1U].atomic_number, 52U);
  EXPECT_EQ(cdte.GetConstituents()[1U].mass_fraction, 0.531642L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, RejectsUnknownOrInexactNames) {
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial("water")),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial(" Water ")),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial("Aluminium")),
               ggems::core::GGEMSRecoverable);

  // NIST labels are source descriptions, not public GGEMS canonical names.
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial(
                   "Brain, Grey/White Matter (ICRU-44)")),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(builtins::BuildBuiltInMaterial("Cadmium Telluride")),
      ggems::core::GGEMSRecoverable);

  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial("Unknown")),
               ggems::core::GGEMSRecoverable);
}
