#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSMassUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::Mass;
using ggems::units::UnitConversionError;

TEST(GGEMSMassUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    std::string_view unit;
    std::uint64_t expected_picograms;
  };

  constexpr std::array<Case, 6U> cases{{
      {.unit = "pg", .expected_picograms = 1ULL},
      {.unit = "ng", .expected_picograms = 1'000ULL},
      {.unit = "ug", .expected_picograms = 1'000'000ULL},
      {.unit = "mg", .expected_picograms = 1'000'000'000ULL},
      {.unit = "g", .expected_picograms = 1'000'000'000'000ULL},
      {.unit = "kg", .expected_picograms = 1'000'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Mass>(1, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected_picograms);
  }
}

TEST(GGEMSMassUnitsTest, RejectsNegativeMass) {
  auto const converted = MakeQuantity<Mass>(-1, "pg");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSMassUnitsTest, PresentsMicrogramsInAsciiAndUnicode) {
  Mass const mass{2'000'000ULL};

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(mass, 2), "2.00 ug");
  }
  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(mass, 2), "2.00 µg");
  }
}

TEST(GGEMSMassUnitsTest, LiteralStoresPicograms) {
  using namespace ggems::units;

  constexpr Mass mass = 3_mg;

  EXPECT_EQ(mass.value, 3'000'000'000ULL);
}

} // namespace
