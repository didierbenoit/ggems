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

#include <format>
#include <string_view>

#include <pybind11/pybind11.h>

#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::python::detail {

// =============================================================================
// =============================================================================

[[noreturn]] inline auto
ThrowQuantityConversionError(ggems::units::UnitConversionError error,
                             std::string_view unit) -> void {
  using ggems::units::UnitConversionError;

  switch (error) {
  case UnitConversionError::UnsupportedUnit:
    throw pybind11::value_error(std::format("Unsupported unit '{}'.", unit));
  case UnitConversionError::NonFinite:
    throw pybind11::value_error("Must be finite.");
  case UnitConversionError::NegativeValue:
    throw pybind11::value_error("Must be positive or zero.");
  case UnitConversionError::OutOfRange:
    throw pybind11::value_error("Too large.");
  case UnitConversionError::InexactConversion:
    throw pybind11::value_error(
      "Cannot be represented in GGEMS canonical units.");
  }

  throw pybind11::value_error("Conversion failed.");
}

// =============================================================================
// =============================================================================

template <ggems::units::QuantityType TargetQuantity>
auto MakeQuantityOrThrow(double value, std::string_view unit)
  -> TargetQuantity {
  auto const conversion = ggems::units::MakeQuantity<TargetQuantity>(
    static_cast<long double>(value), unit);

  if (conversion.has_value()) {
    return *conversion;
  }

  ThrowQuantityConversionError(conversion.error(), unit);
}

// =============================================================================
// =============================================================================

template <ggems::units::QuantityType SourceQuantity>
auto ConvertQuantityToDoubleOrThrow(SourceQuantity quantity,
                                    std::string_view unit) -> double {
  auto const conversion = ggems::units::ConvertTo(quantity, unit);

  if (conversion.has_value()) {
    return static_cast<double>(*conversion);
  }

  ThrowQuantityConversionError(conversion.error(), unit);
}
} // namespace ggems::python::detail
