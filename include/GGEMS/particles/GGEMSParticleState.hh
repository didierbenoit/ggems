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
 * \brief Stores the host counterpart of the OpenCL particle-state record.
 *
 * Positions use signed integer picometers, time uses unsigned integer
 * picoseconds, and energy uses unsigned integer micro-eV. Direction components
 * are dimensionless floats. Default initialization creates an inactive,
 * unspecified particle at the origin, pointing along +Z. This aggregate
 * performs no validation or normalization.
 *
 * Identifiers are assigned by the transport path. In particular,
 * global_particle_id is not always a primary-history identifier: the synthetic
 * branching path assigns a secondary's track identifier to it.
 */
struct GGEMSParticleState {
  /*! \brief Particle identifier assigned by source or transport. */
  std::uint64_t global_particle_id{k_invalid_id_u64};

  /*! \brief Track identifier in the owning transport path. */
  std::uint64_t track_id{k_invalid_id_u64};

  /*! \brief Parent track identifier, or k_invalid_id_u64 for a primary. */
  std::uint64_t parent_track_id{k_invalid_id_u64};

  /*! \brief Nonnegative simulation time in integer picoseconds. */
  std::uint64_t time_ps{0ULL};

  /*! \brief Signed X coordinate in integer picometers. */
  std::int64_t position_x_pm{0LL};

  /*! \brief Signed Y coordinate in integer picometers. */
  std::int64_t position_y_pm{0LL};

  /*! \brief Signed Z coordinate in integer picometers. */
  std::int64_t position_z_pm{0LL};

  /*! \brief Kernel numeric particle-species code. */
  std::uint32_t particle_type{ToKernelParticleType(GGEMSParticleType::Unknown)};

  /*! \brief Kernel numeric particle-status code. */
  std::uint32_t status{ToKernelParticleStatus(GGEMSParticleStatus::Inactive)};

  /*! \brief Descendant generation; source primaries start at zero. */
  std::uint32_t generation{0U};

  /*! \brief Transport-specific flags; zero at source initialization. */
  std::uint32_t flags{0U};

  /*! \brief Current navigator identifier, initially invalid. */
  std::uint32_t current_navigator_id{k_invalid_id_u32};

  /*! \brief Current volume identifier, initially invalid. */
  std::uint32_t current_volume_id{k_invalid_id_u32};

  /*! \brief Current material identifier, initially invalid. */
  std::uint32_t material_id{k_invalid_id_u32};

  /*! \brief Current region identifier, initially invalid. */
  std::uint32_t region_id{k_invalid_id_u32};

  /*! \brief Dimensionless X direction component. */
  float direction_x{0.0F};

  /*! \brief Dimensionless Y direction component. */
  float direction_y{0.0F};

  /*! \brief Dimensionless Z direction component, defaulting to +1. */
  float direction_z{1.0F};

  /*! \brief Reserved fourth direction component, initialized to zero. */
  float direction_w{0.0F};

  /*! \brief Nonnegative kinetic energy in integer micro-eV. */
  std::uint64_t energy_micro_eV{0ULL};
};

} // namespace ggems::core::particles
