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
 * \file GGEMSFrequencyUnits.hh
 * \brief Frequency units for GGEMS expressed in base Hertz (Hz).
 *
 * Defines the \c Frequency quantity bound to the frequency dimension,
 * together with user-defined literals for constructing strongly-typed
 * frequency values. A helper function provides human-readable textual
 * representations using SI prefixes (kHz, MHz, GHz, THz).
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMSQuantity.hh"

#include <array>

namespace ggems::units {
/*!
 * \brief Frequency quantity expressed in Hertz.
 *
 * This alias binds the \c FrequencyDim dimension to an unsigned 64-bit
 * representation. Stored values correspond to base engine units (Hz),
 * while higher-level formatting may expose scaled SI prefixes such as
 * kHz, MHz, GHz, or THz.
 */
using Frequency = Quantity<FrequencyDim, std::uint64_t>;

/*!
 * \brief Converts a frequency quantity into a human-readable UTF-8 string.
 *
 * The function selects an appropriate SI prefix based on the magnitude
 * (kHz, MHz, GHz, THz). Text output uses fixed precision and optional
 * minimum width, providing clear alignment in logs or UI components.
 *
 * \param f         Frequency quantity expressed in Hertz.
 * \param precision Digits after the decimal point.
 * \param width     Minimum formatted field width; if negative, no constraint.
 *
 * \return UTF-8 text representation of the scaled frequency.
 */
inline std::string HumanReadable(Frequency const &f, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(f.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{{{1.0e12L, " THz", 1.0e12L},
                                              {1.0e9L, " GHz", 1.0e9L},
                                              {1.0e6L, " MHz", 1.0e6L},
                                              {1.0e3L, " kHz", 1.0e3L},
                                              {0.0L, " Hz", 1.0L}}};

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

  return std::format("{:.{}f} Hz", v, precision);
}

/*!
 * \brief Constructs a \c Frequency quantity from an integer literal in Hertz.
 * \param v Integer literal in Hz.
 * \return \c Frequency value equal to \c v Hz.
 */
consteval Frequency operator""_Hz(std::uint64_t v) noexcept {
  return Frequency{v};
}

/*!
 * \brief Constructs a \c Frequency quantity from a floating-point literal in
 * Hertz.
 * \param v Floating-point literal in Hz.
 * \return \c Frequency value approximating \c v Hz (truncated to integer).
 */
consteval Frequency operator""_Hz(long double v) noexcept {
  return Frequency{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief Constructs a \c Frequency quantity from an integer literal in
 * kilohertz.
 * \param v Integer literal in kHz.
 * \return \c Frequency value equal to \c v × 10³ Hz.
 */
consteval Frequency operator""_kHz(std::uint64_t v) noexcept {
  return Frequency{v * 1000ull};
}

/*!
 * \brief Constructs a \c Frequency quantity from a floating-point literal in
 * kilohertz.
 * \param v Floating-point literal in kHz.
 * \return \c Frequency value approximating \c v × 10³ Hz.
 */
consteval Frequency operator""_kHz(long double v) noexcept {
  return Frequency{static_cast<std::uint64_t>(v * 1.0e3L)};
}

/*!
 * \brief Constructs a \c Frequency quantity from an integer literal in
 * megahertz.
 * \param v Integer literal in MHz.
 * \return \c Frequency value equal to \c v × 10⁶ Hz.
 */
consteval Frequency operator""_MHz(std::uint64_t v) noexcept {
  return Frequency{v * 1'000'000ull};
}

/*!
 * \brief Constructs a \c Frequency quantity from a floating-point literal in
 * megahertz.
 * \param v Floating-point literal in MHz.
 * \return \c Frequency value approximating \c v × 10⁶ Hz.
 */
consteval Frequency operator""_MHz(long double v) noexcept {
  return Frequency{static_cast<std::uint64_t>(v * 1.0e6L)};
}

/*!
 * \brief Constructs a \c Frequency quantity from an integer literal in
 * gigahertz.
 * \param v Integer literal in GHz.
 * \return \c Frequency value equal to \c v × 10⁹ Hz.
 */
consteval Frequency operator""_GHz(std::uint64_t v) noexcept {
  return Frequency{v * 1'000'000'000ull};
}

/*!
 * \brief Constructs a \c Frequency quantity from a floating-point literal in
 * gigahertz.
 * \param v Floating-point literal in GHz.
 * \return \c Frequency value approximating \c v × 10⁹ Hz.
 */
consteval Frequency operator""_GHz(long double v) noexcept {
  return Frequency{static_cast<std::uint64_t>(v * 1.0e9L)};
}

/*!
 * \brief Constructs a \c Frequency quantity from an integer literal in
 * terahertz.
 * \param v Integer literal in THz.
 * \return \c Frequency value equal to \c v × 10¹² Hz.
 */
consteval Frequency operator""_THz(std::uint64_t v) noexcept {
  return Frequency{v * 1'000'000'000'000ull};
}

/*!
 * \brief Constructs a \c Frequency quantity from a floating-point literal in
 * terahertz.
 * \param v Floating-point literal in THz.
 * \return \c Frequency value approximating \c v × 10¹² Hz.
 */
consteval Frequency operator""_THz(long double v) noexcept {
  return Frequency{static_cast<std::uint64_t>(v * 1.0e12L)};
}
} // namespace ggems::units
