// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Declares GGEMS unit registries, quantity traits, and checked unit
 * conversion helpers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <concepts>
#include <cstdint>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>
/// \endcond

#include "GGEMS/units/GGEMSQuantity.hh"

namespace ggems::units {

/*!
 * \brief Selects the sign policy enforced by MakeQuantity().
 *
 * Direct construction, arithmetic, and ConvertTo() do not enforce this policy.
 */
enum class QuantityDomain : std::uint8_t {
  /*! \brief Quantity values must be nonnegative. */
  NonNegative,

  /*! \brief Quantity values may be negative or positive. */
  Signed,
};

/*!
 * \brief Selects the human-readable formatting strategy for a quantity family.
 */
enum class QuantityFormatPolicy : std::uint8_t {
  /*!
   * \brief Selects a display unit automatically from the registered unit
   * scales.
   */
  AutomaticScale,

  /*! \brief Always formats using the configured fixed display unit. */
  FixedUnit,

  /*!
   * \brief Formats long durations as hours, minutes, seconds, and milliseconds
   * when appropriate.
   */
  DurationBreakdown,
};

/*! \brief Reports failures produced by checked GGEMS unit conversions. */
enum class UnitConversionError : std::uint8_t {
  /*! \brief The requested unit symbol is not registered. */
  UnsupportedUnit,

  /*! \brief The supplied floating-point value is not finite. */
  NonFinite,

  /*! \brief A negative value was supplied for a nonnegative quantity. */
  NegativeValue,

  /*!
   * \brief The converted value cannot be represented by the destination type.
   */
  OutOfRange,

  /*!
   * \brief An exact integral conversion was requested but the value is not
   * exactly representable.
   */
  InexactConversion,
};

/*!
 * \brief Describes a unit scale relative to the canonical unit of its quantity
 * family.
 */
struct UnitScale {
  /*! \brief Rational scale numerator. */
  std::uint64_t numerator{1ULL};

  /*! \brief Rational scale denominator. */
  std::uint64_t denominator{1ULL};

  /*! \brief Base-10 exponent applied to the rational scale. */
  std::int16_t decimal_exponent{0};

  /*!
   * \brief Explicit scale factor used when a rational decimal scale is
   * unsuitable; zero selects the rational representation.
   */
  long double special_factor{0.0L};
};

/*!
 * \brief Creates an exact rational decimal unit scale.
 *
 * \param[in] exponent Base-10 exponent.
 * \param[in] numerator Rational scale numerator.
 * \param[in] denominator Rational scale denominator.
 * \return UnitScale describing the requested exact scale.
 *
 * \pre Use a nonzero numerator and denominator and an exponent yielding a
 * finite positive factor when the scale is used in a conversion. These
 * conditions are not validated here.
 */
consteval auto DecimalScale(std::int16_t exponent,
                            std::uint64_t numerator = 1ULL,
                            std::uint64_t denominator = 1ULL) -> UnitScale {
  return {
    .numerator = numerator,
    .denominator = denominator,
    .decimal_exponent = exponent,
    .special_factor = 0.0L,
  };
};

/*!
 * \brief Creates a unit scale represented by an explicit floating-point factor.
 *
 * \param[in] factor Scale factor relative to the canonical unit.
 * \return UnitScale using the supplied special factor.
 *
 * \pre Use a finite positive factor. Zero is reserved to select the rational
 * representation rather than a special zero scale.
 */
consteval auto SpecialScale(long double factor) -> UnitScale {
  return {
    .numerator = 1ULL,
    .denominator = 1ULL,
    .decimal_exponent = 0,
    .special_factor = factor,
  };
}

/*!
 * \brief Describes one accepted unit symbol and its conversion metadata.
 *
 * Symbol views borrow their character storage. Registry definitions and their
 * symbol storage must outlive lookups and formatting.
 */
struct UnitDefinition {
  /*! \brief ASCII unit symbol accepted by conversion functions. */
  std::string_view symbol;

  /*! \brief Scale relative to the canonical quantity representation. */
  UnitScale scale;

  /*! \brief Optional Unicode symbol used for formatted output. */
  std::string_view unicode_symbol{};

