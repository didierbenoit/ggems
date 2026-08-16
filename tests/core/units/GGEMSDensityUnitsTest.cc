#include <array>
#include <cmath>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSDensityUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::Density;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

constexpr long double k_density_relative_tolerance =
    32.0L * std::numeric_limits<long double>::epsilon();

TEST(GGEMSDensityUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    long double value;
    std::string_view unit;
    long double expected_picograms_per_cubic_picometer;
  };

  constexpr std::array<Case, 2U> cases{{
      {.value = 1.0L,
       .unit = "pg/pm3",
       .expected_picograms_per_cubic_picometer = 1.0L},
      {.value = 1.0L,
       .unit = "g/cm3",
       .expected_picograms_per_cubic_picometer = 1.0e-18L},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted =
        MakeQuantity<Density>(test_case.value, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    long double const tolerance =
        std::abs(test_case.expected_picograms_per_cubic_picometer) *
        k_density_relative_tolerance;
    EXPECT_LE(std::abs(converted->value -
                       test_case.expected_picograms_per_cubic_picometer),
              tolerance);
  }
}

TEST(GGEMSDensityUnitsTest, EnforcesFiniteNonNegativeInput) {
  auto const negative = MakeQuantity<Density>(-1.0L, "pg/pm3");
  auto const infinity = MakeQuantity<Density>(
      std::numeric_limits<long double>::infinity(), "g/cm3");
  auto const not_a_number = MakeQuantity<Density>(
      std::numeric_limits<long double>::quiet_NaN(), "pg/pm3");

  ASSERT_FALSE(negative.has_value());
  ASSERT_FALSE(infinity.has_value());
  ASSERT_FALSE(not_a_number.has_value());
  EXPECT_EQ(negative.error(), UnitConversionError::NegativeValue);
  EXPECT_EQ(infinity.error(), UnitConversionError::NonFinite);
  EXPECT_EQ(not_a_number.error(), UnitConversionError::NonFinite);
}

TEST(GGEMSDensityUnitsTest, UsesFixedGramsPerCubicCentimeterDisplay) {
  Density const density{1.0e-18L};

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(density, 2), "1.00 g/cm3");
  }
  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(density, 2), "1.00 g/cm³");
  }
}

TEST(GGEMSDensityUnitsTest, LiteralsStoreCanonicalDensity) {
  using namespace ggems::units;

  constexpr Density canonical = 3_pg_pm3;
  constexpr Density laboratory_density = 2_g_cm3;

  EXPECT_EQ(canonical.value, 3.0L);
  EXPECT_LE(std::abs(laboratory_density.value - 2.0e-18L),
            2.0e-18L * k_density_relative_tolerance);
}

} // namespace
