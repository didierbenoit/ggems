// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Declares strongly typed byte-count units and literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstdint>
#include <string_view>
/// \endcond

#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the byte count unit registry.
 */
struct BytesUnitSet {};

/*!
 * \brief Defines the supported byte count units and their canonical scale factors.
 */
template <> struct UnitRegistry<BytesUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 9U> units{{
      {.symbol = "B",
       .scale = DecimalScale(0)},
      {.symbol = "kB",
       .scale = DecimalScale(3),
       .automatic_display = false},
      {.symbol = "MB",
       .scale = DecimalScale(6),
       .automatic_display = false},
      {.symbol = "GB",
       .scale = DecimalScale(9),
       .automatic_display = false},
      {.symbol = "TB",
       .scale = DecimalScale(12),
       .automatic_display = false},
      {.symbol = "KiB",
       .scale = DecimalScale(0, 1'024ULL)},
      {.symbol = "MiB",
       .scale = DecimalScale(0, 1'048'576ULL)},
      {.symbol = "GiB",
       .scale = DecimalScale(0, 1'073'741'824ULL)},
      {.symbol = "TiB",
       .scale = DecimalScale(0, 1'099'511'627'776ULL)},
  }};
};

/*!
 * \brief Tag type identifying byte-count quantities.
 */
struct BytesTag {};

/*!
 * \brief Defines conversion and formatting traits for Bytes quantities.
 */
template <> struct QuantityTraits<BytesTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = BytesUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  /*!
   * \brief Formatting policy used for human-readable output.
   */
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  /*!
   * \brief Fixed display unit, or an empty string when the policy selects units automatically.
   */
  static constexpr std::string_view fixed_display_unit{};
  /*!
   * \brief Default number of digits after the decimal point for formatted output.
   */
  static constexpr std::int8_t default_precision{7};
};

/*!
 * \brief Strongly typed byte-count quantity stored canonically in bytes.
 */
using Bytes = Quantity<BytesTag, std::uint64_t>;

static_assert(ValidateUnitSet<BytesUnitSet>());
static_assert(ValidateQuantityTraits<BytesTag, std::uint64_t>());

/*!
 * \brief Creates a byte-count quantity from a \c _B literal.
 *
 * \param[in] value Literal value expressed in B.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_B(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "B");
}

/*!
 * \brief Creates a byte-count quantity from a \c _B literal.
 *
 * \param[in] value Literal value expressed in B.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_B(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "B");
}

/*!
 * \brief Creates a byte-count quantity from a \c _kB literal.
 *
 * \param[in] value Literal value expressed in kB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_kB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "kB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _kB literal.
 *
 * \param[in] value Literal value expressed in kB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_kB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "kB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _MB literal.
 *
 * \param[in] value Literal value expressed in MB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_MB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "MB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _MB literal.
 *
 * \param[in] value Literal value expressed in MB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_MB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "MB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _GB literal.
 *
 * \param[in] value Literal value expressed in GB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_GB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "GB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _GB literal.
 *
 * \param[in] value Literal value expressed in GB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_GB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "GB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _TB literal.
 *
 * \param[in] value Literal value expressed in TB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_TB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "TB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _TB literal.
 *
 * \param[in] value Literal value expressed in TB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_TB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "TB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _KiB literal.
 *
 * \param[in] value Literal value expressed in KiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_KiB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "KiB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _KiB literal.
 *
 * \param[in] value Literal value expressed in KiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_KiB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "KiB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _MiB literal.
 *
 * \param[in] value Literal value expressed in MiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_MiB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "MiB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _MiB literal.
 *
 * \param[in] value Literal value expressed in MiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_MiB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "MiB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _GiB literal.
 *
 * \param[in] value Literal value expressed in GiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_GiB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "GiB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _GiB literal.
 *
 * \param[in] value Literal value expressed in GiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_GiB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "GiB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _TiB literal.
 *
 * \param[in] value Literal value expressed in TiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_TiB(unsigned long long value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "TiB");
}

/*!
 * \brief Creates a byte-count quantity from a \c _TiB literal.
 *
 * \param[in] value Literal value expressed in TiB.
 * \return Bytes converted to its canonical GGEMS representation.
 */
consteval auto operator""_TiB(long double value) -> Bytes {
  return detail::MakeLiteralQuantity<Bytes>(value, "TiB");
}
} // namespace ggems::units
