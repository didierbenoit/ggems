#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::Energy;
using ggems::units::EnergyChange;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

TEST(GGEMSEnergyUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    std::string_view unit;
    std::uint64_t expected_milli_electron_volts;
  };

  constexpr std::array<Case, 6U> cases{{
      {.unit = "meV", .expected_milli_electron_volts = 1ULL},
      {.unit = "eV", .expected_milli_electron_volts = 1'000ULL},
      {.unit = "keV", .expected_milli_electron_volts = 1'000'000ULL},
      {.unit = "MeV", .expected_milli_electron_volts = 1'000'000'000ULL},
      {.unit = "GeV", .expected_milli_electron_volts = 1'000'000'000'000ULL},
      {.unit = "TeV",
       .expected_milli_electron_volts = 1'000'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Energy>(1, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected_milli_electron_volts);
  }
}

TEST(GGEMSEnergyUnitsTest, RejectsNegativeEnergy) {
  auto const converted = MakeQuantity<Energy>(-1, "eV");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSEnergyUnitsTest, EnergyChangeSupportsSignedValuesAndRounding) {
  auto const positive = MakeQuantity<EnergyChange>(1.5L, "eV");
  auto const negative = MakeQuantity<EnergyChange>(-1.5L, "eV");
  auto const positive_half = MakeQuantity<EnergyChange>(0.5L, "meV");
  auto const negative_half = MakeQuantity<EnergyChange>(-0.5L, "meV");

  ASSERT_TRUE(positive.has_value());
  ASSERT_TRUE(negative.has_value());
  ASSERT_TRUE(positive_half.has_value());
  ASSERT_TRUE(negative_half.has_value());
  EXPECT_EQ(positive->value, 1'500LL);
  EXPECT_EQ(negative->value, -1'500LL);
  EXPECT_EQ(positive_half->value, 1LL);
  EXPECT_EQ(negative_half->value, -1LL);
}

TEST(GGEMSEnergyUnitsTest, AutomaticallyDisplaysTheLargestApplicableUnit) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};

  EXPECT_EQ(HumanReadable(Energy{1'500'000ULL}, 2), "1.50 keV");
  EXPECT_EQ(HumanReadable(EnergyChange{-2'000'000LL}, 1), "-2.0 keV");
}

TEST(GGEMSEnergyUnitsTest, LiteralStoresMilliElectronVolts) {
  using namespace ggems::units;

  constexpr Energy energy = 2_keV;

  EXPECT_EQ(energy.value, 2'000'000ULL);
}

} // namespace
