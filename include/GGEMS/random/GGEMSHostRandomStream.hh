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
 * \brief Declares the host-side GGEMS random stream.
 *
 * Provides deterministic host sampling from the same JKISS, PCG32, and Philox state definitions used by GGEMS OpenCL kernels.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <variant>

/// \endcond
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomState.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"

namespace ggems::core::random {

/*!
 * \brief Generates deterministic host random values from a GGEMS stream state.
 *
 * The host stream reproduces the state transition rules used by the corresponding OpenCL engine and owns the state of one logical stream identifier.
 */
class GGEMSHostRandomStream {
public:
  /*!
   * \brief Constructs and initializes one host random stream.
   *
   * \param[in] random Random-engine and seed configuration.
   * \param[in] stream_id Logical stream identifier.
   *
   * \throws ggems::core::GGEMSRecoverable If the stream identifier is invalid for the selected engine.
   * \throws ggems::core::GGEMSInternal If the selected engine is unsupported.
   */
  GGEMSHostRandomStream(GGEMSRandom const &random, std::uint64_t stream_id);

  /*!
   * \brief Returns the engine used by this stream.
   *
   * \return Random-engine identifier.
   */
  [[nodiscard]] auto GetEngine() const noexcept -> GGEMSRandomEngine;
  /*!
   * \brief Returns the logical stream identifier.
   *
   * \return Stream identifier used for deterministic state initialization.
   */
  [[nodiscard]] auto GetStreamId() const noexcept -> std::uint64_t;

  /*!
   * \brief Generates the next raw 32-bit random value and advances the stream state.
   *
   * \return Next random 32-bit unsigned integer.
   */
  auto NextUInt32() noexcept -> std::uint32_t;
  /*!
   * \brief Generates a single-precision uniform value in the half-open interval [0, 1).
   *
   * \return Uniform binary32 value.
   */
  auto UniformFloat01() noexcept -> float;
  /*!
   * \brief Generates a double-precision uniform value in the open interval (0, 1).
   *
   * \return Uniform binary64 value strictly greater than zero and strictly less than one.
   */
  auto UniformDoubleOpen01() noexcept -> double;

private:
  /*!
   * \brief Random engine selected when the stream was created.
   */
  GGEMSRandomEngine engine_;
  /*!
   * \brief Logical identifier of this deterministic stream.
   */
  std::uint64_t stream_id_;
  /*!
   * \brief Mutable engine-specific stream state.
   */
  std::variant<GGEMSJKissState, GGEMSPCG32State, GGEMSPhiloxState> state_;
};

} // namespace ggems::core::random
