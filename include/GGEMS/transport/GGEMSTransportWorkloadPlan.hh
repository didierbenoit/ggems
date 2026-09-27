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
 * \brief Partitions run populations across devices and bounded kernel launches.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <vector>

/*!
 * \namespace ggems::core::transport
 * \brief Partitions primary histories and executes diagnostic device workloads.
 */
namespace ggems::core::transport {

/*!
 * \brief Assigns a contiguous run-primary interval to one device workload.
 *
 * A workload owns persistent device resources and workers. Its assigned primary
 * interval may span multiple launch chunks. A worker repeatedly consumes
 * complete histories within a chunk; worker count is not the number of assigned
 * primaries. Global history IDs add projection_history_offset to the
 * run-primary index.
 */
struct GGEMSTransportWorkloadPlan {
  /*! \brief Ordinal of this workload in the host execution plan. */
  std::uint32_t workload_index{0U};

  /*! \brief Active OpenCL context index for this workload. */
  std::uint32_t context_index{0U};

  /*! \brief Number of run primaries assigned to this workload. */
  std::uint64_t primary_count{0ULL};

  /*! \brief Persistent RNG-backed workers used by each launch. */
  std::uint32_t worker_count{0U};

  /*! \brief Global history base reserved for the entire run. */
  std::uint64_t projection_history_offset{0ULL};

  /*!
   * \brief Inclusive primary offset within the entire run, not a device-local
   * ID.
   */
  std::uint64_t device_primary_offset{0ULL};
};

/*! \brief Describes one uint32-bounded launch within a workload interval. */
struct GGEMSTransportChunk {
  /*! \brief Number of primaries in this launch chunk. */
  std::uint32_t primary_count{0U};

  /*!
   * \brief Inclusive primary offset within the entire run, not a device-local
   * ID.
   */
  std::uint64_t device_primary_offset{0ULL};
};

/*!
 * \brief Splits a workload primary interval into contiguous bounded launches.
 */
class GGEMSTransportChunkIterator {
public:
  /*!
   * \brief Starts chunking a caller-validated workload interval.
   * \param[in] device_primary_offset First run-primary index assigned to the
   * workload.
   * \param[in] primary_count Total assigned primaries, possibly zero.
   * \param[in] launch_primary_count_limit Positive maximum primaries per
   * launch.
   *
   * The caller ensures a positive limit and that the interval fits uint64. The
   * constructor stores these values without validation.
   */
  GGEMSTransportChunkIterator(std::uint64_t device_primary_offset,
                              std::uint64_t primary_count,
                              std::uint32_t launch_primary_count_limit);

  /*!
   * \brief Reports whether unissued primaries remain.
   *
   * \return True until every assigned primary belongs to a returned chunk.
   */
  [[nodiscard]] auto HasNext() const noexcept -> bool {
    return remaining_primary_count_ != 0ULL;
  }

  /*!
   * \brief Returns the next chunk and advances the remaining interval.
   *
   * \return Next contiguous interval with count no greater than the launch
   * limit.
   * \throws GGEMSRecoverable If the iterator is exhausted.
   */
  [[nodiscard]] auto Next() -> GGEMSTransportChunk;

private:
  /*! \brief Next chunk first primary index within the run population. */
  std::uint64_t next_device_primary_offset_{0ULL};

  /*! \brief Assigned primaries that have not yet been yielded. */
  std::uint64_t remaining_primary_count_{0ULL};

  /*! \brief Caller-admitted positive primary limit for one launch. */
  std::uint32_t launch_primary_count_limit_{0U};
};

/*!
 * \brief Reserves uint32 cursor headroom for worker termination probes.
 *
 * A usable launch requires this result to be positive. The helper performs only
 * the subtraction.
 *
 * \param[in] worker_count Workers that may each claim one out-of-range primary.
 * \return UINT32_MAX minus worker_count.
 */
[[nodiscard]] auto
ComputeSafeTransportLaunchPrimaryCount(std::uint32_t worker_count) noexcept
  -> std::uint32_t;

/*!
 * \brief Partitions all run primaries as evenly as possible across workloads.
 *
 * Workload and context indices are identical in this equal partition. A
 * workload can receive zero primaries; all devices retain the same source
 * chronology.
 *
 * \param[in] projection_history_offset Global history base shared by every
 * workload.
 * \param[in] total_primary_count Total run population to partition.
 * \param[in] workload_count Positive number of active workloads; division
 * requires nonzero.
 * \param[in] worker_count_per_workload Worker count copied to each plan entry.
 * \return Ordered entries with contiguous ranges; earlier entries receive
 * remainder primaries.
 */
[[nodiscard]] auto BuildEqualTransportWorkloadPlan(
  std::uint64_t projection_history_offset, std::uint64_t total_primary_count,
  std::uint32_t workload_count, std::uint32_t worker_count_per_workload)
  -> std::vector<GGEMSTransportWorkloadPlan>;

/*!
 * \brief Sums workload primary counts in an execution plan.
 *
 * \param[in] workload_plan Plan whose summed counts must fit uint64.
 * \return Total assigned primaries; zero for an empty plan.
 */
[[nodiscard]] auto CountAssignedPrimaries(
  std::vector<GGEMSTransportWorkloadPlan> const &workload_plan)
  -> std::uint64_t;

} // namespace ggems::core::transport
