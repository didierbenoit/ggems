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
 * \file GGEMSSpeedUnits.hh
 * \brief Strongly-typed physical speed expressed in engine base units (pm/ps).
 *
 * \author Julien BERT
 * \author Didier BENOIT
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 *
 * \details
 * The \c Speed quantity represents a physical propagation rate, internally
 * stored as picometres per picosecond (pm/ps). Since \c 1 m = 10¹² pm and
 * \c 1 s = 10¹² ps, the ratio \c m/s maps exactly to \c pm/ps. Therefore,
 * engine speed values correspond directly to SI velocities without requiring
 * any rescaling.
 *
 * The provided \c HumanReadable function formats speed back into
 * the standard SI form "m/s". User-defined literals allow direct construction
 * of speed quantities from SI values, while \c MakeSpeed offers a convenient
 * helper to derive a \c Speed from a \c Length over a \c Time.
 */

#include "GGEMSLengthUnits.hh"
#include "GGEMSTimeUnits.hh"

namespace ggems::units {
/*!
 * \brief Physical speed quantity stored in picometres per picosecond (pm/ps).
 *
 * This alias binds the compile-time speed dimension to a long-double
 * representation. Internally, values are expressed in pm/ps. Since
 * \c 1 m/s = 1 pm/ps exactly, stored engine quantities correspond directly
 * to SI velocities.
 */
using Speed = Quantity<SpeedDim, long double>;

/*!
 * \brief Formats a speed value into an SI-readable string.
 *
 * The internal representation \c v.value is expressed in pm/ps, which is
 * dimensionally equivalent to m/s. The resulting string displays the value
 * in conventional SI notation, with adjustable precision and optional field
 * width.
 *
 * \param v          Speed quantity to format.
 * \param precision  Number of digits after the decimal point (default: 7).
 * \param width      Optional minimum field width. If negative, no width is
 *                   enforced.
 *
 * \return A UTF-8 string such as "3.0000000 m/s".
 */
inline std::string HumanReadable(Speed const &v, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  std::string fmt;
  if (width < 0) {
    fmt = std::format("{{:.{}f}} m/s", precision);
  } else {
    fmt = std::format("{{:{}.{}f}} m/s", width, precision);
  }

  return std::vformat(fmt, std::make_format_args(v.value));
}

/*!
 * \brief User-defined literal for integer speed in metres per second.
 *
 * \param v Integer literal representing speed in m/s.
 * \return \c Speed quantity equal to \c v pm/ps.
 */
consteval Speed operator""_m_s(std::uint64_t v) noexcept {
  return Speed{static_cast<long double>(v)};
}

/*!
 * \brief User-defined literal for floating-point speed in metres per second.
 *
 * \param v Floating-point literal representing speed in m/s.
 * \return \c Speed quantity approximating \c v pm/ps.
 */
consteval Speed operator""_m_s(long double v) noexcept { return Speed{v}; }

/*!
 * \brief Constructs a speed value from a travelled length over a duration.
 *
 * Both \c Length and \c Time follow GGEMS base units internally:
 *   - length in picometres (pm)
 *   - time in picoseconds (ps)
 *
 * Dividing length by time therefore yields a quantity in pm/ps, exactly
 * equivalent to m/s in standard SI units. No rescaling is necessary.
 *
 * \param L Travelled length, internally expressed in picometres.
 * \param t Elapsed time, internally expressed in picoseconds.
 *
 * \return \c Speed quantity equal to \c L/t, expressed in pm/ps (≡ m/s).
 */
inline Speed MakeSpeed(Length L, Time t) noexcept {
  long double pm = static_cast<long double>(L.value);
  long double ps = static_cast<long double>(t.value);
  return Speed{pm / ps};
}
} // namespace ggems::units
