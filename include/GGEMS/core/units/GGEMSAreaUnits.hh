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
 * \file GGEMSAreaUnits.hh
 * \brief Area quantity alias and human-readable formatting helpers.
 *
 * This header defines the \c Area quantity used by GGEMS to represent
 * surface values in engine base units (square picometres), together
 * with a human-readable formatter and a set of user-defined literals
 * for expressing surface constants at compile time.
 *
 * The base stored unit is the square picometre (pm²). Higher-level
 * interfaces convert to nm², μm², mm², m² or km² as needed.
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
 * \brief Area quantity expressed in square picometres.
 *
 * The alias binds the \c AreaDim dimension (L²) to a \c long double
 * representation. All stored values are expressed in base engine units
 * (pm²), while user-facing APIs may expose higher-level square units.
 */
using Area = Quantity<AreaDim, long double>;

/*!
 * \brief Area quantity expressed in square picometres.
 *
 * The alias binds the \c AreaDim dimension (L²) to a \c long double
 * representation. All stored values are expressed in base engine units
 * (pm²), while user-facing APIs may expose higher-level square units.
 */
inline std::string HumanReadable(Area const &a, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double v = a.value;

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 6> units{{{1.0e30L, " km2", 1.0e30L},
                                              {1.0e24L, " m2", 1.0e24L},
                                              {1.0e18L, " mm2", 1.0e18L},
                                              {1.0e12L, " um2", 1.0e12L},
                                              {1.0e6L, " nm2", 1.0e6L},
                                              {0.0L, " pm2", 1.0L}}};

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

  return std::format("{:.{}f} pm2", v, precision);
}
/*!
 * \brief User-defined literal for area in square picometres (integer).
 *
 * \param v Integer literal representing a value in pm².
 * \return \c Area quantity equal to \c v pm².
 */
consteval Area operator""_pm2(std::uint64_t v) noexcept {
  return Area{static_cast<long double>(v)};
}

/*!
 * \brief User-defined literal for area in square picometres (floating-point).
 *
 * Fractional parts are preserved in the stored \c long double.
 *
 * \param v Floating-point literal representing a value in pm².
 * \return \c Area quantity approximating \c v pm².
 */
consteval Area operator""_pm2(long double v) noexcept { return Area{v}; }

/*!
 * \brief User-defined literal for area in square nanometres (integer).
 *
 * \param v Integer literal in nm².
 * \return \c Area quantity equal to \c v × 10⁶ pm².
 */
consteval Area operator""_nm2(std::uint64_t v) noexcept {
  return Area{static_cast<long double>(v) * 1.0e6L};
}

/*!
 * \brief User-defined literal for area in square nanometres (floating-point).
 *
 * \param v Floating-point literal in nm².
 * \return \c Area quantity approximating \c v × 10⁶ pm².
 */
consteval Area operator""_nm2(long double v) noexcept {
  return Area{v * 1.0e6L};
}

/*!
 * \brief User-defined literal for area in square micrometres (integer).
 *
 * \param v Integer literal in μm².
 * \return \c Area quantity equal to \c v × 10¹² pm².
 */
consteval Area operator""_um2(std::uint64_t v) noexcept {
  return Area{static_cast<long double>(v) * 1.0e12L};
}

/*!
 * \brief User-defined literal for area in square micrometres (floating-point).
 *
 * \param v Floating-point literal in μm².
 * \return \c Area quantity approximating \c v × 10¹² pm².
 */
consteval Area operator""_um2(long double v) noexcept {
  return Area{v * 1.0e12L};
}

/*!
 * \brief User-defined literal for area in square millimetres (integer).
 *
 * \param v Integer literal in mm².
 * \return \c Area quantity equal to \c v × 10¹⁸ pm².
 */
consteval Area operator""_mm2(std::uint64_t v) noexcept {
  return Area{static_cast<long double>(v) * 1.0e18L};
}

/*!
 * \brief User-defined literal for area in square millimetres (floating-point).
 *
 * \param v Floating-point literal in mm².
 * \return \c Area quantity approximating \c v × 10¹⁸ pm².
 */
consteval Area operator""_mm2(long double v) noexcept {
  return Area{v * 1.0e18L};
}

/*!
 * \brief User-defined literal for area in square metres (integer).
 *
 * \param v Integer literal in m².
 * \return \c Area quantity equal to \c v × 10²⁴ pm².
 */
consteval Area operator""_m2(std::uint64_t v) noexcept {
  return Area{static_cast<long double>(v) * 1.0e24L};
}

/*!
 * \brief User-defined literal for area in square metres (floating-point).
 *
 * \param v Floating-point literal in m².
 * \return \c Area quantity approximating \c v × 10²⁴ pm².
 */
consteval Area operator""_m2(long double v) noexcept {
  return Area{v * 1.0e24L};
}

/*!
 * \brief User-defined literal for area in square kilometres (integer).
 *
 * \param v Integer literal in km².
 * \return \c Area quantity equal to \c v × 10³⁰ pm².
 */
consteval Area operator""_km2(std::uint64_t v) noexcept {
  return Area{static_cast<long double>(v) * 1.0e30L};
}

/*!
 * \brief User-defined literal for area in square kilometres (floating-point).
 *
 * \param v Floating-point literal in km².
 * \return \c Area quantity approximating \c v × 10³⁰ pm².
 */
consteval Area operator""_km2(long double v) noexcept {
  return Area{v * 1.0e30L};
}
} // namespace ggems::units
