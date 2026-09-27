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
 * \brief Checks integer endpoints for the one-meter diagnostic transport path.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <span>

#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"

namespace ggems::core::transport {
/*! \brief One-meter diagnostic projection length expressed in picometers. */
inline constexpr std::uint64_t k_diagnostic_projection_distance_pm{
  1'000'000'000'000ULL};

/*!
 * \brief Scales one binary32 direction component by one meter.
 *
 * Decomposes binary32 into an integer significand and a power of two, then
 * rounds the exact scaled value half away from zero.
 *
 * \param[in] component Finite direction component to scale.
 * \param[out] displacement_pm Rounded signed displacement in pm, unchanged on
 * failure.
 * \return True if the result fits int64.
 */
[[nodiscard]] auto TryScaleDiagnosticProjectionComponent(
  float component, std::int64_t &displacement_pm) noexcept -> bool;

/*!
 * \brief Adds a signed displacement without overflowing a pm coordinate.
 *
 * \param[in] position_pm Signed starting coordinate in pm.
 * \param[in] displacement_pm Signed displacement in pm.
 * \param[out] endpoint_pm Sum in pm, unchanged on failure.
 * \return True if the sum fits int64.
 */
[[nodiscard]] auto TryAddDiagnosticProjectionDisplacement(
  std::int64_t position_pm, std::int64_t displacement_pm,
  std::int64_t &endpoint_pm) noexcept -> bool;

/*!
 * \brief Checks representable endpoints for nonempty source populations.
 *
 * \param[in] source_records Analytic source records.
 * \param[in] source_ranges Parallel run-primary ranges; zero-count sources are
 * skipped.
 * \throws GGEMSRecoverable If counts differ, an analytic record is invalid, or
 * a diagnostic endpoint/envelope is not representable.
 * \throws GGEMSInternal If the built-in normalized-direction bound cannot be
 * scaled.
 */
auto ValidateDiagnosticTransportSources(
  std::span<sources::GGEMSSourceRecord const> source_records,
  std::span<sources::GGEMSSourceRunRange const> source_ranges) -> void;
} // namespace ggems::core::transport
