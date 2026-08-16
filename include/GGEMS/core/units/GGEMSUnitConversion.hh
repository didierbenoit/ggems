#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

enum class QuantityDomain : std::uint8_t { NonNegative, Signed };

enum class QuantityFormatPolicy : std::uint8_t {
  AutomaticScale,
  FixedUnit,
  DurationBreakdown
};

enum class UnitConversionError : std::uint8_t {
  UnsupportedUnit,
  NonFinite,
  NegativeValue,
  OutOfRange,
  InexactConversion
};

struct UnitScale {
  std::uint64_t numerator{1ULL};
  std::uint64_t denominator{1ULL};
  std::int16_t decimal_exponent{0};
  long double special_factor{0.0L};
};

consteval auto DecimalScale(std::int16_t exponent,
                            std::uint64_t numerator = 1ULL,
                            std::uint64_t denominator = 1ULL) -> UnitScale {
  return {.numerator = numerator,
          .denominator = denominator,
          .decimal_exponent = exponent,
          .special_factor = 0.0L};
};

consteval auto SpecialScale(long double factor) -> UnitScale {
  return {.numerator = 1ULL,
          .denominator = 1ULL,
          .decimal_exponent = 0,
          .special_factor = factor};
}

struct UnitDefinition {
  std::string_view symbol;
  UnitScale scale;
  std::string_view unicode_symbol{};
  bool automatic_display{true};
};

template <typename UniSet> struct UnitRegistry;

template <typename Tag> struct QuantityTraits;

namespace detail {

constexpr auto Pow10(std::int16_t exponent) noexcept -> long double {
  long double result{1.0L};
  if (exponent >= 0) {
    for (std::int16_t index = 0; index < exponent; ++index) {
      result *= 10.0L;
    }
  } else {
    for (std::int16_t index = 0; index > exponent; --index) {
      result /= 10.0L;
    }
  }
  return result;
}

constexpr auto ScaleFactor(UnitScale const &scale) noexcept -> long double {
  if (scale.special_factor != 0.0L) {
    return scale.special_factor;
  }
  return static_cast<long double>(scale.numerator) /
         static_cast<long double>(scale.denominator) *
         Pow10(scale.decimal_exponent);
}

constexpr auto IsFinite(long double value) noexcept -> bool {
  return value == value && value <= std::numeric_limits<long double>::max() &&
         value >= -std::numeric_limits<long double>::max();
}

constexpr auto ExactIntegralFactor(UnitScale const &scale,
                                   std::uint64_t &factor) noexcept -> bool {
  if (scale.special_factor != 0.0L || scale.denominator != 1ULL ||
      scale.decimal_exponent < 0) {
    return false;
  }
  factor = scale.numerator;
  for (std::int16_t index = 0; index < scale.decimal_exponent; ++index) {
    if (factor > std::numeric_limits<std::uint64_t>::max() / 10ULL) {
      return false;
    }
    factor *= 10ULL;
  }
  return true;
}

template <typename Representation>
constexpr auto ConvertCanonical(long double value)
    -> std::expected<Representation, UnitConversionError> {
  if (!IsFinite(value)) {
    return std::unexpected(UnitConversionError::OutOfRange);
  }

  if constexpr (std::floating_point<Representation>) {
    auto const converted = static_cast<Representation>(value);
    if (!IsFinite(static_cast<long double>(converted))) {
      return std::unexpected(UnitConversionError::OutOfRange);
    }
    return converted;
  } else {
    long double upper{1.0L};
    for (int digit = 0; digit < std::numeric_limits<Representation>::digits;
         ++digit) {
      upper *= 2.0L;
    }
    if constexpr (std::unsigned_integral<Representation>) {
      if (value < 0.0L) {
        return std::unexpected(UnitConversionError::NegativeValue);
      }
      if (value >= upper) {
        return std::unexpected(UnitConversionError::OutOfRange);
      }
    } else if (value < -upper || value >= upper) {
      return std::unexpected(UnitConversionError::OutOfRange);
    }
    auto converted = static_cast<Representation>(value);
    long double const remainder = value - static_cast<long double>(converted);
    if (remainder >= 0.5L) {
      if (converted == std::numeric_limits<Representation>::max()) {
        return std::unexpected(UnitConversionError::OutOfRange);
      }
      ++converted;
    } else if (remainder <= -0.5L) {
      if (converted == std::numeric_limits<Representation>::min()) {
        return std::unexpected(UnitConversionError::OutOfRange);
      }
      --converted;
    }
    return converted;
  }
}

template <typename Type>
concept ExactIntegral =
    std::integral<Type> && !std::same_as<std::remove_cv_t<Type>, bool> &&
    sizeof(Type) <= sizeof(std::uint64_t);

template <ExactIntegral Integer>
constexpr auto IntegralIsNegative(Integer value) noexcept -> bool {
  if constexpr (std::signed_integral<Integer>) {
    return value < 0;
  } else {
    return false;
  }
}

template <ExactIntegral Integer>
constexpr auto IntegralMagnitude(Integer value) noexcept -> std::uint64_t {
  using UnsignedInteger = std::make_unsigned_t<Integer>;
  auto magnitude = static_cast<UnsignedInteger>(value);
  if constexpr (std::signed_integral<Integer>) {
    if (value < 0) {
      magnitude = static_cast<UnsignedInteger>(UnsignedInteger{0} - magnitude);
    }
  }
  return static_cast<std::uint64_t>(magnitude);
}

template <ExactIntegral Representation>
constexpr auto ConvertIntegralMagnitude(std::uint64_t magnitude, bool negative)
    -> std::expected<Representation, UnitConversionError> {
  if (negative) {
    if constexpr (std::unsigned_integral<Representation>) {
      return std::unexpected(UnitConversionError::NegativeValue);
    } else {
      auto const negative_limit =
          static_cast<std::uint64_t>(
              std::numeric_limits<Representation>::max()) +
          1ULL;
      if (magnitude > negative_limit) {
        return std::unexpected(UnitConversionError::OutOfRange);
      }
      if (magnitude == negative_limit) {
        return std::numeric_limits<Representation>::min();
      }
      return static_cast<Representation>(
          -static_cast<Representation>(magnitude));
    }
  }
  if (magnitude >
      static_cast<std::uint64_t>(std::numeric_limits<Representation>::max())) {
    return std::unexpected(UnitConversionError::OutOfRange);
  }
  return static_cast<Representation>(magnitude);
}

} // namespace detail

