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
 * \file GGEMSTimeUnits.hh
 * \brief Time quantity alias and human-readable formatting helpers.
 *
 * This header defines the \c Time quantity used by GGEMS to represent
 * durations in engine base units (picoseconds), together with a
 * human-readable formatter and a set of user-defined literals for
 * expressing time constants at compile time.
 *
 * The base unit for storage is the picosecond (ps); higher-level
 * interfaces convert to or from nanoseconds, microseconds, milliseconds,
 * seconds, minutes and hours as needed.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include <array>

#include "GGEMSQuantity.hh"

namespace ggems::units {
/*!
 * \brief Time quantity expressed in picoseconds.
 *
 * This alias binds the \c TimeDim dimension to an unsigned 64-bit
 * representation. All stored values are expressed in base engine units
 * (picoseconds), while user-facing APIs may expose higher-level units.
 */
using Time = Quantity<TimeDim, uint64_t>;

/*!
 * \brief Converts a time quantity to a human-readable string.
 *
 * For durations shorter than 60 seconds, the function selects the most
 * suitable unit among ps, ns, us, ms and s depending on the magnitude.
 * For durations longer than or equal to 60 seconds, it returns a compact
 * breakdown in hours, minutes, seconds and milliseconds.
 *
 * \param t         Time quantity in base units (picoseconds).
 * \param precision Number of digits after the decimal point for the
 *                  mantissa in the short-duration case.
 * \param width     Optional minimum field width; if negative, no width
 *                  constraint is applied.
 *
 * \return UTF-8 encoded string describing the duration with its unit.
 */
inline std::string HumanReadable(Time const &t, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(t.value);

  // >= 60 s → h / min / s / ms
  if (v >= 60.0L * 1.0e12L) {
    uint64_t total_ps = t.value;

    uint64_t total_s = total_ps / 1'000'000'000'000ull;
    uint64_t ps_rest = total_ps % 1'000'000'000'000ull;

    uint64_t hours = total_s / 3600ull;
    uint64_t minutes = (total_s % 3600ull) / 60ull;
    uint64_t seconds = total_s % 60ull;

    uint64_t ms = ps_rest / 1'000'000'000ull; // 1 ms = 1e9 ps

    if (hours > 0) {
      return std::format("{} h {} min {} s {} ms", hours, minutes, seconds, ms);
    }

    return std::format("{} min {} s {} ms", minutes, seconds, ms);
  }

  struct Unit {
    long double threshold_ps;
    long double scale;
    std::string_view suffix;
  };

  static constexpr std::array<Unit, 5> units{{
      {1.0e12L, 1.0e12L, " s"}, // >= 1s
      {1.0e9L, 1.0e9L, " ms"},  // >= 1ms
      {1.0e6L, 1.0e6L, " us"},  // >= 1us
      {1.0e3L, 1.0e3L, " ns"},  // >= 1ns
      {0.0L, 1.0L, " ps"}       // < 1ns
  }};

  for (auto const &u : units) {
    if (v >= u.threshold_ps) {

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

  return std::format("{:.{}f} ps", v, precision);
}

/*!
 * \brief User-defined literal for time in picoseconds (integer).
 *
 * \param v Integer literal representing a number of picoseconds.
 * \return \c Time quantity equal to \c v ps.
 */
consteval Time operator""_ps(std::uint64_t v) noexcept { return Time{v}; }

/*!
 * \brief User-defined literal for time in picoseconds (floating-point).
 *
 * Fractional parts are truncated when converted to the integer engine
 * unit representation.
 *
 * \param v Floating-point literal representing a number of picoseconds.
 * \return \c Time quantity approximating \c v ps.
 */
consteval Time operator""_ps(long double v) noexcept {
  return Time{static_cast<uint64_t>(v)};
}

/*!
 * \brief User-defined literal for time in nanoseconds (integer).
 *
 * \param v Integer literal representing a number of nanoseconds.
 * \return \c Time quantity equal to \c v ns expressed in picoseconds.
 */
consteval Time operator""_ns(std::uint64_t v) noexcept {
  return Time{v * 1000ull};
}

/*!
 * \brief User-defined literal for time in nanoseconds (floating-point).
 *
 * \param v Floating-point literal representing a number of nanoseconds.
 * \return \c Time quantity approximating \c v ns expressed in picoseconds.
 */
consteval Time operator""_ns(long double v) noexcept {
  return Time{static_cast<uint64_t>(v * 1.0e3L)};
}

/*!
 * \brief User-defined literal for time in microseconds (integer).
 *
 * \param v Integer literal representing a number of microseconds.
 * \return \c Time quantity equal to \c v µs expressed in picoseconds.
 */
consteval Time operator""_us(std::uint64_t v) noexcept {
  return Time{v * 1'000'000ull};
}

/*!
 * \brief User-defined literal for time in microseconds (floating-point).
 *
 * \param v Floating-point literal representing a number of microseconds.
 * \return \c Time quantity approximating \c v µs expressed in picoseconds.
 */
consteval Time operator""_us(long double v) noexcept {
  return Time{static_cast<uint64_t>(v * 1.0e6L)};
}

/*!
 * \brief User-defined literal for time in milliseconds (integer).
 *
 * \param v Integer literal representing a number of milliseconds.
 * \return \c Time quantity equal to \c v ms expressed in picoseconds.
 */
consteval Time operator""_ms(std::uint64_t v) noexcept {
  return Time{v * 1'000'000'000ull};
}

/*!
 * \brief User-defined literal for time in milliseconds (floating-point).
 *
 * \param v Floating-point literal representing a number of milliseconds.
 * \return \c Time quantity approximating \c v ms expressed in picoseconds.
 */
consteval Time operator""_ms(long double v) noexcept {
  return Time{static_cast<uint64_t>(v * 1.0e9L)};
}

/*!
 * \brief User-defined literal for time in seconds (integer).
 *
 * \param v Integer literal representing a number of seconds.
 * \return \c Time quantity equal to \c v s expressed in picoseconds.
 */
consteval Time operator""_s(std::uint64_t v) noexcept {
  return Time{v * 1'000'000'000'000ull};
}

/*!
 * \brief User-defined literal for time in seconds (floating-point).
 *
 * \param v Floating-point literal representing a number of seconds.
 * \return \c Time quantity approximating \c v s expressed in picoseconds.
 */
consteval Time operator""_s(long double v) noexcept {
  return Time{static_cast<uint64_t>(v * 1.0e12L)};
}

/*!
 * \brief User-defined literal for time in minutes (integer).
 *
 * \param v Integer literal representing a number of minutes.
 * \return \c Time quantity equal to \c v min expressed in picoseconds.
 */
consteval Time operator""_min(std::uint64_t v) noexcept {
  return Time{v * 60ull * 1'000'000'000'000ull};
}

/*!
 * \brief User-defined literal for time in minutes (floating-point).
 *
 * \param v Floating-point literal representing a number of minutes.
 * \return \c Time quantity approximating \c v min expressed in picoseconds.
 */
consteval Time operator""_min(long double v) noexcept {
  return Time{static_cast<uint64_t>(v * 60.0L * 1.0e12L)};
}

/*!
 * \brief User-defined literal for time in hours (integer).
 *
 * \param v Integer literal representing a number of hours.
 * \return \c Time quantity equal to \c v h expressed in picoseconds.
 */
consteval Time operator""_h(std::uint64_t v) noexcept {
  return Time{v * 3600ull * 1'000'000'000'000ull};
}

/*!
 * \brief User-defined literal for time in hours (floating-point).
 *
 * \param v Floating-point literal representing a number of hours.
 * \return \c Time quantity approximating \c v h expressed in picoseconds.
 */
consteval Time operator""_h(long double v) noexcept {
  return Time{static_cast<uint64_t>(v * 3600.0L * 1.0e12L)};
}
} // namespace ggems::units
