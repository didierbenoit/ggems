#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeMassAuthority.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

using Profile = materials::GGEMSIsotopeProfile;

constexpr long double k_relative_budget{
  64.0L * std::numeric_limits<long double>::epsilon(),
};

// Elements without a NIST 4.1 composition and their documented legacy
// reference isotope (element catalog).
constexpr std::array<materials::GGEMSIsotope, 8U> k_reference_isotopes{
  {
    {43U, 97U, 0U},
    {61U, 145U, 0U},
    {84U, 209U, 0U},
    {85U, 210U, 0U},
    {86U, 222U, 0U},
    {87U, 223U, 0U},
    {88U, 226U, 0U},
    {89U, 227U, 0U},
  },
};

struct ExpectedFraction {
  materials::GGEMSIsotope isotope;
  long double fraction;
};

// =============================================================================
// =============================================================================

auto ExpectProfile(std::uint32_t atomic_number,
                   std::vector<ExpectedFraction> const &expected) -> void {
  SCOPED_TRACE(atomic_number);

  auto const composition =
    materials::ResolveIsotopeProfile(Profile::Nist41Natural, atomic_number);

  EXPECT_EQ(composition.GetBasis(),
            materials::GGEMSFractionBasis::AtomFraction);
  EXPECT_EQ(composition.GetAtomicNumber(), atomic_number);

  auto const fractions = composition.GetFractions();
  ASSERT_EQ(fractions.size(), expected.size());

  for (std::size_t index = 0U; index < expected.size(); ++index) {
    EXPECT_EQ(fractions[index].isotope, expected[index].isotope);
    EXPECT_LE(std::abs(fractions[index].fraction - expected[index].fraction),
              k_relative_budget * expected[index].fraction);
  }
}

// =============================================================================
// =============================================================================