template <typename UnitSet>
concept HasUnitRegistry = requires { UnitRegistry<UnitSet>::units; };

template <typename UnitSet> consteval auto ValidateUnitSet() -> bool {
  if constexpr (!HasUnitRegistry<UnitSet>) {
    return false;
  } else {
    auto const &units = UnitRegistry<UnitSet>::units;
    if (units.empty()) {
      return false;
    }
    for (std::size_t lhs_index = 0U; lhs_index < units.size(); ++lhs_index) {
      auto const &lhs = units[lhs_index];
      if (lhs.symbol.empty() || lhs.scale.numerator == 0ULL ||
          lhs.scale.denominator == 0ULL) {
        return false;
      }
      long double const factor = detail::ScaleFactor(lhs.scale);
      if (!detail::IsFinite(factor) || factor <= 0.0L) {
        return false;
      }
      for (std::size_t rhs_index = lhs_index + 1U; rhs_index < units.size();
           ++rhs_index) {
        if (lhs.symbol == units[rhs_index].symbol) {
          return false;
        }
      }
    }
    return true;
  }
}

template <typename UnitSet>
constexpr auto FindUnit(std::string_view symbol) noexcept
    -> UnitDefinition const * {
  static_assert(ValidateUnitSet<UnitSet>());
  for (auto const &unit : UnitRegistry<UnitSet>::units) {
    if (unit.symbol == symbol) {
      return &unit;
    }
  }
  return nullptr;
}

