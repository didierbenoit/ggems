#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

TEST(GGEMSResolvedIsotopeTableTest, FindsExactKeysOnly) {
  materials::GGEMSResolvedIsotopeTable const table{{
    {.isotope = {73U, 180U, 1U}, .molar_mass_grams_per_mole = 179.95L},
    {.isotope = {5U, 10U, 0U}, .molar_mass_grams_per_mole = 10.01L},
  }};

  auto const resolved = table.GetResolvedIsotopes();
  ASSERT_EQ(resolved.size(), 2U);
  EXPECT_EQ(resolved[0].isotope, (materials::GGEMSIsotope{5U, 10U, 0U}));
  EXPECT_EQ(resolved[1].isotope, (materials::GGEMSIsotope{73U, 180U, 1U}));

  auto const *boron_10 = table.Find({5U, 10U, 0U});
  ASSERT_NE(boron_10, nullptr);
  EXPECT_EQ(boron_10->molar_mass_grams_per_mole, 10.01L);
  EXPECT_EQ(table.Require({73U, 180U, 1U}).molar_mass_grams_per_mole, 179.95L);

  // No neighbor, ground/isomer or natural-element substitution.
  EXPECT_EQ(table.Find({5U, 11U, 0U}), nullptr);
  EXPECT_EQ(table.Find({73U, 180U, 0U}), nullptr);
  EXPECT_THROW(static_cast<void>(table.Require({5U, 11U, 0U})),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(table.Require({73U, 180U, 0U})),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSResolvedIsotopeTableTest, PropertiesDoNotRedefineIdentity) {
  materials::GGEMSResolvedIsotope const first{
    .isotope = {5U, 10U, 0U},
    .molar_mass_grams_per_mole = 10.01L,
  };
  materials::GGEMSResolvedIsotope const second{
    .isotope = {5U, 10U, 0U},
    .molar_mass_grams_per_mole = 10.02L,
  };

  EXPECT_EQ(first.isotope, second.isotope);
  EXPECT_THROW(
    static_cast<void>(materials::GGEMSResolvedIsotopeTable{{first, second}}),
    ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSResolvedIsotopeTableTest, RejectsInvalidMolarMass) {
  auto const make_table =
    [](long double molar_mass) -> materials::GGEMSResolvedIsotopeTable {
    return materials::GGEMSResolvedIsotopeTable{
      {{.isotope = {1U, 1U, 0U}, .molar_mass_grams_per_mole = molar_mass}}};
  };

  EXPECT_THROW(static_cast<void>(make_table(0.0L)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(make_table(-1.0L)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(make_table(std::numeric_limits<long double>::infinity())),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(
                 make_table(std::numeric_limits<long double>::quiet_NaN())),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(
                 make_table(std::numeric_limits<long double>::denorm_min())),
               ggems::core::GGEMSRecoverable);
}
