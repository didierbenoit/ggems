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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

namespace ggems::test::fp32 {

// Independent binary64 references with absolute FP32 arithmetic error bounds.
// Inputs are already uploaded binary32 values; authoring quantization is
// separate. These helpers cover finite, well-conditioned test expressions, not
// production navigation. Scalar.error has the same physical unit as
// Scalar.value.
struct Scalar {
  double value{};
  double error{};
};

[[nodiscard]] inline auto FromFloat(float value) -> Scalar {
  return {.value = static_cast<double>(value), .error = 0.0};
}

[[nodiscard]] inline auto Ulp(double value) -> double {
  auto const packed = static_cast<float>(std::abs(value));
  auto const infinity = std::numeric_limits<float>::infinity();
  return std::max(
    static_cast<double>(std::nextafter(packed, infinity)) - packed,
    static_cast<double>(packed) - std::nextafter(packed, -infinity));
}

[[nodiscard]] inline auto Gamma(unsigned operations) -> double {
  double const accumulated = static_cast<double>(operations) *
                             std::numeric_limits<float>::epsilon() / 2.0;
  return accumulated / (1.0 - accumulated);
}

[[nodiscard]] inline auto RoundoffBound(double magnitude, unsigned operations)
  -> double {
  return (Gamma(operations) * std::abs(magnitude)) +
         (static_cast<double>(operations) *
          std::numeric_limits<float>::denorm_min() / 2.0);
}

[[nodiscard]] inline auto Add(Scalar left, Scalar right) -> Scalar {
  double const value = left.value + right.value;
  double const input_error = left.error + right.error;
  return {
    .value = value,
    .error = input_error + (0.5 * Ulp(std::abs(value) + input_error)),
  };
}

[[nodiscard]] inline auto Subtract(Scalar left, Scalar right) -> Scalar {
  return Add(left, {.value = -right.value, .error = right.error});
}

[[nodiscard]] inline auto Multiply(Scalar left, Scalar right) -> Scalar {
  double const value = left.value * right.value;
  double const input_error = (std::abs(left.value) * right.error) +
                             (std::abs(right.value) * left.error) +
                             (left.error * right.error);
  return {
    .value = value,
    .error = input_error + (0.5 * Ulp(std::abs(value) + input_error)),
  };
}

// OpenCL C, full profile, table "ULP Values for Single-Precision Built-in
// Math Functions": division <= 2.5 ULP; sqrt <= 3 ULP without the optional
// correctly-rounded-divide-sqrt build option. +, -, * are correctly rounded.
// https://registry.khronos.org/OpenCL/specs/unified/html/OpenCL_C.html
[[nodiscard]] inline auto Divide(Scalar numerator, Scalar denominator)
  -> Scalar {
  double const value = numerator.value / denominator.value;
  double const denominator_margin =
    std::abs(denominator.value) - denominator.error;
  if (denominator_margin <= 0.0) {
    return {
      .value = value,
      .error = std::numeric_limits<double>::infinity(),
    };
  }
  double const input_error =
    (numerator.error + (std::abs(value) * denominator.error)) /
    denominator_margin;
  return {
    .value = value,
    .error = input_error + (2.5 * Ulp(std::abs(value) + input_error)),
  };
}

[[nodiscard]] inline auto Sqrt(Scalar argument) -> Scalar {
  double const value = std::sqrt(argument.value);
  double const low = std::sqrt(std::max(0.0, argument.value - argument.error));
  double const high = std::sqrt(argument.value + argument.error);
  double const input_error = std::max(value - low, high - value);
  return {
    .value = value,
    .error = input_error + (3.0 * Ulp(high)),
  };
}

// A non-finite/ill-conditioned bound is a failed test, never an infinite pass
// tolerance. Add 0.5 pm only for canonical integer observer export.
inline auto ExpectWithin(double actual, Scalar reference,
                         double export_error = 0.0) -> void {
  ASSERT_TRUE(std::isfinite(actual));
  ASSERT_TRUE(std::isfinite(reference.value));
  ASSERT_TRUE(std::isfinite(reference.error));
  ASSERT_GE(reference.error, 0.0);
  ASSERT_TRUE(std::isfinite(export_error));
  ASSERT_GE(export_error, 0.0);
  EXPECT_LE(std::abs(actual - reference.value), reference.error + export_error)
    << "actual=" << actual << "; represented reference=" << reference.value
    << "; error budget=" << reference.error
    << "; observer export budget=" << export_error;
}

} // namespace ggems::test::fp32