template <typename Tag, typename Representation>
consteval auto ValidateQuantityTraits() -> bool {
  if constexpr (!requires {
                  typename QuantityTraits<Tag>::unit_set;
                  {
                    QuantityTraits<Tag>::domain
                  } -> std::same_as<QuantityDomain const &>;
                  {
                    QuantityTraits<Tag>::format_policy
                  } -> std::same_as<QuantityFormatPolicy const &>;
                  {
                    QuantityTraits<Tag>::fixed_display_unit
                  } -> std::convertible_to<std::string_view>;
                  {
                    QuantityTraits<Tag>::default_precision
                  } -> std::convertible_to<std::int8_t>;
                } || !std::is_arithmetic_v<Representation>) {
    return false;
  } else {
    using Traits = QuantityTraits<Tag>;
    using UnitSet = typename Traits::unit_set;
    if constexpr (!ValidateUnitSet<UnitSet>()) {
      return false;
    } else {
      if (Traits::default_precision < 0 ||
          (Traits::domain == QuantityDomain::Signed &&
           std::unsigned_integral<Representation>)) {
        return false;
      }

      if (Traits::format_policy == QuantityFormatPolicy::FixedUnit) {
        return FindUnit<UnitSet>(Traits::fixed_display_unit) != nullptr;
      }

      bool has_automatic_unit{false};
      for (auto const &unit : UnitRegistry<UnitSet>::units) {
        has_automatic_unit = has_automatic_unit || unit.automatic_display;
      }

      if (!has_automatic_unit) {
        return false;
      }

      if (Traits::format_policy == QuantityFormatPolicy::DurationBreakdown) {
        if constexpr (!std::unsigned_integral<Representation>) {
          return false;
        } else {
          auto const *second = FindUnit<UnitSet>("s");
          auto const *millisecond = FindUnit<UnitSet>("ms");

          if (second == nullptr || millisecond == nullptr) {
            return false;
          }

          std::uint64_t second_factor{0ULL};
          std::uint64_t millisecond_factor{0ULL};
          return detail::ExactIntegralFactor(second->scale, second_factor) &&
                 detail::ExactIntegralFactor(millisecond->scale,
                                             millisecond_factor) &&
                 millisecond_factor <=
                     std::numeric_limits<std::uint64_t>::max() / 1'000ULL &&
                 second_factor == millisecond_factor * 1'000ULL;
        }
      }
      return true;
    }
  }
}

template <QuantityType TargetQuantity>
[[nodiscard]] constexpr auto MakeQuantity(long double value,
                                          std::string_view unit_symbol)
    -> std::expected<TargetQuantity, UnitConversionError> {
  using Traits = QuantityTraits<typename TargetQuantity::tag>;
  auto const *unit = FindUnit<typename Traits::unit_set>(unit_symbol);

  if (unit == nullptr) {
    return std::unexpected(UnitConversionError::UnsupportedUnit);
  }

  if (!detail::IsFinite(value)) {
    return std::unexpected(UnitConversionError::NonFinite);
  }

  if (Traits::domain == QuantityDomain::NonNegative && value < 0.0L) {
    return std::unexpected(UnitConversionError::NegativeValue);
  }

  auto converted =
      detail::ConvertCanonical<typename TargetQuantity::representation>(
          value * detail::ScaleFactor(unit->scale));

  if (!converted.has_value()) {
    return std::unexpected(converted.error());
  }

  return TargetQuantity{*converted};
}

template <QuantityType TargetQuantity, detail::ExactIntegral SourceInteger>
  requires detail::ExactIntegral<typename TargetQuantity::representation>
[[nodiscard]] constexpr auto MakeQuantity(SourceInteger value,
                                          std::string_view unit_symbol)
    -> std::expected<TargetQuantity, UnitConversionError> {
  using Traits = QuantityTraits<typename TargetQuantity::tag>;
  using Representation = typename TargetQuantity::representation;

  auto const *unit = FindUnit<typename Traits::unit_set>(unit_symbol);
  if (unit == nullptr) {
    return std::unexpected(UnitConversionError::UnsupportedUnit);
  }

  bool const negative = detail::IntegralIsNegative(value);
  if (Traits::domain == QuantityDomain::NonNegative && negative) {
    return std::unexpected(UnitConversionError::NegativeValue);
  }

  std::uint64_t factor{0ULL};
  if (!detail::ExactIntegralFactor(unit->scale, factor)) {
    return MakeQuantity<TargetQuantity>(static_cast<long double>(value),
                                        unit_symbol);
  }

  std::uint64_t const magnitude = detail::IntegralMagnitude(value);
  if (magnitude > std::numeric_limits<std::uint64_t>::max() / factor) {
    return std::unexpected(UnitConversionError::OutOfRange);
  }

  auto const converted = detail::ConvertIntegralMagnitude<Representation>(
      magnitude * factor, negative);
  if (!converted.has_value()) {
    return std::unexpected(converted.error());
  }

  return TargetQuantity{*converted};
}

