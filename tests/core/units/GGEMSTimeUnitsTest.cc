#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace {

using ggems::units::Duration;
using ggems::units::TimePoint;
using ggems::units::TryConvertTo;
using ggems::units::TryMakeQuantity;
using ggems::units::UnitConversionError;

struct ToPicosecondCase {
  long double value;
  std::string_view unit;
  std::uint64_t expected;
};

struct FromPicosecondCase {
  std::string_view unit;
  long double expected;
};

template <typename QuantityValue>
auto ExpectConversionError(long double value, std::string_view unit,
                           UnitConversionError expected_error) -> void {
  auto const conversion = TryMakeQuantity<QuantityValue>(value, unit);

  ASSERT_FALSE(conversion.has_value());
  EXPECT_EQ(conversion.error(), expected_error);
}

static_assert(!std::same_as<Duration, TimePoint>);
static_assert(!std::convertible_to<Duration, TimePoint>);
static_assert(!std::convertible_to<TimePoint, Duration>);

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSTimeUnits, ConvertsEveryRegisteredTimeTokenToPicoseconds) {
  constexpr std::array<ToPicosecondCase, 9U> cases{{
      {.value = 2.0L, .unit = "ps", .expected = 2ULL},
      {.value = 2.0L, .unit = "ns", .expected = 2'000ULL},
      {.value = 2.0L, .unit = "us", .expected = 2'000'000ULL},
      {.value = 2.0L, .unit = "µs", .expected = 2'000'000ULL},
      {.value = 2.0L, .unit = "μs", .expected = 2'000'000ULL},
      {.value = 2.0L, .unit = "ms", .expected = 2'000'000'000ULL},
      {.value = 2.0L, .unit = "s", .expected = 2'000'000'000'000ULL},
      {.value = 2.0L, .unit = "min", .expected = 120'000'000'000'000ULL},
      {.value = 2.0L, .unit = "h", .expected = 7'200'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    auto const duration =
        TryMakeQuantity<Duration>(test_case.value, test_case.unit);
    auto const time_point =
        TryMakeQuantity<TimePoint>(test_case.value, test_case.unit);

    ASSERT_TRUE(duration.has_value()) << test_case.unit;
    EXPECT_EQ(duration->value, test_case.expected) << test_case.unit;
    ASSERT_TRUE(time_point.has_value()) << test_case.unit;
    EXPECT_EQ(time_point->value, test_case.expected) << test_case.unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeUnits, RoundsToTheNearestPicosecond) {
  constexpr std::array<ToPicosecondCase, 5U> cases{{
      {.value = 0.0L, .unit = "ps", .expected = 0ULL},
      {.value = 0.49L, .unit = "ps", .expected = 0ULL},
      {.value = 0.5L, .unit = "ps", .expected = 1ULL},
      {.value = 1.49L, .unit = "ps", .expected = 1ULL},
      {.value = 1.5L, .unit = "ps", .expected = 2ULL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        TryMakeQuantity<Duration>(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value());
    EXPECT_EQ(conversion->value, test_case.expected);
  }

  auto const negative_zero = TryMakeQuantity<Duration>(-0.0L, "ps");
  ASSERT_TRUE(negative_zero.has_value());
  EXPECT_EQ(negative_zero->value, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeUnits, RejectsInvalidInputAndUnsupportedUnits) {
  ExpectConversionError<Duration>(std::numeric_limits<long double>::quiet_NaN(),
                                  "ps", UnitConversionError::NonFinite);
  ExpectConversionError<Duration>(std::numeric_limits<long double>::infinity(),
                                  "ps", UnitConversionError::NonFinite);
  ExpectConversionError<TimePoint>(
      -std::numeric_limits<long double>::infinity(), "ps",
      UnitConversionError::NonFinite);
  ExpectConversionError<Duration>(-1.0L, "ps",
                                  UnitConversionError::NegativeValue);
  ExpectConversionError<Duration>(1.0L, "fortnight",
                                  UnitConversionError::UnsupportedUnit);
  ExpectConversionError<Duration>(1.0L, "US",
                                  UnitConversionError::UnsupportedUnit);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeUnits, ProtectsTheUint64PicosecondRange) {
  long double const upper_exclusive = std::ldexp(1.0L, 64);

  auto const accepted = TryMakeQuantity<Duration>(
      std::numeric_limits<std::uint64_t>::max(), "ps");
  ASSERT_TRUE(accepted.has_value());
  EXPECT_EQ(accepted->value, std::numeric_limits<std::uint64_t>::max());

  ExpectConversionError<Duration>(upper_exclusive, "ps",
                                  UnitConversionError::OutOfRange);
  ExpectConversionError<Duration>(std::numeric_limits<long double>::max(), "h",
                                  UnitConversionError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeUnits, ConvertsPicosecondsToEveryRegisteredTimeToken) {
  constexpr std::uint64_t k_one_hour_ps{3'600'000'000'000'000ULL};
  constexpr std::array<FromPicosecondCase, 9U> cases{{
      {.unit = "ps", .expected = 3'600'000'000'000'000.0L},
      {.unit = "ns", .expected = 3'600'000'000'000.0L},
      {.unit = "us", .expected = 3'600'000'000.0L},
      {.unit = "µs", .expected = 3'600'000'000.0L},
      {.unit = "μs", .expected = 3'600'000'000.0L},
      {.unit = "ms", .expected = 3'600'000.0L},
      {.unit = "s", .expected = 3'600.0L},
      {.unit = "min", .expected = 60.0L},
      {.unit = "h", .expected = 1.0L},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        TryConvertTo(TimePoint{.value = k_one_hour_ps}, test_case.unit);

    ASSERT_TRUE(conversion.has_value()) << test_case.unit;
    EXPECT_EQ(*conversion, test_case.expected) << test_case.unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeUnits, RejectsUnsupportedOutputUnit) {
  auto const conversion = TryConvertTo(TimePoint{.value = 1ULL}, "fortnight");

  ASSERT_FALSE(conversion.has_value());
  EXPECT_EQ(conversion.error(), UnitConversionError::UnsupportedUnit);
}
