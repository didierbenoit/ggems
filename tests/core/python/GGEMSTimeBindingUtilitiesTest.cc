#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "ggems/GGEMSTimeBindingUtilities.hh"

namespace {

using ggems::python::detail::TimeConversionError;
using ggems::python::detail::TryConvertPicoSecondToTime;
using ggems::python::detail::TryConvertTimeToPicoSecond;

struct ToPicoSecondCase {
  double value;
  std::string_view unit;
  std::uint64_t expected;
};

struct FromPicoSecondCase {
  std::string_view unit;
  double expected;
};

// =============================================================================
// =============================================================================

auto ExpectConversionError(double value, std::string_view unit,
                           TimeConversionError expected_error) -> void {
  auto const conversion = TryConvertTimeToPicoSecond(value, unit);

  ASSERT_FALSE(conversion.has_value());
  EXPECT_EQ(conversion.error(), expected_error);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSTimeBindingUtilities, ConvertsEverySupportedTimeUnitToPicoSecond) {
  constexpr std::array<ToPicoSecondCase, 7U> cases{{
      {.value = 2.0, .unit = "ps", .expected = 2ULL},
      {.value = 2.0, .unit = "ns", .expected = 2'000ULL},
      {.value = 2.0, .unit = "us", .expected = 2'000'000ULL},
      {.value = 2.0, .unit = "ms", .expected = 2'000'000'000ULL},
      {.value = 2.0, .unit = "s", .expected = 2'000'000'000'000ULL},
      {.value = 2.0, .unit = "min", .expected = 120'000'000'000'000ULL},
      {.value = 2.0, .unit = "h", .expected = 7'200'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        TryConvertTimeToPicoSecond(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value()) << test_case.unit;
    EXPECT_EQ(*conversion, test_case.expected) << test_case.unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeBindingUtilities, RoundsToTheNearestPicoSecond) {
  constexpr std::array<ToPicoSecondCase, 5U> cases{{
      {.value = 0.0, .unit = "ps", .expected = 0ULL},
      {.value = 0.49, .unit = "ps", .expected = 0ULL},
      {.value = 0.5, .unit = "ps", .expected = 1ULL},
      {.value = 1.49, .unit = "ps", .expected = 1ULL},
      {.value = 1.5, .unit = "ps", .expected = 2ULL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        TryConvertTimeToPicoSecond(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value());
    EXPECT_EQ(*conversion, test_case.expected);
  }

  auto const negative_zero = TryConvertTimeToPicoSecond(-0.0, "ps");
  ASSERT_TRUE(negative_zero.has_value());
  EXPECT_EQ(*negative_zero, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeBindingUtilities, RejectsInvalidInputAndUnsupportedUnits) {
  ExpectConversionError(std::numeric_limits<double>::quiet_NaN(), "ps",
                        TimeConversionError::NonFinite);
  ExpectConversionError(std::numeric_limits<double>::infinity(), "ps",
                        TimeConversionError::NonFinite);
  ExpectConversionError(-std::numeric_limits<double>::infinity(), "ps",
                        TimeConversionError::NonFinite);
  ExpectConversionError(-1.0, "ps", TimeConversionError::Negative);
  ExpectConversionError(1.0, "fortnight", TimeConversionError::UnsupportedUnit);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeBindingUtilities, ProtectsTheUint64PicoSecondRange) {
  double const upper_exclusive = std::ldexp(1.0, 64);
  double const largest_double_below = std::nextafter(upper_exclusive, 0.0);

  auto const accepted = TryConvertTimeToPicoSecond(largest_double_below, "ps");
  ASSERT_TRUE(accepted.has_value());
  EXPECT_EQ(*accepted, static_cast<std::uint64_t>(largest_double_below));

  ExpectConversionError(upper_exclusive, "ps", TimeConversionError::OutOfRange);
  ExpectConversionError(std::numeric_limits<double>::max(), "h",
                        TimeConversionError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeBindingUtilities, ConvertsPicoSecondToEverySupportedUnit) {
  constexpr std::uint64_t k_one_hour_ps{3'600'000'000'000'000ULL};
  constexpr std::array<FromPicoSecondCase, 7U> cases{{
      {.unit = "ps", .expected = 3'600'000'000'000'000.0},
      {.unit = "ns", .expected = 3'600'000'000'000.0},
      {.unit = "us", .expected = 3'600'000'000.0},
      {.unit = "ms", .expected = 3'600'000.0},
      {.unit = "s", .expected = 3'600.0},
      {.unit = "min", .expected = 60.0},
      {.unit = "h", .expected = 1.0},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        TryConvertPicoSecondToTime(k_one_hour_ps, test_case.unit);

    ASSERT_TRUE(conversion.has_value()) << test_case.unit;
    EXPECT_DOUBLE_EQ(*conversion, test_case.expected) << test_case.unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTimeBindingUtilities, RejectsUnsupportedOutputUnit) {
  auto const conversion = TryConvertPicoSecondToTime(1ULL, "fortnight");

  ASSERT_FALSE(conversion.has_value());
  EXPECT_EQ(conversion.error(), TimeConversionError::UnsupportedUnit);
}
