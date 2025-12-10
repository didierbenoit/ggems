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
 * \file GGEMSMassUnits.hh
 * \brief Mass quantity alias and user-defined literals for picogram storage.
 *
 * This header defines the \c Mass quantity used by GGEMS to represent
 * masses in engine base units (picograms), together with a compact
 * human-readable formatter and a set of user-defined literals for
 * expressing mass constants at compile time.
 *
 * All stored values are expressed in picograms. Higher-level interfaces
 * may display grams or kilograms depending on magnitude.
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
 * \brief Mass quantity expressed in picograms.
 *
 * This alias binds the \c MassDim dimension to an unsigned 64-bit
 * representation. All stored values are expressed in base engine units
 * (picograms).
 */
using Mass = Quantity<MassDim, std::uint64_t>;

/*!
 * \brief Converts a mass quantity to a human-readable string.
 *
 * Depending on its magnitude, the value is formatted in pg, ng, µg, mg,
 * g or kg. The stored integer representation is promoted to
 * \c long double for display.
 *
 * \param m         Mass quantity in base units (picograms).
 * \param precision Number of digits after the decimal point.
 * \param width     Optional minimum field width; if negative, no width
 *                  constraint is applied.
 *
 * \return UTF-8 encoded string describing the mass with its unit.
 */
inline std::string HumanReadable(Mass const &m, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double v = static_cast<long double>(m.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 6> units{{{1.0e15L, " kg", 1.0e15L},
                                              {1.0e12L, " g", 1.0e12L},
                                              {1.0e9L, " mg", 1.0e9L},
                                              {1.0e6L, " ug", 1.0e6L},
                                              {1.0e3L, " ng", 1.0e3L},
                                              {0.0L, " pg", 1.0L}}};

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

  return std::format("{:.{}f} pg", v, precision);
}

/*!
 * \brief User-defined literal for mass in picograms (integer).
 *
 * \param v Integer literal in picograms.
 * \return \c Mass quantity equal to \c v pg.
 */
consteval Mass operator""_pg(unsigned long long v) noexcept {
  return Mass{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief User-defined literal for mass in picograms (floating-point).
 *
 * \param v Floating-point literal in picograms.
 * \return \c Mass quantity approximating \c v pg.
 */
consteval Mass operator""_pg(long double v) noexcept {
  return Mass{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief User-defined literal for mass in nanograms (1 ng = 10³ pg, integer).
 *
 * \param v Integer literal in nanograms.
 * \return \c Mass quantity equal to \c v × 10³ pg.
 */
consteval Mass operator""_ng(unsigned long long v) noexcept {
  return Mass{static_cast<std::uint64_t>(v) * 1'000ULL};
}

/*!
 * \brief User-defined literal for mass in nanograms (floating-point).
 *
 * \param v Floating-point literal in nanograms.
 * \return \c Mass quantity approximating \c v × 10³ pg.
 */
consteval Mass operator""_ng(long double v) noexcept {
  return Mass{static_cast<std::uint64_t>(v * 1.0e3L)};
}

/*!
 * \brief User-defined literal for mass in micrograms (1 µg = 10⁶ pg, integer).
 *
 * \param v Integer literal in micrograms.
 * \return \c Mass quantity equal to \c v × 10⁶ pg.
 */
consteval Mass operator""_ug(unsigned long long v) noexcept {
  return Mass{static_cast<std::uint64_t>(v) * 1'000'000ULL};
}

/*!
 * \brief User-defined literal for mass in micrograms (floating-point).
 *
 * \param v Floating-point literal in micrograms.
 * \return \c Mass quantity approximating \c v × 10⁶ pg.
 */
consteval Mass operator""_ug(long double v) noexcept {
  return Mass{static_cast<std::uint64_t>(v * 1.0e6L)};
}

/*!
 * \brief User-defined literal for mass in milligrams (1 mg = 10⁹ pg, integer).
 *
 * \param v Integer literal in milligrams.
 * \return \c Mass quantity equal to \c v × 10⁹ pg.
 */
consteval Mass operator""_mg(unsigned long long v) noexcept {
  return Mass{static_cast<std::uint64_t>(v) * 1'000'000'000ULL};
}

/*!
 * \brief User-defined literal for mass in milligrams (floating-point).
 *
 * \param v Floating-point literal in milligrams.
 * \return \c Mass quantity approximating \c v × 10⁹ pg.
 */
consteval Mass operator""_mg(long double v) noexcept {
  return Mass{static_cast<std::uint64_t>(v * 1.0e9L)};
}

/*!
 * \brief User-defined literal for mass in grams (1 g = 10¹² pg, integer).
 *
 * \param v Integer literal in grams.
 * \return \c Mass quantity equal to \c v × 10¹² pg.
 */
consteval Mass operator""_g(unsigned long long v) noexcept {
  return Mass{static_cast<std::uint64_t>(v) * 1'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for mass in grams (floating-point).
 *
 * \param v Floating-point literal in grams.
 * \return \c Mass quantity approximating \c v × 10¹² pg.
 */
consteval Mass operator""_g(long double v) noexcept {
  return Mass{static_cast<std::uint64_t>(v * 1.0e12L)};
}

/*!
 * \brief User-defined literal for mass in kilograms (1 kg = 10¹⁵ pg, integer).
 *
 * \param v Integer literal in kilograms.
 * \return \c Mass quantity equal to \c v × 10¹⁵ pg.
 */
consteval Mass operator""_kg(unsigned long long v) noexcept {
  return Mass{static_cast<std::uint64_t>(v) * 1'000'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for mass in kilograms (floating-point).
 *
 * \param v Floating-point literal in kilograms.
 * \return \c Mass quantity approximating \c v × 10¹⁵ pg.
 */
consteval Mass operator""_kg(long double v) noexcept {
  return Mass{static_cast<std::uint64_t>(v * 1.0e15L)};
}
} // namespace ggems::units
