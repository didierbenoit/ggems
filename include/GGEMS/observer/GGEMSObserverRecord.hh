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
 * \brief Defines host records shared with OpenCL diagnostic Observer capture.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#pragma once

#include <cstdint>

#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace ggems::core::observer {

/*!
 * \brief Selects primary histories for optional device diagnostic capture.
 *
 * The 24-byte host/device record combines two selectors with logical OR: the
 * first N source-local primaries of every source and one explicitly selected
 * (source index, source-local primary ID). A zero enabled field disables both.
 */
struct GGEMSObserverConfigRecord {
  /*! \brief Nonzero enables the configured diagnostic selectors. */
  std::uint32_t enabled{0U};
  /*! \brief Exclusive upper source-local ID bound selected for every source. */
  std::uint32_t capture_first_primary_count_per_source{0U};
  /*! \brief Nonzero enables the explicit source/primary selector. */
  std::uint32_t capture_specific_primary_enabled{0U};
  /*! \brief Source slot selected by the explicit selector. */
  std::uint32_t capture_source_index{particles::k_invalid_id_u32};
  /*! \brief Source-local primary ID selected by the explicit selector. */
  std::uint64_t capture_source_local_primary_id{particles::k_invalid_id_u64};
};

/*!
 * \brief Stores the 16-byte per-launch diagnostic capture counters.
 *
 * Device record_count counts successfully reserved slots and never exceeds the
 * configured buffer capacity. overflow_count counts rejected reservations; it
 * does not report a physics failure. Read records only after kernel
 * completion. Host aggregation exposes saturated uint32 overflow and
 * selected-primary totals.
 */
struct GGEMSObserverCounters {
  /*!
   * \brief Reserved device slots, or retained records in the host aggregate.
   */
  std::uint32_t record_count{0U};
  /*! \brief Dropped diagnostic records due to device or host capacity. */
  std::uint32_t overflow_count{0U};
  /*! \brief Selected primary histories, independent of their record count. */
  std::uint32_t captured_primary_count{0U};
  /*! \brief Reserved ABI word, initialized to zero. */
  std::uint32_t reserved_0{0U};
};

/*!
 * \brief Stores one 136-byte host/device diagnostic event snapshot.
 *
 * Global positions are signed integer picometers exported from FP32 particle
 * coordinates, rounded halfway away from zero. This is a diagnostic
 * conversion, not the persistent Navigation position. If any coordinate is
 * unrepresentable, position_valid is zero and all three exported coordinates
 * are zero. Track IDs remain local to the primary history; the invalid uint32
 * track ID is expanded to the invalid uint64 sentinel. Event capture does not
 * advance time, change particle state, consume RNG, or define scientific
 * output completeness.
 */
struct GGEMSObserverRecord {
  /*! \brief Run identifier supplied by the dispatching host. */
  std::uint64_t run_id{0ULL};
  /*! \brief Global primary-history identifier supplied at capture. */
  std::uint64_t global_primary_id{particles::k_invalid_id_u64};
  /*! \brief Primary identifier within its source population. */
  std::uint64_t source_local_primary_id{particles::k_invalid_id_u64};
  /*! \brief Particle identity copied from the persistent particle state. */
  std::uint64_t global_particle_id{particles::k_invalid_id_u64};
  /*!
   * \brief History-local track ID widened to uint64, or its invalid sentinel.
   */
  std::uint64_t track_id{particles::k_invalid_id_u64};
  /*! \brief History-local parent track ID, or the invalid uint64 sentinel. */
  std::uint64_t parent_track_id{particles::k_invalid_id_u64};
  /*! \brief Particle time in unsigned integer picoseconds. */
  std::uint64_t time_ps{0ULL};
  /*! \brief Global X diagnostic coordinate in signed integer picometers. */
  std::int64_t position_x_pm{0LL};
  /*! \brief Global Y diagnostic coordinate in signed integer picometers. */
  std::int64_t position_y_pm{0LL};
  /*! \brief Global Z diagnostic coordinate in signed integer picometers. */
  std::int64_t position_z_pm{0LL};
  /*! \brief Unsigned GGEMSObserverRecordKind device code. */
  std::uint32_t record_kind{
    ToKernelObserverRecordKind(GGEMSObserverRecordKind::Unknown)};
  /*! \brief Unsigned particle-species code copied from the particle. */
  std::uint32_t particle_type{
    particles::ToKernelParticleType(particles::GGEMSParticleType::Unknown)};
  /*! \brief Unsigned lifecycle-status code copied from the particle. */
  std::uint32_t status{particles::ToKernelParticleStatus(
    particles::GGEMSParticleStatus::Inactive)};
  /*! \brief Descendant generation; primary particles use zero. */
  std::uint32_t generation{0U};
  /*! \brief Stored global X direction component, dimensionless FP32. */
  float direction_x{0.0F};
  /*! \brief Stored global Y direction component, dimensionless FP32. */
  float direction_y{0.0F};
  /*! \brief Stored global Z direction component, dimensionless FP32. */
  float direction_z{1.0F};
  /*! \brief Reserved direction component, published as zero. */
  float direction_w{0.0F};
  /*! \brief Particle kinetic energy in unsigned integer micro-eV. */
  std::uint64_t energy_micro_eV{0ULL};
  /*! \brief Local energy attributed to this diagnostic event in micro-eV. */
  std::uint64_t deposited_energy_micro_eV{0ULL};
  /*! \brief Source slot associated with the captured primary history. */
  std::uint32_t source_index{particles::k_invalid_id_u32};
  /*! \brief Nonzero when all three exported integer coordinates are valid. */
  std::uint32_t position_valid{1U};
};

} // namespace ggems::core::observer
