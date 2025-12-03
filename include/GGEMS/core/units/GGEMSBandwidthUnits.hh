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
 * \file GGEMSBandwidthUnits.hh
 * \brief Bandwidth quantity and formatting utilities for GGEMS.
 *
 * Defines the \c Bandwidth quantity representing byte throughput per
 * second (B/s) computed from engine base units. User-defined literals
 * enable concise construction of bandwidth values, and a helper function
 * provides human-readable SI scaling (kB/s → TB/s).
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMSBytesUnits.hh"
#include "GGEMSTimeUnits.hh"

#include <array>

namespace ggems::units {
/*!
 * \brief Bandwidth quantity expressed as bytes per picosecond (B/ps).
 *
 * This alias binds the \c BandwidthDim dimension to a \c long double
 * representation. Stored values correspond to engine base units:
 * bytes transferred per picosecond. Since \c Time is internally stored
 * in picoseconds, this ensures coherent dimensional behaviour across
 * the engine. When displayed through \c HumanReadable, the value is
 * converted to bytes per second (B/s) for clarity.
 */
using Bandwidth = Quantity<BandwidthDim, long double>;

/*!
 * \brief Converts a bandwidth quantity into a human-readable UTF-8 string.
 *
 * Internally, \c Bandwidth stores values in bytes per picosecond (B/ps),
 * reflecting engine base units. For readability, the function scales
 * the physical value to bytes per second (B/s) by multiplying by 10¹²,
 * then applies SI prefixes (kB/s → TB/s). Output formatting uses fixed
 * precision and optional minimum width.
 *
 * \param bw        Bandwidth quantity (stored as B/ps).
 * \param precision Digits after the decimal point.
 * \param width     Minimum formatted width; if negative, no constraint.
 *
 * \return UTF-8 encoded textual representation in scaled B/s.
 */
inline std::string HumanReadable(Bandwidth const &bw, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = bw.value * 1.0e12L;

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{{{1.0e12L, " TB/s", 1.0e12L},
                                              {1.0e9L, " GB/s", 1.0e9L},
                                              {1.0e6L, " MB/s", 1.0e6L},
                                              {1.0e3L, " kB/s", 1.0e3L},
                                              {0.0L, " B/s", 1.0L}}};

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

  return std::format("{:.{}f} B/s", v, precision);
}

/*!
 * \brief User-defined literal for bandwidth given in bytes per second (B/s).
 *
 * \param v Integer literal in bytes per second (B/s).
 * \return \c Bandwidth quantity equal to \c v × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_B_s(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth given in bytes per second (B/s).
 *
 * \param v Floating-point literal in bytes per second (B/s).
 * \return \c Bandwidth quantity approximating \c v × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_B_s(long double v) noexcept {
  return Bandwidth{v * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in kilobytes per second (kB/s).
 *
 * \param v Integer literal in kilobytes per second (kB/s).
 * \return \c Bandwidth quantity equal to \c v × 10³ × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_kB_s(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e3L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in kilobytes per second (kB/s).
 *
 * \param v Floating-point literal in kilobytes per second (kB/s).
 * \return \c Bandwidth quantity approximating \c v × 10³ × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_kB_s(long double v) noexcept {
  return Bandwidth{v * 1.0e3L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in megabytes per second (MB/s).
 *
 * \param v Integer literal in megabytes per second (MB/s).
 * \return \c Bandwidth quantity equal to \c v × 10⁶ × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_MB_s(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e6L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in megabytes per second (MB/s).
 *
 * \param v Floating-point literal in megabytes per second (MB/s).
 * \return \c Bandwidth quantity approximating \c v × 10⁶ × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_MB_s(long double v) noexcept {
  return Bandwidth{v * 1.0e6L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in gigabytes per second (GB/s).
 *
 * \param v Integer literal in gigabytes per second (GB/s).
 * \return \c Bandwidth quantity equal to \c v × 10⁹ × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_GB_s(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e9L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in gigabytes per second (GB/s).
 *
 * \param v Floating-point literal in gigabytes per second (GB/s).
 * \return \c Bandwidth quantity approximating \c v × 10⁹ × 10⁻¹² B/ps.
 */
consteval Bandwidth operator""_GB_s(long double v) noexcept {
  return Bandwidth{v * 1.0e9L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in terabytes per second (TB/s).
 *
 * \param v Integer literal in terabytes per second (TB/s).
 * \return \c Bandwidth quantity equal to \c v B/ps.
 */
consteval Bandwidth operator""_TB_s(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e12L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for bandwidth in terabytes per second (TB/s).
 *
 * \param v Floating-point literal in terabytes per second (TB/s).
 * \return \c Bandwidth quantity approximating \c v B/ps.
 */
consteval Bandwidth operator""_TB_s(long double v) noexcept {
  return Bandwidth{v * 1.0e12L * 1.0e-12L};
}

/*!
 * \brief User-defined literal for integer bandwidth in bytes per second.
 * \param v Integer literal in B/ps.
 * \return \c Bandwidth quantity equal to \c v B/ps.
 */
consteval Bandwidth operator""_B_ps(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v)};
}

/*!
 * \brief User-defined literal for floating-point bandwidth in bytes per second.
 * \param v Floating-point literal in B/ps.
 * \return \c Bandwidth quantity approximating \c v B/ps.
 */
consteval Bandwidth operator""_B_ps(long double v) noexcept {
  return Bandwidth{v};
}

/*!
 * \brief User-defined literal for integer bandwidth in kilobytes per second.
 * \param v Integer literal in kB/ps.
 * \return \c Bandwidth quantity equal to \c v × 10³ B/ps.
 */
consteval Bandwidth operator""_kB_ps(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e3L};
}

/*!
 * \brief User-defined literal for floating-point bandwidth in megabytes per
 * second.
 * \param v Floating-point literal in kB/ps.
 * \return \c Bandwidth quantity approximating \c v × 10³ B/ps.
 */
consteval Bandwidth operator""_kB_ps(long double v) noexcept {
  return Bandwidth{v * 1.0e3L};
}

/*!
 * \brief User-defined literal for integer bandwidth in megabytes per second.
 * \param v Integer literal in MB/ps.
 * \return \c Bandwidth quantity equal to \c v × 10⁶ B/ps.
 */
consteval Bandwidth operator""_MB_ps(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e6L};
}

