#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace units = ggems::units;

using namespace ggems::units;

using Basis = materials::GGEMSFractionBasis;

constexpr long double k_relative_budget{
    64.0L * std::numeric_limits<long double>::epsilon(),
};

constexpr long double k_fixture_molar_mass_constant{1.00000000105L};

constexpr materials::GGEMSIsotope k_hydrogen_1{1U, 1U, 0U};
constexpr materials::GGEMSIsotope k_hydrogen_2{1U, 2U, 0U};
constexpr materials::GGEMSIsotope k_boron_10{5U, 10U, 0U};
constexpr materials::GGEMSIsotope k_boron_11{5U, 11U, 0U};
constexpr materials::GGEMSIsotope k_carbon_12{6U, 12U, 0U};
constexpr materials::GGEMSIsotope k_oxygen_16{8U, 16U, 0U};
constexpr materials::GGEMSIsotope k_oxygen_17{8U, 17U, 0U};
constexpr materials::GGEMSIsotope k_oxygen_18{8U, 18U, 0U};

struct ExpectedIsotope {
  materials::GGEMSIsotope isotope;
  long double number_density;
  long double atom_fraction;
};

struct ExpectedElement {
  std::uint32_t atomic_number;
  long double number_density;
  long double electron_density;
  long double mass_fraction;
};

// =============================================================================
// =============================================================================

auto ExpectWithinBudget(long double actual, long double expected) -> void {
  EXPECT_LE(std::abs(actual - expected), k_relative_budget * std::abs(expected))
      << std::format("actual={:.21g} expected={:.21g}", actual, expected);
}

// =============================================================================
// =============================================================================

struct PrintedReference {
  long double value;
  int significant_digits;
};

// =============================================================================
// =============================================================================

auto ExpectMatchesPrintedReference(long double actual,
                                   PrintedReference reference) -> void {
  long double const rounding =
      0.5L * std::pow(10.0L, static_cast<long double>(
                                 1 - reference.significant_digits));

  EXPECT_LE(std::abs(actual - reference.value),
            (k_relative_budget + rounding) * std::abs(reference.value))
      << std::format("actual={:.21g} reference={:.21g}", actual,
                     reference.value);
}

// =============================================================================
// =============================================================================

auto MakeFixtureTable() -> materials::GGEMSResolvedIsotopeTable {
  auto const molar_mass = [](long double relative_atomic_mass) -> long double {
    return relative_atomic_mass * k_fixture_molar_mass_constant;
  };

  return materials::GGEMSResolvedIsotopeTable{{
      {
          .isotope = k_hydrogen_1,
          .molar_mass_grams_per_mole = molar_mass(1.007825031898L),
      },
      {
          .isotope = k_hydrogen_2,
          .molar_mass_grams_per_mole = molar_mass(2.014101777844L),
      },
      {
          .isotope = k_boron_10,
          .molar_mass_grams_per_mole = molar_mass(10.012936862L),
      },
      {
          .isotope = k_boron_11,
          .molar_mass_grams_per_mole = molar_mass(11.009305166L),
      },
      {
          .isotope = k_oxygen_16,
          .molar_mass_grams_per_mole = molar_mass(15.99491461926L),
      },
      {
          .isotope = k_oxygen_17,
          .molar_mass_grams_per_mole = molar_mass(16.99913175595L),
      },
      {
          .isotope = k_oxygen_18,
          .molar_mass_grams_per_mole = molar_mass(17.99915961214L),
      },
  }};
}

// =============================================================================
// =============================================================================

auto MakeShare(long double mass_fraction, Basis basis,
               std::vector<materials::GGEMSIsotopeFraction> fractions)
    -> materials::GGEMSElementalShare {
  return {
      .mass_fraction = mass_fraction,
      .isotopic_composition =
          materials::GGEMSIsotopicComposition{basis, std::move(fractions)},
  };
}

// =============================================================================
// =============================================================================

auto MakeBoron(Basis basis, long double boron_10_fraction,
               long double boron_11_fraction)
    -> materials::GGEMSMaterialComposition {
  return materials::GGEMSMaterialComposition{
      2.34_g_cm3,
      {
          MakeShare(1.0L, basis,
                    {
                        {.isotope = k_boron_10, .fraction = boron_10_fraction},
                        {.isotope = k_boron_11, .fraction = boron_11_fraction},
                    }),
      },
      MakeFixtureTable()};
}

// =============================================================================
// =============================================================================

