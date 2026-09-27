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
 * \brief Formats source configuration and captured run populations for
 * diagnostics.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"

namespace ggems::core::sources {
class GGEMSSource;
class GGEMSSourceRunSnapshot;

/*!
 * \brief Formats source population, shape, direction, and energy settings.
 *
 * \param[in] source Source configuration to describe.
 * \return Owned human-readable diagnostic string.
 * \throws GGEMSRecoverable If source-record construction rejects the
 * configuration.
 */
[[nodiscard]] auto DescribeSource(GGEMSSource const &source) -> std::string;

/*!
 * \brief Formats source population, shape, direction, and energy settings.
 *
 * The two-argument record overload requires a monoenergetic source.
 *
 * \param[in] record Source pose and analytic birth laws.
 * \param[in] primary_count Explicit primary count to display.
 * \return Owned human-readable diagnostic string.
 * \throws GGEMSInternal If a record energy descriptor or table range is
 * inconsistent.
 */
[[nodiscard]] auto DescribeSource(GGEMSSourceRecord const &record,
                                  std::uint64_t primary_count) -> std::string;

/*!
 * \brief Formats source population, shape, direction, and energy settings.
 *
 * \param[in] record Source pose and analytic birth laws.
 * \param[in] primary_count Explicit primary count to display.
 * \param[in] energy_record Descriptor referring into energy_values.
 * \param[in] energy_values Borrowed concatenated canonical energies in
 * micro-eV.
 * \return Owned human-readable diagnostic string.
 * \throws GGEMSInternal If a record energy descriptor or table range is
 * inconsistent.
 */
[[nodiscard]] auto
DescribeSource(GGEMSSourceRecord const &record, std::uint64_t primary_count,
               GGEMSEnergyDistributionRecord const &energy_record,
               std::span<std::uint64_t const> energy_values) -> std::string;

/*!
 * \brief Formats one captured source slot and its run-population interval.
 *
 * \param[in] source_index Valid source slot within snapshot.
 * \param[in] snapshot Owning run snapshot to describe.
 * \return Owned diagnostic string.
 * \throws GGEMSInternal If snapshot indexing or energy description is
 * inconsistent.
 */
[[nodiscard]] auto DescribeSourceRunSlot(std::size_t source_index,
                                         GGEMSSourceRunSnapshot const &snapshot)
  -> std::string;

/*!
 * \brief Formats one captured source slot and its run-population interval.
 *
 * \param[in] source_index Source slot index to display.
 * \param[in] record Captured source record.
 * \param[in] range Captured run-wide primary interval.
 * \return Owned diagnostic string.
 * \throws GGEMSInternal If snapshot indexing or energy description is
 * inconsistent.
 */
[[nodiscard]] auto DescribeSourceRunSlot(std::size_t source_index,
                                         GGEMSSourceRecord const &record,
                                         GGEMSSourceRunRange const &range)
  -> std::string;

/*!
 * \brief Formats one captured source slot and its run-population interval.
 *
 * \param[in] source_index Source slot index to display.
 * \param[in] record Captured source record.
 * \param[in] range Captured run-wide primary interval.
 * \param[in] energy_record Descriptor referring into energy_values.
 * \param[in] energy_values Borrowed concatenated canonical energies in
 * micro-eV.
 * \return Owned diagnostic string.
 * \throws GGEMSInternal If snapshot indexing or energy description is
 * inconsistent.
 */
[[nodiscard]] auto
DescribeSourceRunSlot(std::size_t source_index, GGEMSSourceRecord const &record,
                      GGEMSSourceRunRange const &range,
                      GGEMSEnergyDistributionRecord const &energy_record,
                      std::span<std::uint64_t const> energy_values)
  -> std::string;
} // namespace ggems::core::sources
