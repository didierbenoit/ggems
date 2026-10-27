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
 * \brief Defines event kinds exchanged by diagnostic Observer capture.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#pragma once

#include <cstdint>
#include <string_view>

/*!
 * \brief Captures bounded diagnostic particle records independently of
 * physics.
 *
 * \namespace ggems::core::observer
 * Capture selection, storage limits, and display ordering do not define
 * scientific Transport or Scoring results.
 */
namespace ggems::core::observer {

/*! \brief Identifies the diagnostic event represented by an Observer record. */
enum class GGEMSObserverRecordKind : std::uint32_t {
  /*! \brief Unrecognized or uninitialized diagnostic event kind. */
  Unknown = 0U,
  /*! \brief Primary birth state before transport. */
  Source = 1U,
  /*! \brief Synthetic track step in the legacy dummy workload. */
  Step = 2U,
  /*!
   * \brief Newly created synthetic child state in the legacy dummy workload.
   */
  SecondaryStep = 3U,
  /*! \brief Final state of a completed or terminated track. */
  Terminal = 4U,
  /*! \brief Diagnostic record describing an execution anomaly. */
  Anomaly = 5U
};

/*!
 * \brief Encodes an Observer event kind for the shared device record.
 *
 * \param[in] record_kind Host event kind.
 * \return Corresponding unsigned device code.
 */
constexpr auto
ToKernelObserverRecordKind(GGEMSObserverRecordKind record_kind) noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(record_kind);
}

/*!
 * \brief Decodes a device Observer event kind.
 *
 * \param[in] record_kind Unsigned code read from a device record.
 * \return Matching host kind, or Unknown for an unsupported code.
 */
constexpr auto FromKernelObserverRecordKind(std::uint32_t record_kind) noexcept
  -> GGEMSObserverRecordKind {
  switch (record_kind) {
  case 1U:
    return GGEMSObserverRecordKind::Source;
  case 2U:
    return GGEMSObserverRecordKind::Step;
  case 3U:
    return GGEMSObserverRecordKind::SecondaryStep;
  case 4U:
    return GGEMSObserverRecordKind::Terminal;
  case 5U:
    return GGEMSObserverRecordKind::Anomaly;
  default:
    return GGEMSObserverRecordKind::Unknown;
  }
}

/*!
 * \brief Returns the stable display label for a diagnostic event kind.
 *
 * \param[in] record_kind Event kind to label.
 * \return Static label, or Unknown for an unrecognized value.
 */
constexpr auto ToLongName(GGEMSObserverRecordKind record_kind) noexcept
  -> std::string_view {
  switch (record_kind) {
  case GGEMSObserverRecordKind::Unknown:
    return "Unknown";
  case GGEMSObserverRecordKind::Source:
    return "Source";
  case GGEMSObserverRecordKind::Step:
    return "Step";
  case GGEMSObserverRecordKind::SecondaryStep:
    return "SecondaryStep";
  case GGEMSObserverRecordKind::Terminal:
    return "Terminal";
  case GGEMSObserverRecordKind::Anomaly:
    return "Anomaly";
  }

  return "Unknown";
}

} // namespace ggems::core::observer
