#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"

namespace {

using ggems::units::Length;
using ggems::units::MakeQuantity;
using ggems::units::PositionCoordinate;
using ggems::units::UnitConversionError;

struct SignedConversionCase {
  long double value;
  std::string_view unit;
  std::int64_t expected;
};

struct LengthConversionCase {
  long double value;
  std::string_view unit;
  std::uint64_t expected;
};

template <typename QuantityValue>
auto ExpectConversionError(long double value, std::string_view unit,
                           UnitConversionError expected_error) -> void {
  auto const conversion = MakeQuantity<QuantityValue>(value, unit);

  ASSERT_FALSE(conversion.has_value());
  EXPECT_EQ(conversion.error(), expected_error);
}

static_assert(!std::same_as<Length, PositionCoordinate>);
static_assert(!std::convertible_to<Length, PositionCoordinate>);
static_assert(!std::convertible_to<PositionCoordinate, Length>);

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSLengthUnits, ConvertsEveryRegisteredLengthToken) {
  constexpr std::array<LengthConversionCase, 8U> cases{{
      {.value = 2.0L, .unit = "pm", .expected = 2ULL},
      {.value = 2.0L, .unit = "nm", .expected = 2'000ULL},
      {.value = 2.0L, .unit = "um", .expected = 2'000'000ULL},
      {.value = 2.0L, .unit = "mm", .expected = 2'000'000'000ULL},
      {.value = 2.0L, .unit = "cm", .expected = 20'000'000'000ULL},
      {.value = 2.0L, .unit = "m", .expected = 2'000'000'000'000ULL},
      {.value = 2.0L, .unit = "km", .expected = 2'000'000'000'000'000ULL},
      {.value = 0.0L, .unit = "km", .expected = 0ULL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        MakeQuantity<Length>(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value()) << test_case.unit;
    EXPECT_EQ(conversion->value, test_case.expected) << test_case.unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSLengthUnits, ConvertsSignedPositionCoordinates) {
  constexpr std::array<SignedConversionCase, 4U> cases{{
      {.value = 2.0L, .unit = "pm", .expected = 2LL},
      {.value = -2.0L, .unit = "nm", .expected = -2'000LL},
      {.value = 2.0L, .unit = "cm", .expected = 20'000'000'000LL},
      {.value = -2.0L, .unit = "km", .expected = -2'000'000'000'000'000LL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        MakeQuantity<PositionCoordinate>(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value()) << test_case.unit;
    EXPECT_EQ(conversion->value, test_case.expected) << test_case.unit;
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSLengthUnits, RoundsHalfPicometersAwayFromZero) {
  constexpr std::array<SignedConversionCase, 8U> cases{{
      {.value = 0.49L, .unit = "pm", .expected = 0LL},
      {.value = 0.5L, .unit = "pm", .expected = 1LL},
      {.value = 1.49L, .unit = "pm", .expected = 1LL},
      {.value = 1.5L, .unit = "pm", .expected = 2LL},
      {.value = -0.49L, .unit = "pm", .expected = 0LL},
      {.value = -0.5L, .unit = "pm", .expected = -1LL},
      {.value = -1.49L, .unit = "pm", .expected = -1LL},
      {.value = -1.5L, .unit = "pm", .expected = -2LL},
  }};

  for (auto const &test_case : cases) {
    auto const conversion =
        MakeQuantity<PositionCoordinate>(test_case.value, test_case.unit);

    ASSERT_TRUE(conversion.has_value());
    EXPECT_EQ(conversion->value, test_case.expected);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSLengthUnits, EnforcesSignedCanonicalRange) {
  long double const upper_exclusive = std::ldexp(1.0L, 63);
  long double const lower_inclusive = -upper_exclusive;
  long double const below_lower = std::nextafter(
      lower_inclusive, -std::numeric_limits<long double>::infinity());

  auto const upper_accepted = MakeQuantity<PositionCoordinate>(
      std::numeric_limits<std::int64_t>::max(), "pm");
  auto const lower_accepted = MakeQuantity<PositionCoordinate>(
      std::numeric_limits<std::int64_t>::min(), "pm");

  ASSERT_TRUE(upper_accepted.has_value());
  EXPECT_EQ(upper_accepted->value, std::numeric_limits<std::int64_t>::max());
  ASSERT_TRUE(lower_accepted.has_value());
  EXPECT_EQ(lower_accepted->value, std::numeric_limits<std::int64_t>::min());

  ExpectConversionError<PositionCoordinate>(upper_exclusive, "pm",
                                            UnitConversionError::OutOfRange);
  ExpectConversionError<PositionCoordinate>(below_lower, "pm",
                                            UnitConversionError::OutOfRange);
  ExpectConversionError<PositionCoordinate>(
      std::numeric_limits<long double>::max(), "m",
      UnitConversionError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSLengthUnits, EnforcesNonNegativeCanonicalRange) {
  long double const upper_exclusive = std::ldexp(1.0L, 64);

  auto const accepted =
      MakeQuantity<Length>(std::numeric_limits<std::uint64_t>::max(), "pm");
  ASSERT_TRUE(accepted.has_value());
  EXPECT_EQ(accepted->value, std::numeric_limits<std::uint64_t>::max());

  auto const rounded_zero = MakeQuantity<Length>(0.49L, "pm");
  ASSERT_TRUE(rounded_zero.has_value());
  EXPECT_EQ(rounded_zero->value, 0ULL);

  auto const negative_zero = MakeQuantity<Length>(-0.0L, "pm");
  ASSERT_TRUE(negative_zero.has_value());
  EXPECT_EQ(negative_zero->value, 0ULL);

  ExpectConversionError<Length>(-0.49L, "pm",
                                UnitConversionError::NegativeValue);
  ExpectConversionError<Length>(upper_exclusive, "pm",
                                UnitConversionError::OutOfRange);
  ExpectConversionError<Length>(std::numeric_limits<long double>::max(), "km",
                                UnitConversionError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSLengthUnits, RejectsNonFiniteValuesAndUnknownTokens) {
  ExpectConversionError<Length>(std::numeric_limits<long double>::quiet_NaN(),
                                "pm", UnitConversionError::NonFinite);
  ExpectConversionError<Length>(std::numeric_limits<long double>::infinity(),
                                "pm", UnitConversionError::NonFinite);
  ExpectConversionError<PositionCoordinate>(
      -std::numeric_limits<long double>::infinity(), "pm",
      UnitConversionError::NonFinite);
  ExpectConversionError<Length>(1.0L, "KM",
                                UnitConversionError::UnsupportedUnit);
  ExpectConversionError<Length>(1.0L, "inch",
                                UnitConversionError::UnsupportedUnit);
}
