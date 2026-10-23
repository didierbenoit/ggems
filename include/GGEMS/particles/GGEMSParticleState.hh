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
 * \brief Defines the host particle-state record exchanged with OpenCL.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace ggems::core::particles {

/*!
 * \brief Stores the 80-byte host counterpart of the OpenCL particle state.
 *
 * Positions use binary32 picometers, time uses unsigned integer picoseconds,
 * and energy uses unsigned integer micro-eV. Directions are dimensionless
 * binary32 values. Default initialization creates an inactive, unspecified
 * particle at the origin, pointing along +Z. This aggregate performs no
 * validation or normalization. Natural tail padding is not particle state.
 *
 * Track identifiers are local to the primary history. Current synthetic
 * descendants retain the primary global identifier; global identity for future
 * physical secondaries is not specified by this record.
 */
struct GGEMSParticleState {
  /*!
   * \brief Primary-history identifier shared by the current synthetic
   * descendants.
   */
  std::uint64_t global_particle_id{k_invalid_id_u64};

  /*! \brief Nonnegative simulation time in integer picoseconds. */
  std::uint64_t time_ps{0ULL};

  /*! \brief Nonnegative kinetic energy in integer micro-eV. */
  std::uint64_t energy_micro_eV{0ULL};

  /*! \brief History-local track identifier; source primaries use zero. */
  std::uint32_t track_id{k_invalid_id_u32};

  /*!
   * \brief History-local parent track identifier, or the invalid 32-bit
   * sentinel.
   */
  std::uint32_t parent_track_id{k_invalid_id_u32};

  /*! \brief Current navigator identifier, initially invalid. */
  std::uint32_t current_navigator_id{k_invalid_id_u32};

  /*! \brief Current volume identifier, initially invalid. */
  std::uint32_t current_volume_id{k_invalid_id_u32};

  /*! \brief Current material identifier, initially invalid. */
  std::uint32_t material_id{k_invalid_id_u32};

  /*! \brief Current region identifier, initially invalid. */
  std::uint32_t region_id{k_invalid_id_u32};

  /*! \brief Global X coordinate in binary32 picometers. */
  float position_x_pm{0.0F};

  /*! \brief Global Y coordinate in binary32 picometers. */
  float position_y_pm{0.0F};

  /*! \brief Global Z coordinate in binary32 picometers. */
  float position_z_pm{0.0F};

  /*! \brief Dimensionless X direction component. */
  float direction_x{0.0F};

  /*! \brief Dimensionless Y direction component. */
  float direction_y{0.0F};

  /*!
   * \brief Dimensionless Z direction component, defaulting to +1 on the host.
   */
  float direction_z{1.0F};

  /*! \brief Kernel numeric particle-species code. */
  std::uint8_t particle_type{ToKernelParticleType(GGEMSParticleType::Unknown)};

  /*! \brief Kernel numeric particle-status code. */
  std::uint8_t status{ToKernelParticleStatus(GGEMSParticleStatus::Inactive)};

  /*! \brief Descendant generation; source primaries start at zero. */
  std::uint8_t generation{0U};
};

} // namespace ggems::core::particles
