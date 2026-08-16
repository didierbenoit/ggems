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
 * \brief Declares human-readable formatting support for GGEMS quantities.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <string_view>
/// \endcond

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

/// \cond
namespace detail {

[[nodiscard]] inline auto SelectUnitSymbol(UnitDefinition const &unit) noexcept
    -> std::string_view {
  if (ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
      ggems::core::Encoding::Ascii) {
    return unit.symbol;
  }
  return unit.unicode_symbol.empty() ? unit.symbol : unit.unicode_symbol;
}

template <typename QuantityType>
auto FormatScaled(QuantityType const &quantity, UnitDefinition const &unit,
                  std::int8_t precision, std::int8_t width) -> std::string {
  long double const scaled =
      static_cast<long double>(quantity.value) / ScaleFactor(unit.scale);

  std::string_view const selected_symbol = SelectUnitSymbol(unit);
  std::string format;
  if (width < 0) {
    format = std::format("{{:.{}f}} {}", precision, selected_symbol);
  } else {
    format = std::format("{{:{}.{}f}} {}", width, precision, selected_symbol);
  }
  return std::vformat(format, std::make_format_args(scaled));
}

} // namespace detail
/// \endcond

template <QuantityType QuantityValue>
/*!
 * \brief Formats a GGEMS quantity using its configured display policy.
 *
 * \tparam QuantityValue GGEMS quantity type to format.
 * \param[in] quantity Quantity to format.
 * \param[in] precision Number of digits after the decimal point.
 * \param[in] width Optional numeric field width; negative selects the default width.
 * \return Human-readable quantity string including the selected unit symbol.
 */
auto HumanReadable(
    QuantityValue const &quantity,
    std::int8_t precision =
        QuantityTraits<typename QuantityValue::tag>::default_precision,
    std::int8_t width = -1) -> std::string {
  using Traits = QuantityTraits<typename QuantityValue::tag>;
  using UnitSet = typename Traits::unit_set;
  if constexpr (Traits::format_policy == QuantityFormatPolicy::FixedUnit) {
    return detail::FormatScaled(quantity,
                                *FindUnit<UnitSet>(Traits::fixed_display_unit),
                                precision, width);
  } else {
    if constexpr (Traits::format_policy ==
                  QuantityFormatPolicy::DurationBreakdown) {
      auto const second_factor = static_cast<std::uint64_t>(
          detail::ScaleFactor(FindUnit<UnitSet>("s")->scale));
      if (quantity.value >= 60ULL * second_factor) {
        auto const millisecond_factor = static_cast<std::uint64_t>(
            detail::ScaleFactor(FindUnit<UnitSet>("ms")->scale));
        auto const total_seconds = quantity.value / second_factor;
        auto const remainder = quantity.value % second_factor;
        auto const hours = total_seconds / 3'600ULL;
        auto const minutes = total_seconds % 3'600ULL / 60ULL;
        auto const seconds = total_seconds % 60ULL;
        auto const milliseconds = remainder / millisecond_factor;
        if (hours > 0ULL) {
          return std::format("{} h {} min {} s {} ms", hours, minutes, seconds,
                             milliseconds);
        }
        return std::format("{} min {} s {} ms", minutes, seconds, milliseconds);
      }
    }

    long double const magnitude =
        std::abs(static_cast<long double>(quantity.value));

    UnitDefinition const *selected{nullptr};
    long double selected_factor{-1.0L};
    for (auto const &unit : UnitRegistry<UnitSet>::units) {
      long double const factor = detail::ScaleFactor(unit.scale);
      if (unit.automatic_display && magnitude >= factor &&
          factor > selected_factor) {
        selected = &unit;
        selected_factor = factor;
      }
    }

    if (selected == nullptr) {
      long double smallest_factor = std::numeric_limits<long double>::max();
      for (auto const &unit : UnitRegistry<UnitSet>::units) {
        long double const factor = detail::ScaleFactor(unit.scale);
        if (unit.automatic_display && factor < smallest_factor) {
          selected = &unit;
          smallest_factor = factor;
        }
      }
    }
    return detail::FormatScaled(quantity, *selected, precision, width);
  }
}

} // namespace ggems::units

namespace std {
template <typename Tag, typename Representation>
/*!
 * \brief Integrates GGEMS quantities with std::format by delegating to HumanReadable.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Underlying quantity representation type.
 */
struct formatter<ggems::units::Quantity<Tag, Representation>>
    : formatter<string> {
/*!
 * \brief Formats a GGEMS quantity into a standard formatting context.
 *
 * \tparam FormatContext Standard formatting context type.
 * \param[in] quantity Quantity to format.
 * \param[in,out] context Destination formatting context.
 * \return Iterator to the end of the formatted output.
 */
  template <typename FormatContext>
  auto format(ggems::units::Quantity<Tag, Representation> const &quantity,
              FormatContext &context) const ->
      typename FormatContext::iterator {
    return formatter<string>::format(ggems::units::HumanReadable(quantity),
                                     context);
  }
};
} // namespace std