auto IsReferenceElement(std::uint32_t atomic_number) -> bool {
  return std::ranges::any_of(
    k_reference_isotopes,
    [atomic_number](materials::GGEMSIsotope const &isotope) -> bool {
      return isotope.GetAtomicNumber() == atomic_number;
    });
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ResolvesNist41ValidationFixtures) {
  // Audit-10 retained NIST 4.1 fixture values.
  ExpectProfile(1U, {
                      {.isotope = {1U, 1U, 0U}, .fraction = 0.999885L},
                      {.isotope = {1U, 2U, 0U}, .fraction = 0.000115L},
                    });
  ExpectProfile(5U, {
                      {.isotope = {5U, 10U, 0U}, .fraction = 0.199L},
                      {.isotope = {5U, 11U, 0U}, .fraction = 0.801L},
                    });
  ExpectProfile(6U, {
                      {.isotope = {6U, 12U, 0U}, .fraction = 0.9893L},
                      {.isotope = {6U, 13U, 0U}, .fraction = 0.0107L},
                    });
  ExpectProfile(8U, {
                      {.isotope = {8U, 16U, 0U}, .fraction = 0.99757L},
                      {.isotope = {8U, 17U, 0U}, .fraction = 0.00038L},
                      {.isotope = {8U, 18U, 0U}, .fraction = 0.00205L},
                    });
  ExpectProfile(64U, {
                       {.isotope = {64U, 152U, 0U}, .fraction = 0.0020L},
                       {.isotope = {64U, 154U, 0U}, .fraction = 0.0218L},
                       {.isotope = {64U, 155U, 0U}, .fraction = 0.1480L},
                       {.isotope = {64U, 156U, 0U}, .fraction = 0.2047L},
                       {.isotope = {64U, 157U, 0U}, .fraction = 0.1565L},
                       {.isotope = {64U, 158U, 0U}, .fraction = 0.2484L},
                       {.isotope = {64U, 160U, 0U}, .fraction = 0.2186L},
                     });
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ResolvesHeavyMultiIsotopeElements) {
  ExpectProfile(82U, {
                       {.isotope = {82U, 204U, 0U}, .fraction = 0.014L},
                       {.isotope = {82U, 206U, 0U}, .fraction = 0.241L},
                       {.isotope = {82U, 207U, 0U}, .fraction = 0.221L},
                       {.isotope = {82U, 208U, 0U}, .fraction = 0.524L},
                     });
  ExpectProfile(92U, {
                       {.isotope = {92U, 234U, 0U}, .fraction = 0.000054L},
                       {.isotope = {92U, 235U, 0U}, .fraction = 0.007204L},
                       {.isotope = {92U, 238U, 0U}, .fraction = 0.992742L},
                     });
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, MapsNaturalTantalum180ToItsIsomer) {
  // NIST 4.1 lists Ta-180; the naturally occurring state is NUBASE2020 180Tam.
  ExpectProfile(73U, {
                       {.isotope = {73U, 180U, 1U}, .fraction = 0.0001201L},
                       {.isotope = {73U, 181U, 0U}, .fraction = 0.9998799L},
                     });
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ProfilesAreCanonicalAndMassResolvable) {
  auto const &masses = materials::GetIsotopeMassAuthority();

  std::size_t natural_elements{0U};
  std::size_t natural_isotopes{0U};

  for (std::uint32_t atomic_number = 1U; atomic_number <= 92U;
       ++atomic_number) {
    SCOPED_TRACE(atomic_number);

    for (auto const profile :
         {Profile::Nist41Natural, Profile::LegacyReferenceIsotope}) {
      if (!materials::HasIsotopeProfile(profile, atomic_number)) {
        continue;
      }

      auto const composition =
        materials::ResolveIsotopeProfile(profile, atomic_number);
      auto const fractions = composition.GetFractions();

      long double fraction_sum{0.0L};
      for (std::size_t index = 0U; index < fractions.size(); ++index) {
        EXPECT_EQ(fractions[index].isotope.GetAtomicNumber(), atomic_number);
        if (index > 0U) {
          EXPECT_LT(fractions[index - 1U].isotope, fractions[index].isotope);
        }
        EXPECT_NE(masses.Find(fractions[index].isotope), nullptr);
        fraction_sum += fractions[index].fraction;
      }
      EXPECT_LE(std::abs(fraction_sum - 1.0L), k_relative_budget);

      if (profile == Profile::Nist41Natural) {
        ++natural_elements;
        natural_isotopes += fractions.size();
      }
    }
  }

  EXPECT_EQ(natural_elements, 84U);
  EXPECT_EQ(natural_isotopes, 288U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ClassifiesEveryMaterialElement) {
  for (std::uint32_t atomic_number = 1U; atomic_number <= 92U;
       ++atomic_number) {
    SCOPED_TRACE(atomic_number);

    bool const reference = IsReferenceElement(atomic_number);

    EXPECT_EQ(
      materials::HasIsotopeProfile(Profile::Nist41Natural, atomic_number),
      !reference);
    EXPECT_EQ(materials::HasIsotopeProfile(Profile::LegacyReferenceIsotope,
                                           atomic_number),
              reference);
    EXPECT_EQ(materials::SelectLegacyElementalIsotopeProfile(atomic_number),
              reference ? Profile::LegacyReferenceIsotope
                        : Profile::Nist41Natural);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ResolvesDocumentedReferenceIsotopesOnly) {
  for (auto const &isotope : k_reference_isotopes) {
    auto const atomic_number = isotope.GetAtomicNumber();
    SCOPED_TRACE(atomic_number);

    auto const composition = materials::ResolveIsotopeProfile(
      Profile::LegacyReferenceIsotope, atomic_number);

    ASSERT_EQ(composition.GetFractions().size(), 1U);
    EXPECT_EQ(composition.GetFractions().front().isotope, isotope);
    EXPECT_EQ(composition.GetFractions().front().fraction, 1.0L);

    // No natural composition can be requested for these elements.
    EXPECT_THROW(static_cast<void>(materials::ResolveIsotopeProfile(
                   Profile::Nist41Natural, atomic_number)),
                 ggems::core::GGEMSRecoverable);
  }

  // A natural element has no legacy reference isotope.
  EXPECT_THROW(static_cast<void>(materials::ResolveIsotopeProfile(
                 Profile::LegacyReferenceIsotope, 6U)),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, RejectsElementsOutsideMaterialsDomain) {
  for (std::uint32_t const atomic_number : {0U, 93U, 95U, 118U}) {
    SCOPED_TRACE(atomic_number);

    for (auto const profile :
         {Profile::Nist41Natural, Profile::LegacyReferenceIsotope}) {
      EXPECT_FALSE(materials::HasIsotopeProfile(profile, atomic_number));
      EXPECT_THROW(static_cast<void>(
                     materials::ResolveIsotopeProfile(profile, atomic_number)),
                   ggems::core::GGEMSRecoverable);
    }

    EXPECT_THROW(
      static_cast<void>(
        materials::SelectLegacyElementalIsotopeProfile(atomic_number)),
      ggems::core::GGEMSRecoverable);
  }
}
