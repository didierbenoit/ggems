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
 * \file GGEMSEnergyUnits.hh
 * \brief Energy quantity alias and formatting utilities.
 *
 * This header defines the \c Energy quantity used by GGEMS to represent
 * physical energies in engine base units (milli-electronvolt). It also
 * provides a helper for human-readable formatting and a set of user-defined
 * literals covering the usual energy scales (eV, keV, MeV, GeV, TeV).
 *
 * Stored values are integers in meV. High-level interfaces may expose
 * energies in eV-based magnitudes when printing results.
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
 * \brief Energy quantity expressed in milli-electronvolts.
 *
 * The alias binds the \c EnergyDim dimension to a \c uint64_t representation.
 * All values represent energies in base engine units (meV), ensuring an
 * integer-only physical core.
 */
using Energy = Quantity<EnergyDim, std::uint64_t>;

/*!
 * \brief Converts an energy value into a human-readable UTF-8 string.
 *
 * Depending on magnitude, values are formatted as meV, eV, keV, MeV, GeV
 * or TeV. Scales are selected automatically.
 *
 * \param e         Energy quantity in meV.
 * \param precision Number of fractional digits.
 * \param width     Optional minimum field width. If negative, no width
 *                  constraint applies.
 *
 * \return UTF-8 string representing the energy with its unit.
 */
inline std::string HumanReadable(Energy const &e, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(e.value);

  struct Scale {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Scale, 6> units{{{1.0e15L, " TeV", 1.0e15L},
                                               {1.0e12L, " GeV", 1.0e12L},
                                               {1.0e9L, " MeV", 1.0e9L},
                                               {1.0e6L, " keV", 1.0e6L},
                                               {1.0e3L, " eV", 1.0e3L},
                                               {0.0L, " meV", 1.0L}}};

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

  return std::format("{:.{}f} meV", v, precision);
}

/*!
 * \brief User-defined literal for energy in meV (integer).
 * \param v Integer literal in meV.
 * \return \c Energy quantity equal to \c v meV.
 */
consteval Energy operator""_meV(std::uint64_t v) noexcept { return Energy{v}; }

/*!
 * \brief User-defined literal for energy in meV (floating-point).
 * Fractional parts are rounded by cast.
 *
 * \param v Floating literal in meV.
 * \return \c Energy quantity approximating \c v meV.
 */
consteval Energy operator""_meV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v)};
}

/*!
 * \brief User-defined literal for energy in eV.
 * \param v Integer literal in eV.
 * \return Stored as \c v × 10³ meV.
 */
consteval Energy operator""_eV(std::uint64_t v) noexcept {
  return Energy{v * 1000ULL};
}

/*!
 * \brief User-defined literal for energy in eV (floating-point).
 * \param v Floating literal in eV.
 * \return Approximated as \c v × 10³ meV.
 */
consteval Energy operator""_eV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e3L)};
}

/*!
 * \brief User-defined literal for energy in keV.
 * \param v Integer literal in keV.
 * \return Stored as \c v × 10⁶ meV.
 */
consteval Energy operator""_keV(std::uint64_t v) noexcept {
  return Energy{v * 1'000'000ULL};
}

/*!
 * \brief User-defined literal for energy in keV (floating-point).
 * \param v Floating literal in keV.
 * \return Approximated as \c v × 10⁶ meV.
 */
consteval Energy operator""_keV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e6L)};
}

/*!
 * \brief User-defined literal for energy in MeV.
 * \param v Integer literal in MeV.
 * \return Stored as \c v × 10⁹ meV.
 */
consteval Energy operator""_MeV(std::uint64_t v) noexcept {
  return Energy{v * 1'000'000'000ULL};
}

/*!
 * \brief User-defined literal for energy in MeV (floating-point).
 * \param v Floating literal in MeV.
 * \return Approximated as \c v × 10⁹ meV.
 */
consteval Energy operator""_MeV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e9L)};
}

/*!
 * \brief User-defined literal for energy in GeV.
 * \param v Integer literal in GeV.
 * \return Stored as \c v × 10¹² meV.
 */
consteval Energy operator""_GeV(std::uint64_t v) noexcept {
  return Energy{v * 1'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for energy in GeV (floating-point).
 * \param v Floating literal in GeV.
 * \return Approximated as \c v × 10¹² meV.
 */
consteval Energy operator""_GeV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e12L)};
}

/*!
 * \brief User-defined literal for energy in TeV.
 * \param v Integer literal in TeV.
 * \return Stored as \c v × 10¹⁵ meV.
 */
consteval Energy operator""_TeV(std::uint64_t v) noexcept {
  return Energy{v * 1'000'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for energy in TeV (floating-point).
 * \param v Floating literal in TeV.
 * \return Approximated as \c v × 10¹⁵ meV.
 */
consteval Energy operator""_TeV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e15L)};
}
} // namespace ggems::units
