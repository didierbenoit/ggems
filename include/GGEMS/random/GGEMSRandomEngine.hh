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
 * \brief Declares GGEMS random-engine identifiers and conversion helpers.
 *
 * Defines the host-side engine identifiers shared with the OpenCL random subsystem and provides conversion between enum, string, and kernel identifiers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <string>
#include <string_view>
/// \endcond

/*!
 * \namespace ggems::core::random
 * \brief Provides random-engine configuration, state management, and stochastic sampling for GGEMS.
 */
namespace ggems::core::random {

/*!
 * \brief Identifies a random-number engine supported by GGEMS.
 *
 * Numeric values are shared with the OpenCL compile-time engine identifiers.
 */
enum class GGEMSRandomEngine : std::uint8_t {
  /*!
   * \brief JKISS legacy random engine.
   */
  JKISS = 1U,
  /*!
   * \brief PCG32 random engine.
   */
  PCG32 = 2U,
  /*!
   * \brief Philox 4x32 counter-based random engine.
   */
  Philox = 3U
};

/*!
 * \brief Returns the canonical name of a random engine.
 *
 * \param[in] engine Random engine to describe.
 * \return Canonical engine name.
 *
 * \throws ggems::core::GGEMSInternal If \p engine is not a supported value.
 */
auto ToString(GGEMSRandomEngine engine) -> std::string;

/*!
 * \brief Parses a user-facing random-engine name.
 *
 * Engine names are normalized case-insensitively and accept the documented short aliases.
 *
 * \param[in] engine_name Engine name or supported alias.
 * \return Parsed random-engine identifier.
 *
 * \throws ggems::core::GGEMSRecoverable If the engine name is unsupported.
 */
auto ParseRandomEngine(std::string_view engine_name) -> GGEMSRandomEngine;

/*!
 * \brief Converts a host random-engine identifier to its OpenCL engine identifier.
 *
 * \param[in] engine Random engine to convert.
 * \return Numeric identifier passed to OpenCL kernel compilation.
 */
auto ToKernelEngineId(GGEMSRandomEngine engine) noexcept -> std::uint32_t;
} // namespace ggems::core::random
