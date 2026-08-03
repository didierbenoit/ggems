#pragma once

#include <format>
#include <string_view>

#include <pybind11/pybind11.h>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::python::detail {

struct QuantityConversionContext {
  std::string_view quantity_name;
  std::string_view unsupported_unit_subject;
};

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

template <ggems::units::QuantityType TargetQuantity>
auto MakeQuantityOrThrow(double value, std::string_view unit,
                         QuantityConversionContext context) -> TargetQuantity {
  auto const conversion = ggems::units::TryMakeQuantity<TargetQuantity>(
      static_cast<long double>(value), unit);

  if (conversion.has_value()) {
    return *conversion;
  }

  ThrowQuantityConversionError(conversion.error(), context, unit);
}

template <ggems::units::QuantityType SourceQuantity>
auto ConvertQuantityToDoubleOrThrow(SourceQuantity quantity,
                                    std::string_view unit,
                                    QuantityConversionContext context)
    -> double {
  auto const conversion = ggems::units::TryConvertTo(quantity, unit);

  if (conversion.has_value()) {
    return static_cast<double>(*conversion);
  }

  ThrowQuantityConversionError(conversion.error(), context, unit);
}
} // namespace ggems::python::detail