auto MakeMultiElementShares() -> std::vector<materials::GGEMSElementalShare> {
  std::vector<materials::GGEMSElementalShare> shares;
  shares.push_back(MakeShare(0.10L, Basis::MassFraction,
                             {
                                 {.isotope = k_hydrogen_1, .fraction = 0.3L},
                                 {.isotope = k_hydrogen_2, .fraction = 0.7L},
                             }));
  shares.push_back(MakeShare(0.25L, Basis::AtomFraction,
                             {
                                 {.isotope = k_boron_10, .fraction = 0.95L},
                                 {.isotope = k_boron_11, .fraction = 0.05L},
                             }));
  shares.push_back(MakeShare(0.65L, Basis::AtomFraction,
                             {
                                 {.isotope = k_oxygen_16, .fraction = 0.99757L},
                                 {.isotope = k_oxygen_17, .fraction = 0.00038L},
                                 {.isotope = k_oxygen_18, .fraction = 0.00205L},
                             }));
  return shares;
}

// =============================================================================
// =============================================================================

auto ExpectComposition(materials::GGEMSMaterialComposition const &composition,
                       std::span<ExpectedIsotope const> isotopes,
                       std::span<ExpectedElement const> elements,
                       long double total_atom_density,
                       long double electron_density) -> void {
  auto const isotope_constituents = composition.GetIsotopeConstituents();
  ASSERT_EQ(isotope_constituents.size(), isotopes.size());

  for (std::size_t index = 0U; index < isotopes.size(); ++index) {
    SCOPED_TRACE(std::format("isotope index {}", index));
    EXPECT_EQ(isotope_constituents[index].isotope, isotopes[index].isotope);
    ExpectWithinBudget(
        isotope_constituents[index].number_density_per_cubic_centimeter,
        isotopes[index].number_density);
    ExpectWithinBudget(isotope_constituents[index].atom_fraction_in_element,
                       isotopes[index].atom_fraction);
  }

  auto const elemental_constituents = composition.GetElementalConstituents();
  ASSERT_EQ(elemental_constituents.size(), elements.size());

  for (std::size_t index = 0U; index < elements.size(); ++index) {
    SCOPED_TRACE(std::format("element index {}", index));
    EXPECT_EQ(elemental_constituents[index].atomic_number,
              elements[index].atomic_number);
    ExpectWithinBudget(
        elemental_constituents[index].number_density_per_cubic_centimeter,
        elements[index].number_density);
    ExpectWithinBudget(
        elemental_constituents[index].electron_density_per_cubic_centimeter,
        elements[index].electron_density);
    ExpectWithinBudget(elemental_constituents[index].mass_fraction,
                       elements[index].mass_fraction);
  }

  ExpectWithinBudget(composition.GetTotalAtomDensityPerCubicCentimeter(),
                     total_atom_density);
  ExpectWithinBudget(composition.GetElectronDensityPerCubicCentimeter(),
                     electron_density);
}

// =============================================================================
// =============================================================================

// Complete canonical row payloads, compared bit for bit.
auto IsotopeRow(materials::GGEMSIsotopeConstituent const &row)
    -> std::tuple<materials::GGEMSIsotope, long double, long double> {
  return {row.isotope, row.atom_fraction_in_element,
          row.number_density_per_cubic_centimeter};
}

auto ElementRow(materials::GGEMSDerivedElementalConstituent const &row)
    -> std::tuple<std::uint32_t, long double, long double, long double> {
  return {row.atomic_number, row.mass_fraction,
          row.number_density_per_cubic_centimeter,
          row.electron_density_per_cubic_centimeter};
}

// =============================================================================
// =============================================================================

