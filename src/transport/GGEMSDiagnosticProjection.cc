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
 * \brief Validates canonical sources before diagnostic device execution.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <span>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceValidation.hh"
#include "GGEMS/transport/GGEMSDiagnosticProjection.hh"

namespace ggems::core::transport {

auto ValidateDiagnosticTransportSources(
  std::span<sources::GGEMSSourceRecord const> source_records,
  std::span<sources::GGEMSSourceRunRange const> source_ranges) -> void {
  if (source_records.size() != source_ranges.size()) {
    throw GGEMSRecoverable(
      "Diagnostic transport source record and range counts must match.");
  }

  for (std::size_t source_index = 0U; source_index < source_records.size();
       ++source_index) {
    if (source_ranges[source_index].primary_count != 0ULL) {
      sources::ValidateAnalyticSourceRecord(source_records[source_index]);
    }
  }
}
} // namespace ggems::core::transport
