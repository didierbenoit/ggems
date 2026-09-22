#include <cmath>
#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace units = ggems::units;

using namespace ggems::units;

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
  EXPECT_EQ(constituents.front().isotope_profile,
            materials::GGEMSIsotopeProfile::Nist41Natural);

  // Independent oracle: NIST 4.1 natural carbon, AME2020 masses, CODATA 2022.
  ExpectRelativeNear(constituents.front().number_density_per_cubic_centimeter,
                     1.00279296879922875467374236961074477e+23L);
  ExpectRelativeNear(material.GetTotalAtomDensityPerCubicCentimeter(),
                     1.00279296879922875467374236961074477e+23L);
  ExpectRelativeNear(material.GetElectronDensityPerCubicCentimeter(),
                     6.01675781279537252804245421766446863e+23L);

  auto const isotopes = material.GetIsotopeConstituents();
  ASSERT_EQ(isotopes.size(), 2U);
  EXPECT_EQ(isotopes[0].isotope, (materials::GGEMSIsotope{6U, 12U, 0U}));
  EXPECT_EQ(isotopes[1].isotope, (materials::GGEMSIsotope{6U, 13U, 0U}));
  ExpectRelativeNear(isotopes[0].atom_fraction_in_element, 0.9893L);
  ExpectRelativeNear(isotopes[1].atom_fraction_in_element, 0.0107L);
  ExpectRelativeNear(isotopes[0].number_density_per_cubic_centimeter,
                     9.92063084033077006998733326255909802e+22L);
  ExpectRelativeNear(isotopes[1].number_density_per_cubic_centimeter,
                     1.07298847661517476750090433548349691e+21L);
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

  ExpectRelativeNear(constituents[0].number_density_per_cubic_centimeter,
                     1.86709286093585851670561142338812427e+23L);
  ExpectRelativeNear(constituents[1].number_density_per_cubic_centimeter,
                     3.52872933923961916000836309328804088e+22L);
  ExpectRelativeNear(material.GetTotalAtomDensityPerCubicCentimeter(),
                     2.21996579485982043270644773271692835e+23L);
  ExpectRelativeNear(material.GetElectronDensityPerCubicCentimeter(),
                     4.69007633232755384471230189801855697e+23L);

  // Isotope number densities aggregate to the elemental view.
  auto const isotopes = material.GetIsotopeConstituents();
  ASSERT_EQ(isotopes.size(), 5U);
  long double total_atom_density{0.0L};
  long double electron_density{0.0L};
  for (auto const &constituent : constituents) {
    long double isotope_sum{0.0L};
    for (auto const &isotope : isotopes) {
      if (isotope.isotope.GetAtomicNumber() == constituent.atomic_number) {
        isotope_sum += isotope.number_density_per_cubic_centimeter;
      }
    }
    ExpectRelativeNear(constituent.number_density_per_cubic_centimeter,
                       isotope_sum);
    total_atom_density += constituent.number_density_per_cubic_centimeter;
    electron_density += static_cast<long double>(constituent.atomic_number) *
                        constituent.number_density_per_cubic_centimeter;
  }
  ExpectRelativeNear(material.GetTotalAtomDensityPerCubicCentimeter(),
                     total_atom_density);
  ExpectRelativeNear(material.GetElectronDensityPerCubicCentimeter(),
                     electron_density);
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

  EXPECT_THROW(
    static_cast<void>(ConstructMaterial("", valid_density, valid_composition)),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(ConstructMaterial(
      "zero density", units::Density{.value = 0.0L}, valid_composition)),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(ConstructMaterial(
      "negative density", units::Density{.value = -1.0L}, valid_composition)),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(ConstructMaterial(
      "infinite density",
      units::Density{.value = std::numeric_limits<long double>::infinity()},
      valid_composition)),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(ConstructMaterial(
      "NaN density",
      units::Density{.value = std::numeric_limits<long double>::quiet_NaN()},
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
                 "Z above uranium", valid_density,
                 {{.atomic_number = 93U, .mass_fraction = 1.0L}})),
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
  EXPECT_TRUE(material.GetIsotopeConstituents().empty());
  EXPECT_EQ(material.GetTotalAtomDensityPerCubicCentimeter(), 0.0L);
  EXPECT_EQ(material.GetElectronDensityPerCubicCentimeter(), 0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, ResolvesNaturalHydrogenAndOxygen) {
  materials::GGEMSMaterial const hydrogen{
    "hydrogen", 1.0_g_cm3, {{.atomic_number = 1U, .mass_fraction = 1.0L}}};
  materials::GGEMSMaterial const oxygen{
    "oxygen", 1.0_g_cm3, {{.atomic_number = 8U, .mass_fraction = 1.0L}}};

  ExpectRelativeNear(hydrogen.GetTotalAtomDensityPerCubicCentimeter(),
                     5.97469715499474725345795655484199765e+23L);
  ASSERT_EQ(hydrogen.GetIsotopeConstituents().size(), 2U);
  ExpectRelativeNear(
    hydrogen.GetIsotopeConstituents()[1].number_density_per_cubic_centimeter,
    6.87090172824395934147665003806829730e+19L);

  ExpectRelativeNear(oxygen.GetTotalAtomDensityPerCubicCentimeter(),
                     3.76397796185559377067558729950724360e+22L);
  ExpectRelativeNear(oxygen.GetElectronDensityPerCubicCentimeter(),
                     3.01118236948447501654046983960579488e+23L);
  ASSERT_EQ(oxygen.GetIsotopeConstituents().size(), 3U);
  ExpectRelativeNear(
    oxygen.GetIsotopeConstituents()[2].number_density_per_cubic_centimeter,
    7.71615482180396722988495396398984939e+19L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, UsesReferenceIsotopeOnlyWithoutNaturalProfile) {
  materials::GGEMSMaterial const technetium{
    "technetium", 11.5_g_cm3, {{.atomic_number = 43U, .mass_fraction = 1.0L}}};

  ASSERT_EQ(technetium.GetConstituents().size(), 1U);
  EXPECT_EQ(technetium.GetConstituents().front().isotope_profile,
            materials::GGEMSIsotopeProfile::LegacyReferenceIsotope);

  auto const isotopes = technetium.GetIsotopeConstituents();
  ASSERT_EQ(isotopes.size(), 1U);
  EXPECT_EQ(isotopes.front().isotope, (materials::GGEMSIsotope{43U, 97U, 0U}));
  EXPECT_EQ(isotopes.front().atom_fraction_in_element, 1.0L);
  ExpectRelativeNear(technetium.GetTotalAtomDensityPerCubicCentimeter(),
                     7.14655035569708993188504263135629618e+22L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, DistinguishesNaturalAndEnrichedBoron) {
  materials::GGEMSMaterial const natural{
    "natural boron",
    2.34_g_cm3,
    {{.atomic_number = 5U, .mass_fraction = 1.0L}}};

  auto const enriched =
    [](materials::GGEMSFractionBasis basis) -> materials::GGEMSMaterial {
    return materials::GGEMSMaterial::FromIsotopicComposition(
      "enriched boron", 2.34_g_cm3,
      {
        {
          .mass_fraction = 1.0L,
          .isotopic_composition =
            materials::GGEMSIsotopicComposition{
              basis,
              {
                {.isotope = {5U, 10U, 0U}, .fraction = 0.9L},
                {.isotope = {5U, 11U, 0U}, .fraction = 0.1L},
              }},
        },
      });
  };

  auto const atom = enriched(materials::GGEMSFractionBasis::AtomFraction);
  auto const mass = enriched(materials::GGEMSFractionBasis::MassFraction);

  ExpectRelativeNear(
    natural.GetIsotopeConstituents()[0].number_density_per_cubic_centimeter,
    2.59389772755087224628642281888082702e+22L);
  ExpectRelativeNear(natural.GetTotalAtomDensityPerCubicCentimeter(),
                     1.30346619474918203330976021049287790e+23L);

  // Audit-11 M01 reference through the production isotope mass authority.
  ExpectRelativeNear(
    atom.GetIsotopeConstituents()[0].number_density_per_cubic_centimeter,
    1.25414447528572653701066421125976271e+23L);
  ExpectRelativeNear(
    mass.GetIsotopeConstituents()[0].number_density_per_cubic_centimeter,
    1.26662422843940630766304315092920560e+23L);
  ExpectRelativeNear(mass.GetTotalAtomDensityPerCubicCentimeter(),
                     1.39462330877304341567647924449795543e+23L);

  EXPECT_EQ(natural.GetConstituents().front().isotope_profile,
            materials::GGEMSIsotopeProfile::Nist41Natural);
  EXPECT_FALSE(atom.GetConstituents().front().isotope_profile.has_value());
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, BuildsExplicitIsotopicMaterial) {
  auto const material = materials::GGEMSMaterial::FromIsotopicComposition(
    "enriched boron carbon oxygen", 1.37_g_cm3,
    {
      {
        .mass_fraction = 0.65L,
        .isotopic_composition = materials::ResolveIsotopeProfile(
          materials::GGEMSIsotopeProfile::Nist41Natural, 8U),
      },
      {
        .mass_fraction = 0.25L,
        .isotopic_composition =
          materials::GGEMSIsotopicComposition{
            materials::GGEMSFractionBasis::AtomFraction,
            {
              {.isotope = {5U, 11U, 0U}, .fraction = 0.05L},
              {.isotope = {5U, 10U, 0U}, .fraction = 0.95L},
            }},
      },
      {
        .mass_fraction = 0.10L,
        .isotopic_composition = materials::ResolveIsotopeProfile(
          materials::GGEMSIsotopeProfile::Nist41Natural, 6U),
      },
    });

  auto const constituents = material.GetConstituents();
  ASSERT_EQ(constituents.size(), 3U);
  EXPECT_EQ(constituents[0].atomic_number, 5U);
  EXPECT_EQ(constituents[1].atomic_number, 6U);
  EXPECT_EQ(constituents[2].atomic_number, 8U);
  ExpectRelativeNear(constituents[0].number_density_per_cubic_centimeter,
                     2.04972013262376511713564420476101700e+22L);
  ExpectRelativeNear(constituents[1].number_density_per_cubic_centimeter,
                     6.86913183627471696951513523183360169e+21L);
  ExpectRelativeNear(constituents[2].number_density_per_cubic_centimeter,
                     3.35182237503240625278661049021120043e+22L);
  ExpectRelativeNear(material.GetTotalAtomDensityPerCubicCentimeter(),
                     6.08845569128364306687376821815557759e+22L);
  ExpectRelativeNear(material.GetElectronDensityPerCubicCentimeter(),
                     4.11846587651429057896801860845948494e+23L);

  for (auto const &constituent : constituents) {
    EXPECT_FALSE(constituent.isotope_profile.has_value());
  }

  auto const isotopes = material.GetIsotopeConstituents();
  ASSERT_EQ(isotopes.size(), 7U);
  EXPECT_EQ(isotopes[0].isotope, (materials::GGEMSIsotope{5U, 10U, 0U}));
  ExpectRelativeNear(isotopes[0].number_density_per_cubic_centimeter,
                     1.94723412599257686127886199452296615e+22L);
  EXPECT_EQ(isotopes[3].isotope, (materials::GGEMSIsotope{6U, 13U, 0U}));
  ExpectRelativeNear(isotopes[3].number_density_per_cubic_centimeter,
                     7.34997106481394715738119469806195380e+19L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, ElementalInputOrderDoesNotChangeResult) {
  materials::GGEMSMaterial const forward{
    "forward",
    1.03_g_cm3,
    {
      {.atomic_number = 1U, .mass_fraction = 0.107L},
      {.atomic_number = 6U, .mass_fraction = 0.145L},
      {.atomic_number = 8U, .mass_fraction = 0.712L},
      {.atomic_number = 20U, .mass_fraction = 0.036L},
    }};
  materials::GGEMSMaterial const reversed{
    "reversed",
    1.03_g_cm3,
    {
      {.atomic_number = 20U, .mass_fraction = 0.036L},
      {.atomic_number = 8U, .mass_fraction = 0.712L},
      {.atomic_number = 1U, .mass_fraction = 0.107L},
      {.atomic_number = 6U, .mass_fraction = 0.145L},
    }};

  auto const forward_isotopes = forward.GetIsotopeConstituents();
  auto const reversed_isotopes = reversed.GetIsotopeConstituents();
  ASSERT_EQ(forward_isotopes.size(), reversed_isotopes.size());
  for (std::size_t index = 0U; index < forward_isotopes.size(); ++index) {
    EXPECT_EQ(forward_isotopes[index].isotope,
              reversed_isotopes[index].isotope);
    EXPECT_EQ(forward_isotopes[index].number_density_per_cubic_centimeter,
              reversed_isotopes[index].number_density_per_cubic_centimeter);
  }
  EXPECT_EQ(forward.GetTotalAtomDensityPerCubicCentimeter(),
            reversed.GetTotalAtomDensityPerCubicCentimeter());
  EXPECT_EQ(forward.GetElectronDensityPerCubicCentimeter(),
            reversed.GetElectronDensityPerCubicCentimeter());
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialTest, IsotopicAuthoringHandlesVacuumAndInvalidInput) {
  auto const vacuum = materials::GGEMSMaterial::FromIsotopicComposition(
    "isotopic vacuum", units::Density{.value = 0.0L}, {});
  EXPECT_TRUE(vacuum.GetConstituents().empty());
  EXPECT_TRUE(vacuum.GetIsotopeConstituents().empty());
  EXPECT_EQ(vacuum.GetTotalAtomDensityPerCubicCentimeter(), 0.0L);

  auto const boron_share = []() -> materials::GGEMSElementalShare {
    return {
      .mass_fraction = 1.0L,
      .isotopic_composition = materials::ResolveIsotopeProfile(
        materials::GGEMSIsotopeProfile::Nist41Natural, 5U),
    };
  };

  EXPECT_THROW(
    static_cast<void>(materials::GGEMSMaterial::FromIsotopicComposition(
      "zero density matter", units::Density{.value = 0.0L}, {boron_share()})),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(materials::GGEMSMaterial::FromIsotopicComposition(
      "", 2.34_g_cm3, {boron_share()})),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(materials::GGEMSMaterial::FromIsotopicComposition(
      "negative density", units::Density{.value = -1.0L}, {boron_share()})),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
    static_cast<void>(materials::GGEMSMaterial::FromIsotopicComposition(
      "empty matter", 2.34_g_cm3, {})),
    ggems::core::GGEMSRecoverable);
}
