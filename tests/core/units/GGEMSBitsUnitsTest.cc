#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSBitsUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::Bits;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

struct BitsConversionCase {
  std::string_view unit;
  std::uint64_t expected;
};

TEST(GGEMSBitsUnits, ConvertsEveryOfficialRuntimeToken) {
  constexpr std::array<BitsConversionCase, 9U> cases{{
      {.unit = "bit", .expected = 1ULL},
      {.unit = "kbit", .expected = 1'000ULL},
      {.unit = "Mbit", .expected = 1'000'000ULL},
      {.unit = "Gbit", .expected = 1'000'000'000ULL},
      {.unit = "Tbit", .expected = 1'000'000'000'000ULL},
      {.unit = "Kibit", .expected = 1'024ULL},
      {.unit = "Mibit", .expected = 1'048'576ULL},
      {.unit = "Gibit", .expected = 1'073'741'824ULL},
      {.unit = "Tibit", .expected = 1'099'511'627'776ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Bits>(1ULL, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected);
  }
}

TEST(GGEMSBitsUnits, RejectsNegativeValues) {
  auto const converted = MakeQuantity<Bits>(-1, "bit");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSBitsUnits, AutomaticDisplayUsesDecimalUnitsForIecValues) {
  using namespace ggems::units;

  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
  EXPECT_EQ(HumanReadable(1_Kibit, 3), "1.024 kbit");
}

TEST(GGEMSBitsUnits, LiteralsPreserveDecimalBinaryAndShortSuffixes) {
  using namespace ggems::units;

  EXPECT_EQ((1_kbit).value, 1'000ULL);
  EXPECT_EQ((1_Kibit).value, 1'024ULL);
  EXPECT_EQ((2_b).value, (2_bit).value);
}

} // namespace
