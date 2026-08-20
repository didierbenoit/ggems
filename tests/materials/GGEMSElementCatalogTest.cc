#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElement.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

struct ExpectedElement {
  std::uint32_t atomic_number;
  std::string_view symbol;
  std::string_view name;
  long double molar_mass;
};

// =============================================================================
// =============================================================================

constexpr std::array<ExpectedElement, 12U> k_expected_elements{
    {{.atomic_number = 1U,
      .symbol = "H",
      .name = "Hydrogen",
      .molar_mass = 1.0080L},
     {.atomic_number = 6U,
      .symbol = "C",
      .name = "Carbon",
      .molar_mass = 12.011L},
     {.atomic_number = 13U,
      .symbol = "Al",
      .name = "Aluminum",
      .molar_mass = 26.9815384L},
     {.atomic_number = 15U,
      .symbol = "P",
      .name = "Phosphorus",
      .molar_mass = 30.973761998L},
     {.atomic_number = 23U,
      .symbol = "V",
      .name = "Vanadium",
      .molar_mass = 50.9415L},
     {.atomic_number = 26U,
      .symbol = "Fe",
      .name = "Iron",
      .molar_mass = 55.845L},
     {.atomic_number = 43U,
      .symbol = "Tc",
      .name = "Technetium",
      .molar_mass = 96.906360720L},
     {.atomic_number = 55U,
      .symbol = "Cs",
      .name = "Cesium",
      .molar_mass = 132.90545196L},
     {.atomic_number = 64U,
      .symbol = "Gd",
      .name = "Gadolinium",
      .molar_mass = 157.249L},
     {.atomic_number = 71U,
      .symbol = "Lu",
      .name = "Lutetium",
      .molar_mass = 174.96669L},
     {.atomic_number = 82U,
      .symbol = "Pb",
      .name = "Lead",
      .molar_mass = 207.2L},
     {.atomic_number = 92U,
      .symbol = "U",
      .name = "Uranium",
      .molar_mass = 238.02891L}}};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSElementCatalogTest, ProvidesAllElementsInAtomicNumberOrder) {
  auto const elements = materials::GetElements();

  ASSERT_EQ(elements.size(), 92U);

  std::set<std::string_view> symbols;
  std::set<std::string_view> names;

  for (std::size_t index = 0U; index < elements.size(); ++index) {
    auto const expected_atomic_number = static_cast<std::uint32_t>(index + 1U);
    auto const &element = elements[index];

    SCOPED_TRACE(expected_atomic_number);

    EXPECT_EQ(element.GetAtomicNumber(), expected_atomic_number);
    EXPECT_FALSE(element.GetSymbol().empty());
    EXPECT_FALSE(element.GetName().empty());
    EXPECT_TRUE(std::isfinite(element.GetMolarMass()));
    EXPECT_GT(element.GetMolarMass(), 0.0L);

    EXPECT_TRUE(symbols.insert(element.GetSymbol()).second);
    EXPECT_TRUE(names.insert(element.GetName()).second);

    EXPECT_EQ(materials::FindElementByAtomicNumber(expected_atomic_number),
              &element);
    EXPECT_EQ(materials::FindElementBySymbol(element.GetSymbol()), &element);
    EXPECT_EQ(materials::FindElementByName(element.GetName()), &element);

    EXPECT_EQ(&materials::RequireElementByAtomicNumber(expected_atomic_number),
              &element);
    EXPECT_EQ(&materials::RequireElementBySymbol(element.GetSymbol()),
              &element);
    EXPECT_EQ(&materials::RequireElementByName(element.GetName()), &element);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSElementCatalogTest, RejectsInvalidOrInexactLookup) {
  EXPECT_EQ(materials::FindElementByAtomicNumber(0U), nullptr);
  EXPECT_EQ(materials::FindElementByAtomicNumber(93U), nullptr);

  EXPECT_EQ(materials::FindElementBySymbol(""), nullptr);
  EXPECT_EQ(materials::FindElementBySymbol("h"), nullptr);
  EXPECT_EQ(materials::FindElementBySymbol(" H "), nullptr);
  EXPECT_EQ(materials::FindElementBySymbol("Np"), nullptr);

  EXPECT_EQ(materials::FindElementByName(""), nullptr);
  EXPECT_EQ(materials::FindElementByName("hydrogen"), nullptr);
  EXPECT_EQ(materials::FindElementByName(" Hydrogen "), nullptr);
  EXPECT_EQ(materials::FindElementByName("Neptunium"), nullptr);

  EXPECT_THROW(static_cast<void>(materials::RequireElementByAtomicNumber(0U)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(materials::RequireElementByAtomicNumber(93U)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(materials::RequireElementBySymbol("Np")),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(materials::RequireElementByName("Neptunium")),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSElementCatalogTest, PreservesRepresentativeScientificValues) {
  for (auto const &expected : k_expected_elements) {
    SCOPED_TRACE(expected.atomic_number);

    auto const &element =
        materials::RequireElementByAtomicNumber(expected.atomic_number);

    EXPECT_EQ(element.GetSymbol(), expected.symbol);
    EXPECT_EQ(element.GetName(), expected.name);
    EXPECT_EQ(element.GetMolarMass(), expected.molar_mass);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSElementCatalogTest, RejectsLegacyOrNonCanonicalElementNames) {
  EXPECT_EQ(materials::FindElementByName("Vandium"), nullptr);
  EXPECT_EQ(materials::FindElementByName("Phosphor"), nullptr);
  EXPECT_EQ(materials::FindElementByName("Aluminium"), nullptr);
  EXPECT_EQ(materials::FindElementByName("Caesium"), nullptr);
}
