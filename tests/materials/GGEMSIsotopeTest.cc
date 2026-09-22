#include <algorithm>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, StoresExplicitKey) {
  constexpr materials::GGEMSIsotope tantalum_180m{73U, 180U, 1U};

  static_assert(tantalum_180m.GetAtomicNumber() == 73U);
  EXPECT_EQ(tantalum_180m.GetAtomicNumber(), 73U);
  EXPECT_EQ(tantalum_180m.GetMassNumber(), 180U);
  EXPECT_EQ(tantalum_180m.GetIsomerState(), 1U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, IdentityIsTheCompleteKey) {
  materials::GGEMSIsotope const boron_10{5U, 10U, 0U};

  EXPECT_EQ(boron_10, (materials::GGEMSIsotope{5U, 10U, 0U}));
  EXPECT_NE(boron_10, (materials::GGEMSIsotope{6U, 10U, 0U}));
  EXPECT_NE(boron_10, (materials::GGEMSIsotope{5U, 11U, 0U}));
  EXPECT_NE(boron_10, (materials::GGEMSIsotope{5U, 10U, 1U}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, OrdersLexicographicallyByAtomicMassAndIsomerKeys) {
  std::vector<materials::GGEMSIsotope> isotopes{
    {73U, 181U, 0U}, {5U, 11U, 0U},   {73U, 180U, 1U}, {1U, 2U, 0U},
    {8U, 16U, 0U},   {73U, 180U, 0U}, {5U, 10U, 0U},   {1U, 1U, 0U},
  };

  std::ranges::sort(isotopes);

  std::vector<materials::GGEMSIsotope> const expected{
    {1U, 1U, 0U},  {1U, 2U, 0U},    {5U, 10U, 0U},   {5U, 11U, 0U},
    {8U, 16U, 0U}, {73U, 180U, 0U}, {73U, 180U, 1U}, {73U, 181U, 0U},
  };

  EXPECT_EQ(isotopes, expected);

  // Z dominates A, and A dominates M.
  EXPECT_LT((materials::GGEMSIsotope{5U, 20U, 9U}),
            (materials::GGEMSIsotope{6U, 11U, 0U}));
  EXPECT_LT((materials::GGEMSIsotope{73U, 180U, 1U}),
            (materials::GGEMSIsotope{73U, 181U, 0U}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, RejectsInvalidStructure) {
  auto const make_isotope =
    [](std::uint32_t atomic_number, std::uint32_t mass_number,
       std::uint32_t isomer_state) -> materials::GGEMSIsotope {
    return materials::GGEMSIsotope{atomic_number, mass_number, isomer_state};
  };

  EXPECT_THROW(static_cast<void>(make_isotope(0U, 1U, 0U)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(make_isotope(119U, 300U, 0U)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(make_isotope(5U, 4U, 0U)),
               ggems::core::GGEMSRecoverable);

  // Structural validity only: acceptance does not claim that an evaluated
  // mass/state authority contains these keys.
  EXPECT_NO_THROW(static_cast<void>(make_isotope(1U, 1U, 0U)));
  EXPECT_NO_THROW(static_cast<void>(make_isotope(92U, 238U, 0U)));
  EXPECT_NO_THROW(static_cast<void>(make_isotope(5U, 30U, 0U)));
  EXPECT_NO_THROW(static_cast<void>(make_isotope(73U, 180U, 2U)));
}
