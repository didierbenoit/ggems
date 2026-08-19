#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElement.hh"

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

TEST(GGEMSElementTest, StoresElementData) {
  constexpr materials::GGEMSElement iron{26U, "Fe", "iron", 55.845L};

  EXPECT_EQ(iron.GetAtomicNumber(), 26U);
  EXPECT_EQ(iron.GetSymbol(), "Fe");
  EXPECT_EQ(iron.GetName(), "iron");
  EXPECT_EQ(iron.GetMolarMass(), 55.845L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSElementTest, RejectsInvalidData) {
  auto const make_element =
      [](std::uint32_t atomic_number, std::string_view symbol,
         std::string_view name,
         long double molar_mass) -> materials::GGEMSElement {
    return materials::GGEMSElement{atomic_number, symbol, name, molar_mass};
  };

  EXPECT_THROW(static_cast<void>(make_element(0U, "H", "hydrogen", 1.0080L)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(make_element(119U, "Og", "oganesson", 294.213979L)),
      ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(make_element(1U, "", "hydrogen", 1.0080L)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(make_element(1U, "H", "", 1.0080L)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(make_element(1U, "H", "hydrogen", 0.0L)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(make_element(
          1U, "H", "hydrogen", std::numeric_limits<long double>::infinity())),
      ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(make_element(
          1U, "H", "hydrogen", std::numeric_limits<long double>::quiet_NaN())),
      ggems::core::GGEMSRecoverable);
}
