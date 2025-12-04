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
 * \file GGEMSDensityUnits.hh
 * \brief Density quantity alias and human-readable conversion helpers.
 *
 * This header defines the \c Density quantity used by GGEMS to represent
 * mass density in engine base units (picograms per cubic picometre),
 * together with a formatting helper and a user-defined literal for the
 * common representation in g/cm³.
 *
 * Internally, the stored value corresponds to pg/pm³. High-level usage
 * may express densities as g/cm³, automatically converted at compile
 * time through the user-defined literal.
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
 * \brief Density quantity expressed in pg/pm³.
 *
 * The alias binds the \c DensityDim dimension (M·L⁻³) to a
 * \c long double representation. Stored values correspond to
 * picograms per cubic picometre. User-facing code may instead use
 * g/cm³, converted transparently at compile time.
 */
using Density = Quantity<DensityDim, long double>;

/*!
 * \brief Formats a density quantity into a human-readable string.
 *
 * If the magnitude corresponds to realistic tissue or material
 * densities, the output is expressed in g/cm³. Otherwise, the raw
 * stored pg/pm³ value is shown.
 *
 * \param d         Density quantity in pg/pm³.
 * \param precision Number of fractional digits.
 * \param width     Optional minimum field width. If negative, no width
 *                  constraint applies.
 *
 * \return UTF-8 formatted string representing the density.
 */
inline std::string HumanReadable(Density const &d, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  // Pull internal stored value: pg/pm³
  long double pg_per_pm3 = d.value;

  // Conversion factor from pg/pm³ to g/cm³:
  // 1 g = 1e12 pg
  // 1 cm³ = 1e30 pm³
  // => 1 g/cm³ = 1e-18 pg/pm³
  long double g_per_cm3 = pg_per_pm3 / 1.0e-18L;

  // Typical human-readable output in g/cm³
  std::string fmt;
  if (width < 0) {
    fmt = std::format("{{:.{}f}} g/cm3", precision);
  } else {
    fmt = std::format("{{:{}.{}f}} g/cm3", width, precision);
  }

  return std::vformat(fmt, std::make_format_args(g_per_cm3));
}

/*!
 * \brief User-defined literal for density in g/cm³.
 *
 * The engine stores density in pg/pm³. The conversion factor is:
 * \f[
 *   \rho_{\text{stored}} = \rho_{\text{g/cm³}} \times 10^{-18}
 * \f]
 *
 * \param v Floating-point literal representing density in g/cm³.
 * \return \c Density quantity expressed in pg/pm³ internally.
 */
consteval Density operator""_g_cm3(long double v) noexcept {
  return Density{v * 1.0e-18L};
}

/*!
 * \brief User-defined literal for density in g/cm³ (integer form).
 *
 * \param v Integer literal in g/cm³.
 * \return \c Density quantity expressed in pg/pm³ internally.
 */
consteval Density operator""_g_cm3(std::uint64_t v) noexcept {
  return Density{static_cast<long double>(v) * 1.0e-18L};
}
} // namespace ggems::units
