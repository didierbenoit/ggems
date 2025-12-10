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
 * \file GGEMSBitsUnits.hh
 * \brief Strongly-typed quantity representing a count of bits.
 *
 * This header defines the \c Bits physical quantity used within GGEMS to
 * represent an amount of information expressed in raw bit units (bit-count).
 * Internally, the value is stored as an integer in engine base information
 * units, ensuring deterministic behaviour on CPU and GPU.
 *
 * Human-readable formatting (bit, kbit, Mbit, Gbit) is provided through
 * the \c HumanReadable() helper. Compile-time user-literals enable safe
 * and concise construction of \c Bits quantities.
 *
 * \authors Julien Bert, Didier Benoit
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 *   GNU General Public License v3.0
 */

#include "GGEMS/core/units/GGEMSQuantity.hh"

/// \cond
#include <array>
/// \endcond

namespace ggems::units {
/*!
 * \brief Information dimension representing a bit count.
 *
 * This alias binds the information exponent of the dimensional system
 * (\c Dim<0,0,1>) to the notion of discrete bit units. All quantities
 * based on \c InfoBitsDim store their value as engine-scale bit counts,
 * ensuring deterministic behaviour across CPU and GPU computations.
 */
using Bits = Quantity<InfoBitsDim, std::uint64_t>;

/*!
 * \brief Converts a \c Bits value into a human-readable string.
 *
 * The stored engine value (bits) is rescaled according to standard
 * orders of magnitude. The following suffixes are used:
 *   - bit  (1)
 *   - kbit (10e3)
 *   - Mbit (10e6)
 *   - Gbit (10e9)
 *   - Tbit (10e12)
 *
 * \param b         Information quantity expressed in bits.
 * \param precision Optional number of decimals (default: 7).
 * \param width     Optional field width used to align the printed value.
 * \return UTF-8 encoded text representing the rescaled bit quantity.
 */
inline std::string HumanReadable(Bits const &b, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double v = static_cast<long double>(b.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{{{1.0e12L, " Tb", 1.0e12L},
                                              {1.0e9L, " Gb", 1.0e9L},
                                              {1.0e6L, " Mb", 1.0e6L},
                                              {1.0e3L, " kb", 1.0e3L},
                                              {0.0L, " b", 1.0L}}};

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

  return std::format("{:.{}f} b", v, precision);
}

/*!
 * \brief Constructs a \c Bits quantity from an integer literal in bits.
 *
 * \param v Integer number of bits.
 * \return Corresponding \c Bits quantity.
 */
consteval Bits operator""_b(unsigned long long v) noexcept {
  return Bits{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief Constructs a \c Bits quantity from a floating-point literal in bits.
 *
 * \param v Floating-point number of bits.
 * \return Corresponding \c Bits quantity.
 */
consteval Bits operator""_b(long double v) noexcept {
  return Bits{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief Constructs a \c Bits quantity from an integer literal in kilobits.
 *
 * \param v Integer number of kilobits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_kb(unsigned long long v) noexcept {
  return Bits{static_cast<std::uint64_t>(v) * 1'000ULL};
}

/*!
 * \brief Constructs a \c Bits quantity from a floating-point literal in
 * kilobits.
 *
 * \param v Floating-point number of kilobits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_kb(long double v) noexcept {
  return Bits{static_cast<std::uint64_t>(v * 1.0e3L)};
}

/*!
 * \brief Constructs a \c Bits quantity from an integer literal in megabits.
 *
 * \param v Integer number of megabits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_Mb(unsigned long long v) noexcept {
  return Bits{static_cast<std::uint64_t>(v) * 1'000'000ULL};
}

/*!
 * \brief Constructs a \c Bits quantity from a floating-point literal in
 * megabits.
 *
 * \param v Floating-point number of megabits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_Mb(long double v) noexcept {
  return Bits{static_cast<std::uint64_t>(v * 1.0e6L)};
}

/*!
 * \brief Constructs a \c Bits quantity from an integer literal in gigabits.
 *
 * \param v Integer number of gigabits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_Gb(unsigned long long v) noexcept {
  return Bits{static_cast<std::uint64_t>(v) * 1'000'000'000ULL};
}

/*!
 * \brief Constructs a \c Bits quantity from a floating-point literal in
 * gigabits.
 *
 * \param v Floating-point number of gigabits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_Gb(long double v) noexcept {
  return Bits{static_cast<std::uint64_t>(v * 1.0e9L)};
}

/*!
 * \brief Constructs a \c Bits quantity from an integer literal in terabits.
 *
 * \param v Integer number of terabits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_Tb(unsigned long long v) noexcept {
  return Bits{static_cast<std::uint64_t>(v) * 1'000'000'000'000ULL};
}

/*!
 * \brief Constructs a \c Bits quantity from a floating-point literal in
 * terabits.
 *
 * \param v Floating-point number of terabits.
 * \return Corresponding \c Bits quantity expressed in bits.
 */
consteval Bits operator""_Tb(long double v) noexcept {
  return Bits{static_cast<std::uint64_t>(v * 1.0e12L)};
}
} // namespace ggems::units
