#include <array>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSSpeedUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::MakeSpeed;
using ggems::units::Speed;
using ggems::units::UnitConversionError;

TEST(GGEMSSpeedUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    std::string_view unit;
    long double expected_picometers_per_picosecond;
  };

  constexpr std::array<Case, 2U> cases{{
      {.unit = "pm/ps", .expected_picometers_per_picosecond = 1.0L},
      {.unit = "m/s", .expected_picometers_per_picosecond = 1.0L},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Speed>(1.0L, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected_picometers_per_picosecond);
  }
}

TEST(GGEMSSpeedUnitsTest, RejectsNegativeSpeed) {
  auto const converted = MakeQuantity<Speed>(-1.0L, "m/s");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSSpeedUnitsTest, UsesFixedMetersPerSecondDisplay) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};

  EXPECT_EQ(HumanReadable(Speed{12.5L}, 2), "12.50 m/s");
}

TEST(GGEMSSpeedUnitsTest, LiteralsUseEquivalentCanonicalScales) {
  using namespace ggems::units;

  constexpr Speed picometer_per_picosecond = 2_pm_ps;
  constexpr Speed meter_per_second = 3_m_s;

  EXPECT_EQ(picometer_per_picosecond.value, 2.0L);
  EXPECT_EQ(meter_per_second.value, 3.0L);
}

TEST(GGEMSSpeedUnitsTest, MakeSpeedComputesOneMeterPerSecond) {
  auto const speed = MakeSpeed(ggems::units::Length{1'000'000'000'000ULL},
                               ggems::units::Time{1'000'000'000'000ULL});

  EXPECT_EQ(speed.value, 1.0L);
}

} // namespace
