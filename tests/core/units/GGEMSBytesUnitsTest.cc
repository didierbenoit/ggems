#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::Bytes;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

struct BytesConversionCase {
  std::string_view unit;
  std::uint64_t expected;
};

TEST(GGEMSBytesUnits, ConvertsEveryOfficialRuntimeToken) {
  constexpr std::array<BytesConversionCase, 9U> cases{{
      {.unit = "B", .expected = 1ULL},
      {.unit = "kB", .expected = 1'000ULL},
      {.unit = "MB", .expected = 1'000'000ULL},
      {.unit = "GB", .expected = 1'000'000'000ULL},
      {.unit = "TB", .expected = 1'000'000'000'000ULL},
      {.unit = "KiB", .expected = 1'024ULL},
      {.unit = "MiB", .expected = 1'048'576ULL},
      {.unit = "GiB", .expected = 1'073'741'824ULL},
      {.unit = "TiB", .expected = 1'099'511'627'776ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Bytes>(1ULL, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected);
  }
}

TEST(GGEMSBytesUnits, RejectsNegativeValues) {
  auto const converted = MakeQuantity<Bytes>(-1, "B");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSBytesUnits, AutomaticDisplayPrefersIecUnits) {
  using namespace ggems::units;

  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
  EXPECT_EQ(HumanReadable(1_kB, 2), "1000.00 B");
  EXPECT_EQ(HumanReadable(1_KiB, 2), "1.00 KiB");
}

TEST(GGEMSBytesUnits, LiteralStoresCanonicalBytes) {
  using namespace ggems::units;

  EXPECT_EQ((2_MiB).value, 2'097'152ULL);
}

} // namespace
