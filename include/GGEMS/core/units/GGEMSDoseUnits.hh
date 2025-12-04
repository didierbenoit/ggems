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
 * \file GGEMSDoseUnits.hh
 * \brief Dose quantity alias and human-readable formatting helpers.
 *
 * This header defines the \c Dose quantity used by GGEMS to represent
 * absorbed radiation dose in engine base units (meV/pg). The conversion
 * to SI units is applied when formatting or interpreting user-level
 * constants.
 *
 * Internally, the stored value corresponds to the absorbed energy per
 * unit mass. One gray (Gy) is equivalent to 1 joule per kilogram; this
 * maps to 6.241509074e6 meV/pg in base engine representation.
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
 * \brief Dose quantity expressed in engine base units (meV/pg).
 *
 * The base representation uses an unsigned 64-bit integer storing
 * milli-electron-volts per picogram. Higher-level views (Gy, mGy, µGy)
 * are handled via literal operators and the \c HumanReadable formatter.
 *
 * Storage in base units ensures deterministic transport computations
 * within the simulation engine, while still supporting readable
 * rendering at interfaces or diagnostics.
 */
using Dose = Quantity<DoseDim, std::uint64_t>;

/*!
 * \brief Converts a dose quantity to a human-readable string.
 *
 * The internal \c value is interpreted as meV/pg and converted to SI
 * gray units using the factor:
 * \f$
 *    1\ \mathrm{Gy} = 6.241509074 \times 10^6\ \mathrm{meV/pg}
 * \f$
 *
 * Depending on magnitude, the value is expressed in Gy, mGy or µGy,
 * selecting the most suitable scale automatically.
 *
 * \param d         Dose quantity in base units (meV/pg).
 * \param precision Number of digits after the decimal point.
 * \param width     Minimum field width; if negative, no width constraint.
 * \return UTF-8 encoded string describing the dose with its unit.
 */
inline std::string HumanReadable(Dose const &d, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double v = static_cast<long double>(d.value) * 1.602176634e-7L;

  struct Scale {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Scale, 3> units{{{1.0L, " Gy", 1.0L},
                                               {1.0e-3L, " mGy", 1.0e-3L},
                                               {1.0e-6L, " uGy", 1.0e-6L}}};

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

  return std::format("{:.{}f} uGy", v / 1.0e-6L, precision);
}

/*!
 * \brief Literal for dose in gray (Gy).
 *
 * The literal performs the conversion
 * \f$v \times 6.241509074\cdot 10^6\ \mathrm{meV/pg}\f$
 * to match the engine storage convention.
 *
 * \param v Floating-point value in Gy.
 * \return \c Dose quantity in base units (meV/pg).
 */
consteval Dose operator""_Gy(long double v) noexcept {
  return Dose{static_cast<std::uint64_t>(v * 6.241509074e6L)};
}

/*!
 * \brief Literal for dose in milligray (mGy).
 *
 * \param v Floating-point value in mGy.
 * \return \c Dose quantity expressed in base units (meV/pg).
 */
consteval Dose operator""_mGy(long double v) noexcept {
  return Dose{static_cast<std::uint64_t>(v * 6.241509074e3L)};
}

/*!
 * \brief Literal for dose in microgray (µGy).
 *
 * \param v Floating-point value in µGy.
 * \return \c Dose quantity expressed in base units (meV/pg).
 */
consteval Dose operator""_uGy(long double v) noexcept {
  return Dose{static_cast<std::uint64_t>(v * 6.241509074L)};
}
} // namespace ggems::units
