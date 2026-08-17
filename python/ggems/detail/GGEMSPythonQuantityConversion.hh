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
 * \brief Internal helpers for converting GGEMS quantities in Python bindings.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <format>
#include <string_view>

#include <pybind11/pybind11.h>
/// \endcond

#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::python::detail {

/*!
 * \brief Describes Python-facing error messages for a quantity conversion.
 */
struct QuantityConversionContext {
  /*!
   * \brief Human-readable quantity name used in diagnostics.
   */
  std::string_view quantity_name;
  /*!
   * \brief Subject used when reporting an unsupported unit.
   */
  std::string_view unsupported_unit_subject;
};

/*!
 * \brief Converts a GGEMS unit-conversion error into a Python exception.
 *
 * \param[in] error Conversion error to report.
 * \param[in] context Python-facing wording used to build the diagnostic.
 * \param[in] unit Unit symbol associated with the failed conversion.
 *
 * \throws pybind11::value_error Always throws with a conversion-specific
 *         diagnostic.
 */
[[noreturn]] inline auto
ThrowQuantityConversionError(ggems::units::UnitConversionError error,
                             QuantityConversionContext context,
                             std::string_view unit) -> void {
  using ggems::units::UnitConversionError;

  switch (error) {
  case UnitConversionError::UnsupportedUnit:
    throw pybind11::value_error(std::format(
        "Unsupported {} unit '{}'.", context.unsupported_unit_subject, unit));
  case UnitConversionError::NonFinite:
    throw pybind11::value_error(
        std::format("{} must be finite.", context.quantity_name));
  case UnitConversionError::NegativeValue:
    throw pybind11::value_error(
        std::format("{} must be positive or zero.", context.quantity_name));
  case UnitConversionError::OutOfRange:
    throw pybind11::value_error(
        std::format("{} is too large.", context.quantity_name));
  case UnitConversionError::InexactConversion:
    throw pybind11::value_error(
        std::format("{} cannot be represented in GGEMS canonical units.",
                    context.quantity_name));
  }

  throw pybind11::value_error(
      std::format("{} conversion failed.", context.quantity_name));
}

/*!
 * \brief Creates a GGEMS quantity from a Python floating-point value.
 *
 * \tparam TargetQuantity GGEMS quantity type to construct.
 * \param[in] value Numeric value supplied by Python.
 * \param[in] unit Unit symbol associated with \p value.
 * \param[in] context Python-facing wording used for conversion diagnostics.
 * \return Converted GGEMS quantity.
 *
 * \throws pybind11::value_error If the value cannot be converted to the
 *         requested GGEMS quantity.
 */
template <ggems::units::QuantityType TargetQuantity>
auto MakeQuantityOrThrow(double value, std::string_view unit,
                         QuantityConversionContext context) -> TargetQuantity {
  auto const conversion = ggems::units::MakeQuantity<TargetQuantity>(
      static_cast<long double>(value), unit);

  if (conversion.has_value()) {
    return *conversion;
  }

  ThrowQuantityConversionError(conversion.error(), context, unit);
}

/*!
 * \brief Converts a GGEMS quantity to a Python floating-point value.
 *
 * \tparam SourceQuantity GGEMS quantity type to convert.
 * \param[in] quantity Quantity stored in GGEMS canonical units.
 * \param[in] unit Target unit symbol requested by Python.
 * \param[in] context Python-facing wording used for conversion diagnostics.
 * \return Converted value as a double.
 *
 * \throws pybind11::value_error If the quantity cannot be converted to the
 *         requested unit.
 */
template <ggems::units::QuantityType SourceQuantity>
auto ConvertQuantityToDoubleOrThrow(SourceQuantity quantity,
                                    std::string_view unit,
                                    QuantityConversionContext context)
    -> double {
  auto const conversion = ggems::units::ConvertTo(quantity, unit);

  if (conversion.has_value()) {
    return static_cast<double>(*conversion);
  }

  ThrowQuantityConversionError(conversion.error(), context, unit);
}
} // namespace ggems::python::detail
