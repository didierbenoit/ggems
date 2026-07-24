#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "ggems/GGEMSSourceBindingUtilities.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::python::detail::DistanceToPicometreError;
using ggems::python::detail::TryConvertDistanceToPicometre;
using ggems::python::detail::TryConvertPositiveDistanceToPicometre;

// =============================================================================
// =============================================================================

struct ConversionCase {
  double value;
  std::string_view unit;
  std::int64_t expected;
};

// =============================================================================
// =============================================================================

auto ExpectConversionError(double value, std::string_view unit,
                           DistanceToPicometreError expected_error) -> void {
  auto const conversion = TryConvertDistanceToPicometre(value, unit);

  ASSERT_FALSE(conversion.has_value());
  EXPECT_EQ(conversion.error(), expected_error);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSSourceBindingUtilities, ConvertsEverySupportedDistanceUnit) {
  constexpr std::array<ConversionCase, 6U> cases{{
      {.value = 2.0, .unit = "pm", .expected = 2LL},
      {.value = 2.0, .unit = "nm", .expected = 2'000LL},
      {.value = 2.0, .unit = "um", .expected = 2'000'000LL},
      {.value = 2.0, .unit = "mm", .expected = 2'000'000'000LL},
      {.value = 2.0, .unit = "cm", .expected = 20'000'000'000LL},
      {.value = 2.0, .unit = "m", .expected = 2'000'000'000'000LL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        TryConvertDistanceToPicometre(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value()) << test_case.unit;
    EXPECT_EQ(*conversion, test_case.expected) << test_case.unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceBindingUtilities, RoundsHalfPicometresAwayFromZero) {
  constexpr std::array<ConversionCase, 8U> cases{{
      {.value = 0.49, .unit = "pm", .expected = 0LL},
      {.value = 0.5, .unit = "pm", .expected = 1LL},
      {.value = 1.49, .unit = "pm", .expected = 1LL},
      {.value = 1.5, .unit = "pm", .expected = 2LL},
      {.value = -0.49, .unit = "pm", .expected = 0LL},
      {.value = -0.5, .unit = "pm", .expected = -1LL},
      {.value = -1.49, .unit = "pm", .expected = -1LL},
      {.value = -1.5, .unit = "pm", .expected = -2LL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        TryConvertDistanceToPicometre(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value());
    EXPECT_EQ(*conversion, test_case.expected);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceBindingUtilities, AcceptsOnlySafelyRepresentableBounds) {
  double const upper_exclusive = std::ldexp(1.0, 63);
  double const positive_safe = std::nextafter(upper_exclusive, 0.0);
  double const lower_inclusive = -upper_exclusive;
  double const negative_safe = std::nextafter(lower_inclusive, 0.0);

  auto const positive_conversion =
      TryConvertDistanceToPicometre(positive_safe, "pm");
  auto const lower_conversion =
      TryConvertDistanceToPicometre(lower_inclusive, "pm");
  auto const negative_conversion =
      TryConvertDistanceToPicometre(negative_safe, "pm");

  ASSERT_TRUE(positive_conversion.has_value());
  EXPECT_EQ(*positive_conversion, static_cast<std::int64_t>(positive_safe));

  ASSERT_TRUE(lower_conversion.has_value());
  EXPECT_EQ(*lower_conversion, std::numeric_limits<std::int64_t>::min());

  ASSERT_TRUE(negative_conversion.has_value());
  EXPECT_EQ(*negative_conversion, static_cast<std::int64_t>(negative_safe));

  ExpectConversionError(upper_exclusive, "pm",
                        DistanceToPicometreError::OutOfRange);
  ExpectConversionError(
      std::nextafter(lower_inclusive, -std::numeric_limits<double>::infinity()),
      "pm", DistanceToPicometreError::OutOfRange);
  ExpectConversionError(std::numeric_limits<double>::max(), "m",
                        DistanceToPicometreError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceBindingUtilities, RejectsInvalidValuesAndUnits) {
  ExpectConversionError(std::numeric_limits<double>::quiet_NaN(), "pm",
                        DistanceToPicometreError::NonFinite);
  ExpectConversionError(std::numeric_limits<double>::infinity(), "pm",
                        DistanceToPicometreError::NonFinite);
  ExpectConversionError(-std::numeric_limits<double>::infinity(), "pm",
                        DistanceToPicometreError::NonFinite);
  ExpectConversionError(1.0, "km", DistanceToPicometreError::UnsupportedUnit);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceBindingUtilities,
     ConvertsPositiveDimensionsInEverySupportedUnit) {
  constexpr std::array cases{
      std::pair{std::string_view{"pm"}, 2ULL},
      std::pair{std::string_view{"nm"}, 2'000ULL},
      std::pair{std::string_view{"um"}, 2'000'000ULL},
      std::pair{std::string_view{"mm"}, 2'000'000'000ULL},
      std::pair{std::string_view{"cm"}, 20'000'000'000ULL},
      std::pair{std::string_view{"m"}, 2'000'000'000'000ULL}};

  for (auto const &[unit, expected] : cases) {
    auto const result = TryConvertPositiveDistanceToPicometre(2.0, unit);
    ASSERT_TRUE(result.has_value()) << unit;
    EXPECT_EQ(*result, expected) << unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceBindingUtilities, RejectsNonPositiveDimensions) {
  for (double value : {-1.0, 0.0, 0.49}) {
    auto const result = TryConvertPositiveDistanceToPicometre(value, "pm");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), DistanceToPicometreError::NonPositive);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceBindingUtilities, ProtectsUint64DimensionRange) {
  double const upper_exclusive = std::ldexp(1.0, 64);
  double const largest_double_below = std::nextafter(upper_exclusive, 0.0);

  auto accepted =
      TryConvertPositiveDistanceToPicometre(largest_double_below, "pm");
  ASSERT_TRUE(accepted.has_value());

  auto rejected = TryConvertPositiveDistanceToPicometre(upper_exclusive, "pm");
  ASSERT_FALSE(rejected.has_value());
  EXPECT_EQ(rejected.error(), DistanceToPicometreError::OutOfRange);
}
