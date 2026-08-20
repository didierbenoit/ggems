#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace units = ggems::units;

using namespace ggems::units;

constexpr long double k_avogadro_constant_per_mole{6.02214076e23L};
constexpr long double k_relative_tolerance_multiplier{64.0L};

// =============================================================================
// =============================================================================

auto ExpectRelativeNear(long double actual, long double expected) -> void {
  long double const tolerance = std::abs(expected) *
                                k_relative_tolerance_multiplier *
                                std::numeric_limits<long double>::epsilon();
  EXPECT_LE(std::abs(actual - expected), tolerance);
}

// =============================================================================
// =============================================================================

auto ConstructMaterial(
    std::string name, units::Density density,
    std::vector<materials::GGEMSMaterialComponent> composition)
    -> materials::GGEMSMaterial {
  return materials::GGEMSMaterial{std::move(name), density,
                                  std::move(composition)};
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, PureElementComputesNumberDensities) {
  materials::GGEMSMaterial const material{
      "carbon", 2.0_g_cm3, {{.atomic_number = 6U, .mass_fraction = 1.0L}}};

  EXPECT_EQ(material.GetName(), "carbon");
  EXPECT_EQ(material.GetDensity(), 2.0_g_cm3);

  auto const constituents = material.GetConstituents();
  ASSERT_EQ(constituents.size(), 1U);
  EXPECT_EQ(constituents.front().atomic_number, 6U);
  EXPECT_EQ(constituents.front().mass_fraction, 1.0L);

  long double const expected_number_density =
      k_avogadro_constant_per_mole * 2.0L / 12.011L;
  ExpectRelativeNear(constituents.front().number_density_per_cubic_centimeter,
                     expected_number_density);
  ExpectRelativeNear(material.GetTotalAtomDensityPerCubicCentimeter(),
                     expected_number_density);
  ExpectRelativeNear(material.GetElectronDensityPerCubicCentimeter(),
                     expected_number_density * 6.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, BinaryMixtureComputesNumberDensities) {
  materials::GGEMSMaterial const material{
      "hydrogen-oxygen mixture",
      1.25_g_cm3,
      {{.atomic_number = 1U, .mass_fraction = 0.25L},
       {.atomic_number = 8U, .mass_fraction = 0.75L}}};

  auto const constituents = material.GetConstituents();
  ASSERT_EQ(constituents.size(), 2U);
  EXPECT_EQ(constituents[0].mass_fraction, 0.25L);
  EXPECT_EQ(constituents[1].mass_fraction, 0.75L);

  long double const expected_hydrogen_number_density =
      k_avogadro_constant_per_mole * 1.25L * 0.25L / 1.0080L;
  long double const expected_oxygen_number_density =
      k_avogadro_constant_per_mole * 1.25L * 0.75L / 15.999L;
  long double const expected_total_atom_density =
      expected_hydrogen_number_density + expected_oxygen_number_density;
  long double const expected_electron_density =
      expected_hydrogen_number_density +
      (8.0L * expected_oxygen_number_density);

  ExpectRelativeNear(constituents[0].number_density_per_cubic_centimeter,
                     expected_hydrogen_number_density);
  ExpectRelativeNear(constituents[1].number_density_per_cubic_centimeter,
                     expected_oxygen_number_density);
  ExpectRelativeNear(material.GetTotalAtomDensityPerCubicCentimeter(),
                     expected_total_atom_density);
  ExpectRelativeNear(material.GetElectronDensityPerCubicCentimeter(),
                     expected_electron_density);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, StoresConstituentsInAtomicNumberOrder) {
  materials::GGEMSMaterial const material{
      "reverse input",
      1.0_g_cm3,
      {{.atomic_number = 8U, .mass_fraction = 0.75L},
       {.atomic_number = 1U, .mass_fraction = 0.25L}}};

  auto const constituents = material.GetConstituents();
  ASSERT_EQ(constituents.size(), 2U);
  EXPECT_EQ(constituents[0].atomic_number, 1U);
  EXPECT_EQ(constituents[1].atomic_number, 8U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, NormalizesNearUnitMassFractionSum) {
  materials::GGEMSMaterial const material{
      "near-unit carbon",
      1.0_g_cm3,
      {{.atomic_number = 6U, .mass_fraction = 1.0L - 5.0e-6L}}};

  auto const constituents = material.GetConstituents();
  ASSERT_EQ(constituents.size(), 1U);
  EXPECT_EQ(constituents.front().mass_fraction, 1.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, RejectsInvalidInput) {
  auto const valid_density = 1.0_g_cm3;
  std::vector<materials::GGEMSMaterialComponent> const valid_composition{
      {.atomic_number = 6U, .mass_fraction = 1.0L}};

  EXPECT_THROW(static_cast<void>(
                   ConstructMaterial("", valid_density, valid_composition)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(ConstructMaterial(
          "zero density", units::Density{.value = 0.0L}, valid_composition)),
      ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(ConstructMaterial(
                   "negative density", units::Density{.value = -1.0L},
                   valid_composition)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(ConstructMaterial(
          "infinite density",
          units::Density{.value = std::numeric_limits<long double>::infinity()},
          valid_composition)),
      ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(ConstructMaterial(
                   "NaN density",
                   units::Density{
                       .value = std::numeric_limits<long double>::quiet_NaN()},
                   valid_composition)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(
                   ConstructMaterial("empty composition", valid_density, {})),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(ConstructMaterial(
                   "invalid Z", valid_density,
                   {{.atomic_number = 0U, .mass_fraction = 1.0L}})),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(ConstructMaterial(
                   "zero fraction", valid_density,
                   {{.atomic_number = 1U, .mass_fraction = 0.0L},
                    {.atomic_number = 8U, .mass_fraction = 1.0L}})),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(ConstructMaterial(
                   "negative fraction", valid_density,
                   {{.atomic_number = 1U, .mass_fraction = -0.25L},
                    {.atomic_number = 8U, .mass_fraction = 1.25L}})),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(ConstructMaterial(
          "infinite fraction", valid_density,
          {{.atomic_number = 1U,
            .mass_fraction = std::numeric_limits<long double>::infinity()},
           {.atomic_number = 8U, .mass_fraction = 1.0L}})),
      ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(ConstructMaterial(
          "NaN fraction", valid_density,
          {{.atomic_number = 1U,
            .mass_fraction = std::numeric_limits<long double>::quiet_NaN()},
           {.atomic_number = 8U, .mass_fraction = 1.0L}})),
      ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(ConstructMaterial(
                   "duplicate Z", valid_density,
                   {{.atomic_number = 6U, .mass_fraction = 0.5L},
                    {.atomic_number = 6U, .mass_fraction = 0.5L}})),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(ConstructMaterial(
                   "bad fraction sum", valid_density,
                   {{.atomic_number = 6U, .mass_fraction = 0.9L}})),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, SupportsVacuum) {
  materials::GGEMSMaterial const material{
      "Vacuum", units::Density{.value = 0.0L}, {}};

  EXPECT_EQ(material.GetName(), "Vacuum");
  EXPECT_TRUE(material.GetConstituents().empty());
  EXPECT_EQ(material.GetTotalAtomDensityPerCubicCentimeter(), 0.0L);
  EXPECT_EQ(material.GetElectronDensityPerCubicCentimeter(), 0.0L);
}
