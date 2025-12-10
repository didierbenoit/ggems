#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSCrossSectionUnits.hh
 * \brief Cross-section quantity alias and human-readable helpers.
 *
 * This header defines the \c CrossSection quantity used by GGEMS to express
 * microscopic interaction cross-sections. All stored values represent areas
 * in the engine base unit, square picometres (pm²). User-defined literals
 * allow construction from common magnitudes such as barn, millibarn,
 * microbarn, nanobarn and picobarn, which are internally converted into pm².
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

/// \cond
#include <array>
/// \endcond

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {
/*!
 * \brief Cross-section quantity expressed in pm².
 *
 * This alias binds the \c AreaDim dimension to an unsigned 64-bit
 * representation. All stored values directly correspond to pm². Higher-level
 * notation, such as barns or derived multiples, is handled by user-defined
 * literals which apply the appropriate scaling at compile time.
 */
using CrossSection = Quantity<AreaDim, std::uint64_t>;

/*!
 * \brief Formats a cross-section value into a human-readable string.
 *
 * The output selects the most appropriate suffix among kbarn, barn, mbarn,
 * ubarn, nbarn or pbarn based on the magnitude of the quantity. Internally,
 * the stored value is treated as square picometres, and rescaling is applied
 * before formatting to expose familiar units used in radiation physics.
 *
 * \param cs        Cross-section quantity expressed in pm².
 * \param precision Formatting precision applied to the mantissa.
 * \param width     Optional minimum field width. If negative, no width
 *                  constraint is applied.
 *
 * \return UTF-8 encoded string describing the cross-section with unit suffix.
 */
inline std::string HumanReadable(CrossSection const &cs,
                                 std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double v = static_cast<long double>(cs.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 6> units{{{1.0e15L, " kbarn", 1.0e15L},
                                              {1.0e12L, " barn", 1.0e12L},
                                              {1.0e9L, " mbarn", 1.0e9L},
                                              {1.0e6L, " ubarn", 1.0e6L},
                                              {1.0e3L, " nbarn", 1.0e3L},
                                              {0.0L, " pbarn", 1.0L}}};

  for (auto const &u : units) {
    if (v >= u.threshold) {

      long double scaled = v / u.scale;

      std::string fmt;

      if (width < 0) {
        fmt = std::format("{{:.{}f}}{}", precision, u.suffix);
      } else {
        fmt = std::format("{{:{}.{}f}}{}", width, precision, u.suffix);
      }

      return std::vformat(fmt, std::make_format_args(scaled));
    }
  }

  return std::format("{:.{}f} pbarn", v, precision);
};

/*!
 * \brief Literal for cross-section in picobarns (integer).
 *
 * \param v Integer literal in pbarn.
 * \return \c CrossSection quantity equal to \c v pbarn.
 */
consteval CrossSection operator""_pbarn(unsigned long long v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief Literal for cross-section in picobarns (floating-point).
 *
 * \param v Floating-point literal in pbarn.
 * \return \c CrossSection quantity approximating \c v pbarn.
 */
consteval CrossSection operator""_pbarn(long double v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief Literal for cross-section in nanobarns (integer).
 *
 * \param v Integer literal in nbarn.
 * \return \c CrossSection quantity equal to \c v × 10³ pbarn.
 */
consteval CrossSection operator""_nbarn(unsigned long long v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v) * 1000ULL};
}

/*!
 * \brief Literal for cross-section in nanobarns (floating-point).
 *
 * \param v Floating-point literal in nbarn.
 * \return \c CrossSection quantity approximating \c v × 10³ pbarn.
 */
consteval CrossSection operator""_nbarn(long double v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v * 1.0e3L)};
}

/*!
 * \brief Literal for cross-section in microbarns (integer).
 *
 * \param v Integer literal in ubarn.
 * \return \c CrossSection quantity equal to \c v × 10⁶ pbarn.
 */
consteval CrossSection operator""_ubarn(unsigned long long v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v) * 1'000'000ULL};
}

/*!
 * \brief Literal for cross-section in microbarns (floating-point).
 *
 * \param v Floating-point literal in ubarn.
 * \return \c CrossSection quantity approximating \c v × 10⁶ pbarn.
 */
consteval CrossSection operator""_ubarn(long double v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v * 1.0e6L)};
}

/*!
 * \brief Literal for cross-section in millibarns (integer).
 *
 * \param v Integer literal in mbarn.
 * \return \c CrossSection quantity equal to \c v × 10⁹ pbarn.
 */
consteval CrossSection operator""_mbarn(unsigned long long v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v) * 1'000'000'000ULL};
}

/*!
 * \brief Literal for cross-section in millibarns (floating-point).
 *
 * \param v Floating-point literal in mbarn.
 * \return \c CrossSection quantity approximating \c v × 10⁹ pbarn.
 */
consteval CrossSection operator""_mbarn(long double v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v * 1.0e9L)};
}

/*!
 * \brief Literal for cross-section in barns (integer).
 *
 * \param v Integer literal in barn.
 * \return \c CrossSection quantity equal to \c v × 10¹² pbarn.
 */
consteval CrossSection operator""_barn(unsigned long long v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v) * 1'000'000'000'000ULL};
}

/*!
 * \brief Literal for cross-section in barns (floating-point).
 *
 * \param v Floating-point literal in barn.
 * \return \c CrossSection quantity approximating \c v × 10¹² pbarn.
 */
consteval CrossSection operator""_barn(long double v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v * 1.0e12L)};
}

/*!
 * \brief Literal for cross-section in kilobarns (integer).
 *
 * \param v Integer literal in kbarn.
 * \return \c CrossSection quantity equal to \c v × 10¹⁵ pbarn.
 */
consteval CrossSection operator""_kbarn(unsigned long long v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v) * 1'000'000'000'000'000ULL};
}

/*!
 * \brief Literal for cross-section in kilobarns (floating-point).
 *
 * \param v Floating-point literal in kbarn.
 * \return \c CrossSection quantity approximating \c v × 10¹⁵ pbarn.
 */
consteval CrossSection operator""_kbarn(long double v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v * 1.0e15L)};
}

/*!
 * \brief Literal for cross-section construction from mm² (floating-point).
 *
 * This helper provides a simple conversion path when a surface expressed
 * in mm² must be represented as a cross-section value in pm². The literal
 * multiplies by the appropriate factor at compile time.
 *
 * \param v Floating-point value in mm².
 * \return \c CrossSection quantity equal to \c v mm² expressed in pm².
 */
consteval CrossSection operator""_mm2cs(long double v) noexcept {
  return CrossSection{static_cast<std::uint64_t>(v * 1.0e34L)};
}
} // namespace ggems::units
