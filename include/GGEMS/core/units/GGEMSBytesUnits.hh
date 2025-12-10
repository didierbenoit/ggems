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
 * \file GGEMSBytesUnits.hh
 * \brief Byte-based information units for GGEMS.
 *
 * This header defines the \c Bytes quantity and its user-defined literals,
 * enabling type-safe manipulation of information sizes in engine base units
 * (bytes). A helper function provides human-readable formatting using
 * binary scaling (kB, MB, GB, TB).
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/core/units/GGEMSQuantity.hh"

/// \cond
#include <array>
/// \endcond

namespace ggems::units {
/*!
 * \brief Information quantity expressed in bytes.
 *
 * This alias binds the \c InfoBytesDim dimension to an unsigned 64-bit
 * representation. Stored values always correspond to engine base units
 * (bytes). Higher-level conversions may represent values in kB, MB, GB or TB.
 */
using Bytes = Quantity<InfoBytesDim, std::uint64_t>;

/*!
 * \brief Converts a byte quantity to a human-readable UTF-8 string.
 *
 * The function selects an appropriate binary scale (kB, MB, GB, TB) based on
 * the stored value. Output formatting uses fixed precision and optional
 * minimum field width, producing an aligned and readable textual
 * representation suitable for logs or UI components.
 *
 * \param b         Quantity expressed in bytes.
 * \param precision Number of digits after the decimal point.
 * \param width     Minimum formatted width; if negative, no constraint.
 *
 * \return UTF-8 encoded string representing the scaled quantity.
 */
inline std::string HumanReadable(Bytes const &b, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double v = static_cast<long double>(b.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{
      {{1099511627776.0L, " TB", 1099511627776.0L},
       {1073741824.0L, " GB", 1073741824.0L},
       {1048576.0L, " MB", 1048576.0L},
       {1024.0L, " kB", 1024.0L},
       {0.0L, " B", 1.0L}}};

  for (auto const &u : units) {
    if (v >= u.threshold) {

      long double scaled = v / u.scale;

      // Buffer local pour fabriquer le format dynamiquement
      std::string fmt;

      if (width < 0) {
        fmt = std::format("{{:.{}f}}{}", precision, u.suffix);
      } else {
        fmt = std::format("{{:{}.{}f}}{}", width, precision, u.suffix);
      }

      return std::vformat(fmt, std::make_format_args(scaled));
    }
  }

  return std::format("{:.{}f} B", v, precision);
}

/*!
 * \brief Constructs a \c Bytes quantity from an integer literal in bytes.
 * \param v Integer literal in bytes.
 * \return \c Bytes quantity equal to \c v B.
 */
consteval Bytes operator""_B(unsigned long long v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief Constructs a \c Bytes quantity from a floating-point literal in bytes.
 * \param v Floating-point literal in bytes.
 * \return \c Bytes quantity approximating \c v B (fraction truncated).
 */
consteval Bytes operator""_B(long double v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief Constructs a \c Bytes quantity from an integer literal in kibibytes.
 * \param v Integer literal in KiB.
 * \return \c Bytes quantity equal to \c v × 1024 B.
 */
consteval Bytes operator""_kB(unsigned long long v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v) * 1'024ULL};
}

/*!
 * \brief Constructs a \c Bytes quantity from a floating-point literal in
 * kibibytes.
 * \param v Floating-point literal in KiB.
 * \return \c Bytes quantity approximating \c v × 1024 B.
 */
consteval Bytes operator""_kB(long double v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v * 1'024.0L)};
}

/*!
 * \brief Constructs a \c Bytes quantity from an integer literal in mebibytes.
 * \param v Integer literal in MiB.
 * \return \c Bytes quantity equal to \c v × 1024² B.
 */
consteval Bytes operator""_MB(unsigned long long v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v) * 1'024ULL * 1'024ULL};
}

/*!
 * \brief Constructs a \c Bytes quantity from a floating-point literal in
 * mebibytes.
 * \param v Floating-point literal in MiB.
 * \return \c Bytes quantity approximating \c v × 1024² B.
 */
consteval Bytes operator""_MB(long double v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v * 1'024.0L * 1'024.0L)};
}

/*!
 * \brief Constructs a \c Bytes quantity from an integer literal in gibibytes.
 * \param v Integer literal in GiB.
 * \return \c Bytes quantity equal to \c v × 1024³ B.
 */
consteval Bytes operator""_GB(unsigned long long v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v) * 1'024ULL * 1'024ULL * 1'024ULL};
}

/*!
 * \brief Constructs a \c Bytes quantity from a floating-point literal in
 * gibibytes.
 * \param v Floating-point literal in GiB.
 * \return \c Bytes quantity approximating \c v × 1024³ B.
 */
consteval Bytes operator""_GB(long double v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v * 1'024.0L * 1'024.0L * 1'024.0L)};
}

/*!
 * \brief Constructs a \c Bytes quantity from an integer literal in tebibytes.
 * \param v Integer literal in TiB.
 * \return \c Bytes quantity equal to \c v × 1024⁴ B.
 */
consteval Bytes operator""_TB(unsigned long long v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v) * 1'024ULL * 1'024ULL * 1'024ULL *
               1'024ULL};
}

/*!
 * \brief Constructs a \c Bytes quantity from a floating-point literal in
 * tebibytes.
 * \param v Floating-point literal in TiB.
 * \return \c Bytes quantity approximating \c v × 1024⁴ B.
 */
consteval Bytes operator""_TB(long double v) noexcept {
  return Bytes{static_cast<std::uint64_t>(v * 1'024.0L * 1'024.0L * 1'024.0L *
                                          1'024.0L)};
}
} // namespace ggems::units
