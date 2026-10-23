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
 * \brief Validates sources for the one-meter diagnostic transport path.
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
/*! \brief One-meter diagnostic distance in canonical host picometers. */
inline constexpr std::uint64_t k_diagnostic_projection_distance_pm{
  1'000'000'000'000ULL};

/*!
 * \brief Validates canonical source configurations with nonempty run ranges.
 *
 * Device movement remains FP32; integer output representability belongs to
 * the Observer publication boundary.
 * \param[in] source_records Canonical analytic source records.
 * \param[in] source_ranges Parallel run-primary ranges; zero-count sources are
 * skipped.
 * \throws GGEMSRecoverable If counts differ or an analytic record is invalid.
 */
auto ValidateDiagnosticTransportSources(
  std::span<sources::GGEMSSourceRecord const> source_records,
  std::span<sources::GGEMSSourceRunRange const> source_ranges) -> void;

} // namespace ggems::core::transport
