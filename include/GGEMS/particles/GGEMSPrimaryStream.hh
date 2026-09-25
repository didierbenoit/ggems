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
 * \brief Declares sequential primary-identifier reservations across runs.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::core::particles {

/*!
 * \brief Describes the global primary identifiers reserved for one run.
 * \details Each primary is one source particle starting a transport history.
 * Its global identifier is global_history_offset plus its zero-based index
 * within this reservation. This value object contains no particle data.
 */
struct GGEMSPrimaryStreamRunView {
  /*! \brief Run label supplied by the caller. */
  std::uint64_t run_id{0ULL};

  /*! \brief Number of reserved primaries. */
  std::uint64_t source_primary_count{0ULL};

  /*! \brief First reserved global identifier. */
  std::uint64_t global_history_offset{0ULL};
};

/*!
 * \brief Allocates consecutive global primary identifiers across runs.
 *
 * A new stream starts at zero. Each reservation advances the next identifier by
 * the supplied count using unchecked unsigned arithmetic. Zero-count
 * reservations leave the next identifier unchanged. Ranges remain disjoint only
 * while the caller avoids wraparound and the reserved invalid identifier.
 *
 * Run labels are copied without affecting allocation and need not be unique.
 * Calls on one stream must be serialized by the caller.
 */
class GGEMSPrimaryStream {
public:
  /*!
   * \brief Creates a stream whose first reservation starts at identifier zero.
   */
  GGEMSPrimaryStream() = default;

  /*! \brief Destroys the stream. */
  ~GGEMSPrimaryStream() = default;

  /*! \brief Prevents copying the identifier allocation state. */
  GGEMSPrimaryStream(GGEMSPrimaryStream const &other) = delete;

  /*! \brief Prevents moving the identifier allocation state. */
  GGEMSPrimaryStream(GGEMSPrimaryStream &&other) = delete;

  /*! \brief Prevents replacing the stream through copy assignment. */
  auto operator=(GGEMSPrimaryStream const &other)
    -> GGEMSPrimaryStream & = delete;

  /*! \brief Prevents replacing the stream through move assignment. */
  auto operator=(GGEMSPrimaryStream &&other) -> GGEMSPrimaryStream & = delete;

  /*!
   * \brief Reserves a consecutive range of global primary identifiers.
   *
   * \pre The count must not exceed k_invalid_id_u64 minus the next identifier,
   * so allocated identifiers remain valid and the addition does not wrap. This
   * is a caller obligation, not a runtime check.
   *
   * \param[in] run_id Run label copied into the returned view.
   * \param[in] primary_count Number of identifiers to reserve; zero is
   * accepted.
   * \return The run label, count, and first identifier before the unchecked
   * advance. An empty reservation consumes no identifiers.
   */
  [[nodiscard]] auto PrepareRun(std::uint64_t run_id,
                                std::uint64_t primary_count)
    -> GGEMSPrimaryStreamRunView;

private:
  /*! \brief Next identifier advanced by unchecked unsigned addition. */
  std::uint64_t next_global_primary_id_{0ULL};
};

} // namespace ggems::core::particles
