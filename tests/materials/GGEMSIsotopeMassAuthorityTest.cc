#include <cmath>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeMassAuthority.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

constexpr long double k_relative_budget{
    64.0L * std::numeric_limits<long double>::epsilon(),
};

// Fixture convention of the Audit-10/11 oracle: M = A_r * M_u (CODATA 2022).
constexpr long double k_molar_mass_constant{1.00000000105L};

// =============================================================================
// =============================================================================

auto ExpectMolarMass(materials::GGEMSIsotope const &isotope,
                     long double expected) -> void {
  auto const *resolved = materials::GetIsotopeMassAuthority().Find(isotope);

  ASSERT_NE(resolved, nullptr);
  EXPECT_LE(std::abs(resolved->molar_mass_grams_per_mole - expected),
            k_relative_budget * expected);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeMassAuthorityTest, ResolvesAme2020MolarMasses) {
  ExpectMolarMass({1U, 1U, 0U}, 1.007825031898L * k_molar_mass_constant);
  ExpectMolarMass({1U, 2U, 0U}, 2.014101777844L * k_molar_mass_constant);
  ExpectMolarMass({5U, 10U, 0U}, 10.012936862L * k_molar_mass_constant);
  ExpectMolarMass({5U, 11U, 0U}, 11.009305166L * k_molar_mass_constant);
  ExpectMolarMass({6U, 12U, 0U}, 12.0L * k_molar_mass_constant);
  ExpectMolarMass({8U, 16U, 0U}, 15.99491461926L * k_molar_mass_constant);
  ExpectMolarMass({64U, 157U, 0U}, 156.923967424L * k_molar_mass_constant);
  ExpectMolarMass({82U, 208U, 0U}, 207.976652005L * k_molar_mass_constant);
  ExpectMolarMass({92U, 238U, 0U}, 238.050786936L * k_molar_mass_constant);
  ExpectMolarMass({43U, 97U, 0U}, 96.90636072L * k_molar_mass_constant);
  ExpectMolarMass({89U, 227U, 0U}, 227.027750594L * k_molar_mass_constant);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeMassAuthorityTest, ResolvesNaturalTantalumIsomerMass) {
  // (179.947467589 u + 75.3 keV / 931494.10242 keV/u) * M_u, Decimal oracle.
  ExpectMolarMass({73U, 180U, 1U}, 1.79947548615815830636932285612193216e+2L);
  ExpectMolarMass({73U, 181U, 0U}, 180.947998528L * k_molar_mass_constant);

  // The ground state is neither natural nor a reference isotope.
  EXPECT_EQ(materials::GetIsotopeMassAuthority().Find({73U, 180U, 0U}),
            nullptr);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeMassAuthorityTest, ContainsOnlyProfileIsotopes) {
  auto const resolved =
      materials::GetIsotopeMassAuthority().GetResolvedIsotopes();

  EXPECT_EQ(resolved.size(), 296U);

  for (auto const &entry : resolved) {
    auto const atomic_number = entry.isotope.GetAtomicNumber();
    SCOPED_TRACE(atomic_number);

    auto const profile =
        materials::SelectLegacyElementalIsotopeProfile(atomic_number);
    auto const composition =
        materials::ResolveIsotopeProfile(profile, atomic_number);

    bool found = false;
    for (auto const &fraction : composition.GetFractions()) {
      found = found || fraction.isotope == entry.isotope;
    }
    EXPECT_TRUE(found);
  }
}
