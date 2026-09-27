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
 * \brief Defines the shared per-launch diagnostic transport counters.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::core::transport {

/*!
 * \brief Stores shared uint32 diagnostics for one kernel launch.
 *
 * Counters reset before each chunk. The atomic claim cursor includes
 * terminating out-of-range claims and must not be interpreted as a
 * completed-history count. Current transport only initializes a primary and
 * projects it one meter; legacy synthetic branching and stack counters remain
 * zero.
 */
struct GGEMSTransportCounters {
  /*!
   * \brief Atomic claim cursor, including worker probes beyond the assigned
   * count.
   */
  std::uint32_t next_primary_id{0U};

  /*!
   * \brief Primaries matched to a source and admitted to birth initialization.
   */
  std::uint32_t consumed_primary_count{0U};

  /*! \brief Histories reaching diagnostic termination. */
  std::uint32_t completed_history_count{0U};

  /*! \brief Particles marked terminal by the diagnostic projection. */
  std::uint32_t terminal_particle_count{0U};

  /*!
   * \brief Created secondary particles; zero in the current diagnostic kernel.
   */
  std::uint32_t created_secondary_count{0U};

  /*! \brief Synthetic Aionino-to-Gamma events; zero in the current kernel. */
  std::uint32_t aionino_to_gamma_count{0U};

  /*! \brief Synthetic Gamma-to-Electron events; zero in the current kernel. */
  std::uint32_t gamma_to_electron_count{0U};

  /*! \brief Synthetic Electron branching events; zero in the current kernel. */
  std::uint32_t electron_to_electron_count{0U};

  /*!
   * \brief Failed identifier, source initialization, or endpoint operations.
   */
  std::uint32_t overflow_count{0U};

  /*!
   * \brief Largest synthetic stack depth; zero in the current diagnostic
   * kernel.
   */
  std::uint32_t max_stack_depth{0U};

  /*! \brief Synthetic step count; zero in the current diagnostic kernel. */
  std::uint32_t total_fake_step_count{0U};
};

} // namespace ggems::core::transport