/*!
 * \brief User-defined literal for floating-point bandwidth in megabytes per
 * second.
 * \param v Floating-point literal in MB/ps.
 * \return \c Bandwidth quantity approximating \c v × 10⁶ B/ps.
 */
consteval Bandwidth operator""_MB_ps(long double v) noexcept {
  return Bandwidth{v * 1.0e6L};
}

/*!
 * \brief User-defined literal for integer bandwidth in gigabytes per second.
 * \param v Integer literal in GB/ps.
 * \return \c Bandwidth quantity equal to \c v × 10⁹ B/ps.
 */
consteval Bandwidth operator""_GB_ps(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e9L};
}

/*!
 * \brief User-defined literal for floating-point bandwidth in gigabytes per
 * second.
 * \param v Floating-point literal in GB/ps.
 * \return \c Bandwidth quantity approximating \c v × 10⁹ B/ps.
 */
consteval Bandwidth operator""_GB_ps(long double v) noexcept {
  return Bandwidth{v * 1.0e9L};
}

/*!
 * \brief User-defined literal for integer bandwidth in terabytes per second.
 *
 * \param v Integer literal in TB/ps.
 * \return \c Bandwidth quantity stored in B/ps.
 */
consteval Bandwidth operator""_TB_ps(std::uint64_t v) noexcept {
  return Bandwidth{static_cast<long double>(v) * 1.0e12L};
}

/*!
 * \brief User-defined literal for floating-point bandwidth in terabytes per
 * second.
 *
 * \param v Floating-point literal in TB/ps.
 * \return \c Bandwidth quantity stored in B/ps.
 */
consteval Bandwidth operator""_TB_ps(long double v) noexcept {
  return Bandwidth{v * 1.0e12L};
}

/*!
 * \brief Computes bandwidth from transferred bytes over elapsed time.
 *
 * The engine measures time in picoseconds. The byte-per-second rate is
 * computed by converting the duration to seconds before forming the
 * ratio. This function provides a safe and explicit construction of a
 * \c Bandwidth quantity from two engine values.
 *
 * \param bytes Byte quantity representing transferred data.
 * \param time  Time quantity expressed in picoseconds.
 *
 * \return Bandwidth quantity in B/ps.
 */
inline Bandwidth MakeBandwidth(Bytes bytes, Time time) noexcept {
  long double B = static_cast<long double>(bytes.value);
  long double ps = static_cast<long double>(time.value);
  return Bandwidth{B / ps};
}
} // namespace ggems::units