auto ExpectIdentical(materials::GGEMSMaterialComposition const &actual,
                     materials::GGEMSMaterialComposition const &expected)
    -> void {
  EXPECT_TRUE(std::ranges::equal(actual.GetIsotopeConstituents(),
                                 expected.GetIsotopeConstituents(), {},
                                 IsotopeRow, IsotopeRow));
  EXPECT_TRUE(std::ranges::equal(actual.GetElementalConstituents(),
                                 expected.GetElementalConstituents(), {},
                                 ElementRow, ElementRow));
  EXPECT_EQ(actual.GetTotalAtomDensityPerCubicCentimeter(),
            expected.GetTotalAtomDensityPerCubicCentimeter());
  EXPECT_EQ(actual.GetElectronDensityPerCubicCentimeter(),
            expected.GetElectronDensityPerCubicCentimeter());
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, BoronAtomFractionsReproduceAudit11) {
  auto const composition = MakeBoron(Basis::AtomFraction, 0.9L, 0.1L);

  std::array<ExpectedIsotope, 2U> const isotopes{
      {
          {
              .isotope = k_boron_10,
              .number_density = 1.25414447528572653701066421125976271e+23L,
              .atom_fraction = 0.9L,
          },
          {
              .isotope = k_boron_11,
              .number_density = 1.39349386142858504112296023473306968e+22L,
              .atom_fraction = 0.1L,
          },
      },
  };
  std::array<ExpectedElement, 1U> const elements{
      {
          {
              .atomic_number = 5U,
              .number_density = 1.39349386142858504112296023473306968e+23L,
              .electron_density = 6.96746930714292520561480117366534840e+23L,
              .mass_fraction = 1.0L,
          },
      },
  };

  ExpectComposition(composition, isotopes, elements,
                    1.39349386142858504112296023473306968e+23L,
                    6.96746930714292520561480117366534840e+23L);

  // Audit-11 M01 printed reference (17 significant digits).
  ExpectMatchesPrintedReference(
      composition.GetIsotopeConstituents()[0]
          .number_density_per_cubic_centimeter,
      {.value = 1.2541444752857265e23L, .significant_digits = 17});
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, BoronMassFractionsDifferFromAtomFractions) {
  auto const composition = MakeBoron(Basis::MassFraction, 0.9L, 0.1L);

  std::array<ExpectedIsotope, 2U> const isotopes{
      {
          {
              .isotope = k_boron_10,
              .number_density = 1.26662422843940630766304315092920560e+23L,
              .atom_fraction = 9.08219603438115723243673710274510905e-1L,
          },
          {
              .isotope = k_boron_11,
              .number_density = 1.27999080333637108013436093568749834e+22L,
              .atom_fraction = 9.17803965618842767563262897254890954e-2L,
          },
      },
  };
  std::array<ExpectedElement, 1U> const elements{
      {
          {
              .atomic_number = 5U,
              .number_density = 1.39462330877304341567647924449795543e+23L,
              .electron_density = 6.97311654386521707838239622248977715e+23L,
              .mass_fraction = 1.0L,
          },
      },
  };

  ExpectComposition(composition, isotopes, elements,
                    1.39462330877304341567647924449795543e+23L,
                    6.97311654386521707838239622248977715e+23L);

  // Audit-11 M01 printed references (17 and 16 significant digits).
  ExpectMatchesPrintedReference(
      composition.GetIsotopeConstituents()[0]
          .number_density_per_cubic_centimeter,
      {.value = 1.2666242284394063e23L, .significant_digits = 17});
  ExpectMatchesPrintedReference(
      composition.GetIsotopeConstituents()[0].atom_fraction_in_element,
      {.value = 0.9082196034381157L, .significant_digits = 16});

  auto const atom_composition = MakeBoron(Basis::AtomFraction, 0.9L, 0.1L);
  for (std::size_t index = 0U; index < 2U; ++index) {
    long double const mass_basis = composition.GetIsotopeConstituents()[index]
                                       .number_density_per_cubic_centimeter;
    long double const atom_basis =
        atom_composition.GetIsotopeConstituents()[index]
            .number_density_per_cubic_centimeter;
    EXPECT_GT(std::abs(mass_basis - atom_basis), 1.0e-3L * atom_basis);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, NormalizedEnrichedBoronMatchesOracle) {
  auto const composition = MakeBoron(Basis::AtomFraction, 0.9000045L, 0.1L);

  std::array<ExpectedIsotope, 2U> const isotopes{
      {
          {
              .isotope = k_boron_10,
              .number_density = 1.25414515796049116376070298077628008e+23L,
              .atom_fraction = 9.00000449997975009112458993934527295e-1L,
          },
          {
              .isotope = k_boron_11,
              .number_density = 1.39348765251783870387392838677615509e+22L,
              .atom_fraction = 9.99995500020249908875410060654727054e-2L,
          },
      },
  };
  std::array<ExpectedElement, 1U> const elements{
      {
          {
              .atomic_number = 5U,
              .number_density = 1.39349392321227503414809581945389559e+23L,
              .electron_density = 6.96746961606137517074047909726947793e+23L,
              .mass_fraction = 1.0L,
          },
      },
  };

  ExpectComposition(composition, isotopes, elements,
                    1.39349392321227503414809581945389559e+23L,
                    6.96746961606137517074047909726947793e+23L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, EnrichedBoronDoesNotUseCatalogMolarMass) {
  materials::GGEMSMaterialComposition const composition{
      2.34_g_cm3,
      {
          MakeShare(1.0L, Basis::AtomFraction,
                    {{.isotope = k_boron_10, .fraction = 1.0L}}),
      },
      MakeFixtureTable()};

  long double const number_density = composition.GetElementalConstituents()
                                         .front()
                                         .number_density_per_cubic_centimeter;

  ExpectWithinBudget(number_density,
                     1.40736025382156256407004794547689511e+23L);

  // The natural representative catalog mass (10.81 g/mol) would give a value
  // about 7% lower.
  long double const catalog_route =
      6.02214076e23L * 2.34L /
      materials::RequireElementByAtomicNumber(5U).GetMolarMass();
  EXPECT_GT(std::abs(number_density - catalog_route), 5.0e-2L * catalog_route);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, MultiElementMixedBasesMatchOracle) {
  materials::GGEMSMaterialComposition const composition{
      1.37_g_cm3, MakeMultiElementShares(), MakeFixtureTable()};

  std::array<ExpectedIsotope, 7U> const isotopes{
      {
          {
              .isotope = k_hydrogen_1,
              .number_density = 2.45588249093186362910843801558872494e+22L,
              .atom_fraction = 4.61347497048610025164927068301914601e-1L,
          },
          {
              .isotope = k_hydrogen_2,
              .number_density = 2.86739878108747669152538761493934116e+22L,
              .atom_fraction = 5.38652502951389974835072931698085399e-1L,
          },
          {
              .isotope = k_boron_10,
              .number_density = 1.94723412599257686127886199452296615e+22L,
              .atom_fraction = 0.95L,
          },
          {
              .isotope = k_boron_11,
              .number_density = 1.02486006631188255856782210238050850e+21L,
              .atom_fraction = 0.05L,
          },
          {
              .isotope = k_oxygen_16,
              .number_density = 3.34367744666107750559233902671998721e+22L,
              .atom_fraction = 0.99757L,
          },
          {
              .isotope = k_oxygen_17,
              .number_density = 1.27369250251231437605891198628025616e+19L,
              .atom_fraction = 0.00038L,
          },
          {
              .isotope = k_oxygen_18,
              .number_density = 6.87123586881643281821255150493296088e+19L,
              .atom_fraction = 0.00205L,
          },
      },
  };
  std::array<ExpectedElement, 3U> const elements{
      {
          {
              .atomic_number = 1U,
              .number_density = 5.32328127201934032063382563052806610e+22L,
              .electron_density = 5.32328127201934032063382563052806610e+22L,
              .mass_fraction = 0.10L,
          },
          {
              .atomic_number = 5U,
              .number_density = 2.04972013262376511713564420476101700e+22L,
              .electron_density = 1.02486006631188255856782210238050850e+23L,
              .mass_fraction = 0.25L,
          },
          {
              .atomic_number = 8U,
              .number_density = 3.35182237503240625278661049021120043e+22L,
              .electron_density = 2.68145790002592500222928839216896034e+23L,
              .mass_fraction = 0.65L,
          },
      },
  };

  ExpectComposition(composition, isotopes, elements,
                    1.07248237796755116905560803255002835e+23L,
                    4.23864609353974159286049305760227545e+23L);

  // Aggregation identities checked from the isotope rows themselves.
  auto const isotope_rows = composition.GetIsotopeConstituents();
  auto const element_rows = composition.GetElementalConstituents();

  long double total_atom_density{0.0L};
  long double total_electron_density{0.0L};
  long double mass_fraction_sum{0.0L};
  for (auto const &element : element_rows) {
    long double isotope_sum{0.0L};
    for (auto const &isotope : isotope_rows) {
      if (isotope.isotope.GetAtomicNumber() == element.atomic_number) {
        isotope_sum += isotope.number_density_per_cubic_centimeter;
      }
    }

    ExpectWithinBudget(element.number_density_per_cubic_centimeter,
                       isotope_sum);
    total_atom_density += element.number_density_per_cubic_centimeter;
    total_electron_density += static_cast<long double>(element.atomic_number) *
                              element.number_density_per_cubic_centimeter;
    mass_fraction_sum += element.mass_fraction;
  }

  ExpectWithinBudget(composition.GetTotalAtomDensityPerCubicCentimeter(),
                     total_atom_density);
  ExpectWithinBudget(composition.GetElectronDensityPerCubicCentimeter(),
                     total_electron_density);
  ExpectWithinBudget(mass_fraction_sum, 1.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, InputPermutationDoesNotChangeResult) {
  materials::GGEMSMaterialComposition const reference{
      1.37_g_cm3, MakeMultiElementShares(), MakeFixtureTable()};

  std::vector<materials::GGEMSElementalShare> permuted;
  permuted.push_back(
      MakeShare(0.65L, Basis::AtomFraction,
                {
                    {.isotope = k_oxygen_18, .fraction = 0.00205L},
                    {.isotope = k_oxygen_16, .fraction = 0.99757L},
                    {.isotope = k_oxygen_17, .fraction = 0.00038L},
                }));
  permuted.push_back(MakeShare(0.10L, Basis::MassFraction,
                               {
                                   {.isotope = k_hydrogen_2, .fraction = 0.7L},
                                   {.isotope = k_hydrogen_1, .fraction = 0.3L},
                               }));
  permuted.push_back(MakeShare(0.25L, Basis::AtomFraction,
                               {
                                   {.isotope = k_boron_11, .fraction = 0.05L},
                                   {.isotope = k_boron_10, .fraction = 0.95L},
                               }));

  ExpectIdentical(materials::GGEMSMaterialComposition{1.37_g_cm3,
                                                      std::move(permuted),
                                                      MakeFixtureTable()},
                  reference);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, RemovesExplicitZeroShareWithoutMassData) {
  materials::GGEMSMaterialComposition const reference{
      1.37_g_cm3, MakeMultiElementShares(), MakeFixtureTable()};

  // C-12 is absent from the resolved table: a removed explicit zero share
  // requires no mass data.
  auto shares = MakeMultiElementShares();
  shares.push_back(MakeShare(0.0L, Basis::AtomFraction,
                             {{.isotope = k_carbon_12, .fraction = 1.0L}}));

  ExpectIdentical(materials::GGEMSMaterialComposition{1.37_g_cm3,
                                                      std::move(shares),
                                                      MakeFixtureTable()},
                  reference);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialCompositionTest, RejectsInvalidInput) {
  auto const expect_rejected =
      [](units::Density density,
         std::vector<materials::GGEMSElementalShare> shares) -> void {
    EXPECT_THROW(static_cast<void>(materials::GGEMSMaterialComposition{
                     density, std::move(shares), MakeFixtureTable()}),
                 ggems::core::GGEMSRecoverable);
  };

  auto const boron =
      [](long double mass_fraction) -> materials::GGEMSElementalShare {
    return MakeShare(mass_fraction, Basis::AtomFraction,
                     {
                         {.isotope = k_boron_10, .fraction = 0.9L},
                         {.isotope = k_boron_11, .fraction = 0.1L},
                     });
  };

  // Exact Vacuum (zero density) is not a matter composition.
  expect_rejected(units::Density{.value = 0.0L}, {boron(1.0L)});
  expect_rejected(units::Density{.value = -1.0L}, {boron(1.0L)});
  expect_rejected(
      units::Density{.value = std::numeric_limits<long double>::infinity()},
      {boron(1.0L)});
  expect_rejected(
      units::Density{.value = std::numeric_limits<long double>::quiet_NaN()},
      {boron(1.0L)});

  // Elemental share admission.
  expect_rejected(2.34_g_cm3, {});
  expect_rejected(2.34_g_cm3, {boron(0.0L)});
  expect_rejected(2.34_g_cm3, {boron(0.5L)});
  expect_rejected(2.34_g_cm3, {boron(-1.0L)});
  expect_rejected(2.34_g_cm3,
                  {boron(std::numeric_limits<long double>::quiet_NaN())});
  expect_rejected(2.34_g_cm3, {
                                  boron(0.5L),
                                  MakeShare(0.5L, Basis::MassFraction,
                                            {
                                                {
                                                    .isotope = k_boron_10,
                                                    .fraction = 1.0L,
                                                },
                                            }),
                              });

  // Missing exact isotope mass: no fallback to a neighbor or natural mass.
  EXPECT_THROW(static_cast<void>(materials::GGEMSMaterialComposition{
                   2.34_g_cm3,
                   {boron(1.0L)},
                   materials::GGEMSResolvedIsotopeTable{
                       {{.isotope = k_boron_10,
                         .molar_mass_grams_per_mole = 10.012936862L}}}}),
               ggems::core::GGEMSRecoverable);
}
