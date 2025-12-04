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
// ************************************************************************

/*!
 * \file GGEMSVolumeUnits.hh
 * \brief Volume quantity alias and conversion helpers.
 *
 * This header defines the \c Volume quantity used by GGEMS to represent
 * volumetric values in engine base units (cubic picometres), along with
 * a human-readable formatter and a set of compile-time user-defined
 * literals for common volumetric scales.
 *
 * The stored base unit is the cubic picometre (pm³). Higher-level
 * interfaces may convert to nm³, μm³, mm³, m³, or km³ for display.
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

#include "GGEMSQuantity.hh"

namespace ggems::units {

/*!
 * \brief Volume quantity expressed in cubic picometres.
 *
 * The alias binds the \c VolumeDim dimension (L³) to a \c long double
 * representation. All values are stored in engine base units (pm³),
 * while user-facing formatting may present higher-level scales.
 */
using Volume = Quantity<VolumeDim, long double>;

/*!
 * \brief Converts a volume quantity to a formatted UTF-8 string.
 *
 * Depending on magnitude, the function selects among pm³, nm³, μm³,
 * mm³, m³, or km³. Values are scaled accordingly and a suffix is added.
 *
 * \param v         Volume quantity in pm³.
 * \param precision Number of fractional digits in the formatted output.
 * \param width     Optional minimum field width. If negative, no width
 *                  constraint applies.
 *
 * \return Human-readable representation of the stored volume.
 */
inline std::string HumanReadable(Volume const &v, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const val = v.value;

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 6> units{{{1.0e45L, " km3", 1.0e45L},
                                              {1.0e36L, " m3", 1.0e36L},
                                              {1.0e27L, " mm3", 1.0e27L},
                                              {1.0e18L, " um3", 1.0e18L},
                                              {1.0e9L, " nm3", 1.0e9L},
                                              {0.0L, " pm3", 1.0L}}};

  for (auto const &u : units) {
    if (val >= u.threshold) {

      long double scaled = val / u.scale;

      std::string fmt;
      if (width < 0) {
        fmt = std::format("{{:.{}f}}{}", precision, u.suffix);
      } else {
        fmt = std::format("{{:{}.{}f}}{}", width, precision, u.suffix);
      }

      return std::vformat(fmt, std::make_format_args(scaled));
    }
  }

  return std::format("{:.{}f} pm3", val, precision);
}

/*!
 * \brief User-defined literal for integer volume in pm³.
 * \param v Integer literal in pm³.
 * \return \c Volume quantity equal to \c v pm³.
 */
consteval Volume operator""_pm3(std::uint64_t v) noexcept {
  return Volume{static_cast<long double>(v)};
}

/*!
 * \brief User-defined literal for floating-point volume in pm³.
 * \param v Floating-point literal in pm³.
 * \return \c Volume quantity approximating \c v pm³.
 */
consteval Volume operator""_pm3(long double v) noexcept { return Volume{v}; }

/*!
 * \brief User-defined literal for integer volume in nm³.
 * \param v Integer literal in nm³.
 * \return \c Volume quantity equal to \c v × 10⁹ pm³.
 */
consteval Volume operator""_nm3(std::uint64_t v) noexcept {
  return Volume{static_cast<long double>(v) * 1.0e9L};
}

/*!
 * \brief User-defined literal for floating-point volume in nm³.
 * \param v Floating-point literal in nm³.
 * \return \c Volume quantity approximating \c v × 10⁹ pm³.
 */
consteval Volume operator""_nm3(long double v) noexcept {
  return Volume{v * 1.0e9L};
}

/*!
 * \brief User-defined literal for integer volume in μm³.
 * \param v Integer literal in μm³.
 * \return \c Volume quantity equal to \c v × 10¹⁸ pm³.
 */
consteval Volume operator""_um3(std::uint64_t v) noexcept {
  return Volume{static_cast<long double>(v) * 1.0e18L};
}

/*!
 * \brief User-defined literal for floating-point volume in μm³.
 * \param v Floating-point literal in μm³.
 * \return \c Volume quantity approximating \c v × 10¹⁸ pm³.
 */
consteval Volume operator""_um3(long double v) noexcept {
  return Volume{v * 1.0e18L};
}

/*!
 * \brief User-defined literal for integer volume in mm³.
 * \param v Integer literal in mm³.
 * \return \c Volume quantity equal to \c v × 10²⁷ pm³.
 */
consteval Volume operator""_mm3(std::uint64_t v) noexcept {
  return Volume{static_cast<long double>(v) * 1.0e27L};
}

/*!
 * \brief User-defined literal for floating-point volume in mm³.
 * \param v Floating-point literal in mm³.
 * \return \c Volume quantity approximating \c v × 10²⁷ pm³.
 */
consteval Volume operator""_mm3(long double v) noexcept {
  return Volume{v * 1.0e27L};
}

/*!
 * \brief User-defined literal for integer volume in m³.
 * \param v Integer literal in m³.
 * \return \c Volume quantity equal to \c v × 10³⁶ pm³.
 */
consteval Volume operator""_m3(std::uint64_t v) noexcept {
  return Volume{static_cast<long double>(v) * 1.0e36L};
}

/*!
 * \brief User-defined literal for floating-point volume in m³.
 * \param v Floating-point literal in m³.
 * \return \c Volume quantity approximating \c v × 10³⁶ pm³.
 */
consteval Volume operator""_m3(long double v) noexcept {
  return Volume{v * 1.0e36L};
}

/*!
 * \brief User-defined literal for integer volume in km³.
 * \param v Integer literal in km³.
 * \return \c Volume quantity equal to \c v × 10⁴⁵ pm³.
 */
consteval Volume operator""_km3(std::uint64_t v) noexcept {
  return Volume{static_cast<long double>(v) * 1.0e45L};
}

/*!
 * \brief User-defined literal for floating-point volume in km³.
 * \param v Floating-point literal in km³.
 * \return \c Volume quantity approximating \c v × 10⁴⁵ pm³.
 */
consteval Volume operator""_km3(long double v) noexcept {
  return Volume{v * 1.0e45L};
}

} // namespace ggems::units