  /*! \brief Whether automatic formatting may select this unit. */
  bool automatic_display{true};
};

/*!
 * \brief Primary template for quantity-family unit registries.
 *
 * \tparam UniSet Unit-set marker type.
 */
template <typename UniSet> struct UnitRegistry;

/*!
 * \brief Primary template for quantity-family conversion and formatting traits.
 *
 * \tparam Tag Quantity-family tag type.
 */
template <typename Tag> struct QuantityTraits;

/// \cond
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
        static_cast<std::uint64_t>(std::numeric_limits<Representation>::max()) +
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
/// \endcond

template <typename UnitSet>
/*!
 * \brief Finds a unit definition by its ASCII symbol.
 *
 * \tparam UnitSet Unit-set marker type.
 * \param[in] symbol ASCII unit symbol to locate.
 * \return Pointer to the matching unit definition, or nullptr when the symbol
 * is unsupported.
 *
 * Matching is exact and case-sensitive, without trimming or Unicode aliases.
 * The returned pointer borrows the static registry entry.
 */
constexpr auto FindUnit(std::string_view symbol) noexcept
  -> UnitDefinition const * {
  for (auto const &unit : UnitRegistry<UnitSet>::units) {
    if (unit.symbol == symbol) {
      return &unit;
    }
  }
  return nullptr;
}

template <QuantityType TargetQuantity>
/*!
 * \brief Constructs a quantity from a floating-point value and unit symbol with
 * checked conversion.
 *
 * \tparam TargetQuantity GGEMS quantity type to construct.
 * \param[in] value Input value expressed in the supplied unit.
 * \param[in] unit_symbol ASCII symbol of the input unit.
 * \return Constructed quantity, or a UnitConversionError describing the failed
 * conversion.
 *
 * After checking the input sign and finiteness, conversion multiplies by the
 * working long-double scale. Integral destinations round to the nearest
 * canonical integer, with halfway values away from zero; floating destinations
 * use a representation cast. Nonfinite or unrepresentable canonical results
 * report OutOfRange.
 */
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

/*!
 * \brief Constructs an integral-representation quantity from an integral value
 * and unit symbol while preserving exact conversions when possible.
 *
 * \tparam TargetQuantity GGEMS quantity type to construct.
 * \tparam SourceInteger Integral source type.
 * \param[in] value Input integer expressed in the supplied unit.
 * \param[in] unit_symbol ASCII symbol of the input unit.
 * \return Constructed quantity, or a UnitConversionError describing the failed
 * conversion.
 *
 * For a representable positive integral scale, multiplication and range checks
 * use integer arithmetic. Other scales fall back to the long-double overload
 * and its rounding policy; this overload does not require an exact result in
 * that case.
 */
template <QuantityType TargetQuantity, detail::ExactIntegral SourceInteger>
  requires detail::ExactIntegral<typename TargetQuantity::representation>
[[nodiscard]] constexpr auto MakeQuantity(SourceInteger value,
                                          std::string_view unit_symbol)
  -> std::expected<TargetQuantity, UnitConversionError> {
  using Traits = QuantityTraits<typename TargetQuantity::tag>;
  using Representation = TargetQuantity::representation;

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
/*!
 * \brief Converts a quantity to a floating-point value in another unit.
 *
 * \tparam SourceQuantity GGEMS quantity type to convert.
 * \param[in] quantity Quantity to convert.
 * \param[in] unit_symbol ASCII symbol of the destination unit.
 * \return Converted value, or a UnitConversionError when the unit is
 * unsupported or the result is not finite.
 *
 * The stored canonical value is divided by the long-double scale. The
 * quantity-family sign policy is not revalidated, and conversion to long double
 * can lose integer precision.
 */
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

/*!
 * \brief Converts an integral quantity to an integral value in another unit
 * when the result is exact.
 *
 * \tparam TargetRepresentation Integral destination representation.
 * \tparam SourceQuantity GGEMS quantity type to convert.
 * \param[in] quantity Quantity to convert.
 * \param[in] unit_symbol ASCII symbol of the destination unit.
 * \return Exact converted value, or a UnitConversionError when the unit is
 * unsupported, the conversion is inexact, or the value is out of range.
 *
 * A supported unit converts zero exactly. For nonzero values, only scales
 * accepted as positive integral factors are handled, and the canonical
 * magnitude must be divisible by that factor. Other scales report
 * InexactConversion even when a mathematically exact quotient might exist.
 */
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

/// \cond
namespace detail {

template <QuantityType QuantityValue>
consteval auto MakeLiteralQuantity(unsigned long long value,
                                   std::string_view unit_symbol)
  -> QuantityValue {
  using Representation = QuantityValue::representation;
  using Input = std::conditional_t<std::floating_point<Representation>,
                                   long double, unsigned long long>;

  return MakeQuantity<QuantityValue>(static_cast<Input>(value), unit_symbol)
    .value();
}

template <QuantityType QuantityValue>
consteval auto MakeLiteralQuantity(long double value,
                                   std::string_view unit_symbol)
  -> QuantityValue {
  return MakeQuantity<QuantityValue>(value, unit_symbol).value();
}

} // namespace detail
/// \endcond

} // namespace ggems::units