template <QuantityType SourceQuantity>
[[nodiscard]] constexpr auto ConvertTo(SourceQuantity quantity,
                                       std::string_view unit_symbol)
    -> std::expected<long double, UnitConversionError> {
  using Traits = QuantityTraits<typename SourceQuantity::tag>;

  auto const *unit = FindUnit<typename Traits::unit_set>(unit_symbol);
  if (unit == nullptr) {
    return std::unexpected(UnitConversionError::UnsupportedUnit);
  }

  auto const canonical = static_cast<long double>(quantity.value);
  if (!detail::IsFinite(canonical)) {
    return std::unexpected(UnitConversionError::NonFinite);
  }

  long double const converted = canonical / detail::ScaleFactor(unit->scale);
  if (!detail::IsFinite(converted)) {
    return std::unexpected(UnitConversionError::OutOfRange);
  }

  return converted;
}

template <detail::ExactIntegral TargetRepresentation,
          QuantityType SourceQuantity>
  requires detail::ExactIntegral<typename SourceQuantity::representation>
[[nodiscard]] constexpr auto ConvertTo(SourceQuantity quantity,
                                       std::string_view unit_symbol)
    -> std::expected<TargetRepresentation, UnitConversionError> {
  using Traits = QuantityTraits<typename SourceQuantity::tag>;

  auto const *unit = FindUnit<typename Traits::unit_set>(unit_symbol);
  if (unit == nullptr) {
    return std::unexpected(UnitConversionError::UnsupportedUnit);
  }

  if (quantity.value == 0) {
    return TargetRepresentation{0};
  }

  std::uint64_t factor{0ULL};
  if (!detail::ExactIntegralFactor(unit->scale, factor)) {
    return std::unexpected(UnitConversionError::InexactConversion);
  }

  bool const negative = detail::IntegralIsNegative(quantity.value);
  std::uint64_t const magnitude = detail::IntegralMagnitude(quantity.value);
  if (magnitude % factor != 0ULL) {
    return std::unexpected(UnitConversionError::InexactConversion);
  }

  return detail::ConvertIntegralMagnitude<TargetRepresentation>(
      magnitude / factor, negative);
}

namespace detail {

template <QuantityType QuantityValue>
consteval auto MakeLiteralQuantity(unsigned long long value,
                                   std::string_view unit_symbol)
    -> QuantityValue {
  using Representation = typename QuantityValue::representation;
  using Traits = QuantityTraits<typename QuantityValue::tag>;

  auto const *unit = FindUnit<typename Traits::unit_set>(unit_symbol);
  if (unit == nullptr) {
    throw "Unsupported GGEMS quantity literal unit.";
  }

  if constexpr (std::integral<Representation>) {
    std::uint64_t factor{0ULL};
    if (detail::ExactIntegralFactor(unit->scale, factor)) {
      auto const maximum = static_cast<std::uint64_t>(
          std::numeric_limits<Representation>::max());
      if (factor == 0ULL || value > maximum / factor) {
        throw "GGEMS quantity literal is out of range.";
      }
      return QuantityValue{static_cast<Representation>(value * factor)};
    }
  }

  auto const result =
      MakeQuantity<QuantityValue>(static_cast<long double>(value), unit_symbol);
  if (!result.has_value()) {
    throw "Invalid GGEMS quantity literal.";
  }

  return *result;
}

template <QuantityType QuantityValue>
consteval auto MakeLiteralQuantity(long double value,
                                   std::string_view unit_symbol)
    -> QuantityValue {
  auto const result = MakeQuantity<QuantityValue>(value, unit_symbol);
  if (!result.has_value()) {
    throw "Invalid GGEMS quantity literal.";
  }
  return *result;
}

} // namespace detail
} // namespace ggems::units
