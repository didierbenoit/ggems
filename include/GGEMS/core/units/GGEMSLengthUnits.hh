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
 * \file GGEMSLengthUnits.hh
 * \brief Length quantity and formatting utilities for GGEMS.
 *
 * Defines the \c Length quantity stored in engine base units
 * (picometres). User-defined literals provide conversions from common
 * physical scales (nm → km), and a helper function emits readable
 * textual representations with SI prefixes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
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
 * \brief Length quantity expressed in picometres (pm).
 *
 * This alias binds the \c LengthDim dimension to an unsigned 64-bit
 * representation. All stored values correspond to engine base units in
 * picometres. Higher-level user-facing scales (nm → km) are produced
 * only by \c HumanReadable for display convenience.
 */
using Length = Quantity<LengthDim, std::uint64_t>;

/*!
 * \brief Converts a length quantity to a human-readable UTF-8 string.
 *
 * Internal values are stored as picometres (pm). For readability, the
 * function selects the most appropriate SI prefix (nm, μm, mm, m, km)
 * based on magnitude and formats the scaled value using fixed precision
 * and optional minimum width.
 *
 * \param l         Length quantity expressed in picometres.
 * \param precision Digits after the decimal point.
 * \param width     Minimum formatted width; if negative, no constraint.
 *
 * \return UTF-8 encoded textual representation with an SI suffix.
 */
inline std::string HumanReadable(Length const &l, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(l.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 6> units{{{1.0e15L, " km", 1.0e15L},
                                              {1.0e12L, " m", 1.0e12L},
                                              {1.0e9L, " mm", 1.0e9L},
                                              {1.0e6L, " um", 1.0e6L},
                                              {1.0e3L, " nm", 1.0e3L},
                                              {0.0L, " pm", 1.0L}}};

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

  return std::format("{:.{}f} pm", v, precision);
}

/*!
 * \brief User-defined literal for picometres (pm).
 * \param v Integer literal in pm.
 * \return \c Length quantity equal to \c v pm.
 */
consteval Length operator""_pm(std::uint64_t v) noexcept { return Length{v}; }

/*!
 * \brief User-defined literal for picometres (pm) from floating-point.
 * \param v Floating-point literal in pm.
 * \return \c Length quantity approximating \c v pm (fraction truncated).
 */
consteval Length operator""_pm(long double v) noexcept {
  return Length{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief User-defined literal for nanometres (nm).
 * \param v Integer literal in nm.
 * \return \c Length quantity equal to \c v × 10³ pm.
 */
consteval Length operator""_nm(std::uint64_t v) noexcept {
  return Length{v * 1000ULL};
}

/*!
 * \brief User-defined literal for nanometres (nm) from floating-point.
 * \param v Floating-point literal in nm.
 * \return \c Length quantity approximating \c v × 10³ pm.
 */
consteval Length operator""_nm(long double v) noexcept {
  return Length{static_cast<std::uint64_t>(v * 1.0e3L)};
}

/*!
 * \brief User-defined literal for micrometres (μm).
 * \param v Integer literal in um.
 * \return \c Length quantity equal to \c v × 10⁶ pm.
 */
consteval Length operator""_um(std::uint64_t v) noexcept {
  return Length{v * 1'000'000ULL};
}

/*!
 * \brief User-defined literal for micrometres (μm) from floating-point.
 * \param v Floating-point literal in um.
 * \return \c Length quantity approximating \c v × 10⁶ pm.
 */
consteval Length operator""_um(long double v) noexcept {
  return Length{static_cast<std::uint64_t>(v * 1.0e6L)};
}

/*!
 * \brief User-defined literal for millimetres (mm).
 * \param v Integer literal in mm.
 * \return \c Length quantity equal to \c v × 10⁹ pm.
 */
consteval Length operator""_mm(std::uint64_t v) noexcept {
  return Length{v * 1'000'000'000ULL};
}

/*!
 * \brief User-defined literal for millimetres (mm) from floating-point.
 * \param v Floating-point literal in mm.
 * \return \c Length quantity approximating \c v × 10⁹ pm.
 */
consteval Length operator""_mm(long double v) noexcept {
  return Length{static_cast<std::uint64_t>(v * 1.0e9L)};
}

/*!
 * \brief User-defined literal for metres (m).
 * \param v Integer literal in m.
 * \return \c Length quantity equal to \c v × 10¹² pm.
 */
consteval Length operator""_m(std::uint64_t v) noexcept {
  return Length{v * 1'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for metres (m) from floating-point.
 * \param v Floating-point literal in m.
 * \return \c Length quantity approximating \c v × 10¹² pm.
 */
consteval Length operator""_m(long double v) noexcept {
  return Length{static_cast<std::uint64_t>(v * 1.0e12L)};
}

/*!
 * \brief User-defined literal for kilometres (km).
 *
 * \param v Integer literal in kilometres.
 * \return \c Length quantity equal to \c v × 10¹⁵ pm.
 */
consteval Length operator""_km(std::uint64_t v) noexcept {
  return Length{v * 1'000'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for kilometres (km) from floating-point.
 *
 * \param v Floating-point literal in kilometres.
 * \return \c Length quantity approximating \c v × 10¹⁵ pm.
 */
consteval Length operator""_km(long double v) noexcept {
  return Length{static_cast<std::uint64_t>(v * 1.0e15L)};
}
} // namespace ggems::units
