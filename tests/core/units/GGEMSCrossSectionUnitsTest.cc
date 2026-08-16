#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSCrossSectionUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::CrossSection;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

struct CrossSectionConversionCase {
  std::string_view unit;
  std::uint64_t expected;
};

TEST(GGEMSCrossSectionUnits, ConvertsEveryOfficialRuntimeToken) {
  constexpr std::array<CrossSectionConversionCase, 6U> cases{{
      {.unit = "pb", .expected = 1ULL},
      {.unit = "nb", .expected = 1'000ULL},
      {.unit = "ub", .expected = 1'000'000ULL},
      {.unit = "mb", .expected = 1'000'000'000ULL},
      {.unit = "barn", .expected = 1'000'000'000'000ULL},
      {.unit = "kbarn", .expected = 1'000'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<CrossSection>(1ULL, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected);
  }
}

TEST(GGEMSCrossSectionUnits, BarnHasExactlyOneTrillionPicobarns) {
  auto const barn = MakeQuantity<CrossSection>(1ULL, "barn");

  ASSERT_TRUE(barn.has_value());
  EXPECT_EQ(barn->value, 1'000'000'000'000ULL);
}

TEST(GGEMSCrossSectionUnits, RejectsNegativeValues) {
  auto const converted = MakeQuantity<CrossSection>(-1, "pb");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSCrossSectionUnits, UsesAsciiAndUnicodeMicrobarnDisplay) {
  using namespace ggems::units;

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(1_ub, 2), "1.00 ub");
  }

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(1_ub, 2), "1.00 µb");
  }
}

TEST(GGEMSCrossSectionUnits, ConvenienceBarnLiteralsMatchRuntimeUnits) {
  using namespace ggems::units;

  EXPECT_EQ((2_pbarn).value, (2_pb).value);
  EXPECT_EQ((2_nbarn).value, (2_nb).value);
  EXPECT_EQ((2_ubarn).value, (2_ub).value);
  EXPECT_EQ((2_mbarn).value, (2_mb).value);
}

} // namespace
