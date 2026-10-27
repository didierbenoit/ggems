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
 * \brief Configures Observer capture and publishes validated run records.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace ggems::core::observer {

/*! \brief Aggregates unsaturated diagnostic counts while assembling one run. */
struct GGEMSObserverRunResultCounters {
  /*! \brief Logical count of records supplied for run aggregation. */
  std::uint64_t record_count{0ULL};
  /*! \brief Device drops plus records discarded by the host storage limit. */
  std::uint64_t overflow_count{0ULL};
  /*! \brief Selected primary histories across the run's device launches. */
  std::uint64_t captured_primary_count{0ULL};
};

/*!
 * \brief Owns diagnostic capture settings and the retained run result.
 *
 * Capture and storage limits affect diagnostics only. A temporary candidate
 * can assemble records before CommitRunResult swaps them into the published
 * result. The uint32 public counters saturate, while candidate aggregation
 * uses checked uint64 totals. Instances provide no internal synchronization;
 * callers serialize configuration, result mutation, and access to borrowed
 * record storage.
 */
class GGEMSTransportObserver {
public:
  /*!
   * \brief Constructs a disabled observer with the default capture capacity.
   */
  GGEMSTransportObserver();
  /*! \brief Releases the retained diagnostic record storage. */
  ~GGEMSTransportObserver() = default;

  /*! \brief Disallows copying diagnostic state and configuration. */
  GGEMSTransportObserver(GGEMSTransportObserver const &) = delete;
  /*! \brief Disallows moving an observer bound to run coordination. */
  GGEMSTransportObserver(GGEMSTransportObserver &&) = delete;
  /*! \brief Disallows copy assignment of diagnostic state. */
  auto operator=(GGEMSTransportObserver const &)
    -> GGEMSTransportObserver & = delete;
  /*! \brief Disallows move assignment of diagnostic state. */
  auto operator=(GGEMSTransportObserver &&)
    -> GGEMSTransportObserver & = delete;

  /*!
   * \brief Enables or disables the existing diagnostic selectors.
   *
   * \param[in] enabled Whether subsequent dispatches should capture selected
   * histories.
   * \return This observer for fluent configuration.
   */
  auto Enable(bool enabled = true) noexcept -> GGEMSTransportObserver &;
  /*!
   * \brief Disables capture while retaining selectors and stored records.
   *
   * \return This observer for fluent configuration.
   */
  auto Disable() noexcept -> GGEMSTransportObserver &;

  /*!
   * \brief Sets the maximum capture-buffer record count per workload launch.
   *
   * Existing records are retained; host capacity is reserved when necessary.
   *
   * \param[in] record_capacity Positive device capture capacity in records.
   * \return This observer for fluent configuration.
   * \throws GGEMSRecoverable If the requested capacity is zero.
   */
  auto SetRecordCapacity(std::uint32_t record_capacity)
    -> GGEMSTransportObserver &;
  /*!
   * \brief Sets the maximum number of retained host diagnostic records.
   *
   * Reducing the limit truncates existing storage and updates
   * GetRecordCount(). That immediate truncation does not increment the
   * existing overflow counter.
   *
   * \param[in] max_stored_record_count Positive host storage limit in records.
   * \return This observer for fluent configuration.
   * \throws GGEMSRecoverable If the requested limit is zero.
   */
  auto SetMaxStoredRecordCount(std::uint32_t max_stored_record_count)
    -> GGEMSTransportObserver &;

  /*!
   * \brief Enables capture for the first N primary histories of each source.
   *
   * The explicit CapturePrimary selector remains active independently.
   *
   * \param[in] primary_count_per_source Exclusive source-local primary ID
   * bound; zero selects none by this rule.
   * \return This observer for fluent configuration.
   */
  auto CaptureFirstPrimaries(std::uint32_t primary_count_per_source) noexcept
    -> GGEMSTransportObserver &;

  /*!
   * \brief Enables capture for one source-local primary history.
   *
   * The first-N selector remains active; selection uses their logical OR.
   *
   * \param[in] source_index Source slot to select.
   * \param[in] source_local_primary_id Primary ID within that source.
   * \return This observer for fluent configuration.
   */
  auto CapturePrimary(std::uint32_t source_index,
                      std::uint64_t source_local_primary_id) noexcept
    -> GGEMSTransportObserver &;
  /*!
   * \brief Clears the explicit selector without disabling first-N capture.
   *
   * \return This observer for fluent configuration.
   */
  auto ClearCapturedPrimary() noexcept -> GGEMSTransportObserver &;

  /*!
   * \brief Clears retained records and counts while preserving capture
   * settings.
   */
  void Clear() noexcept;

  /*!
   * \brief Appends a bounded device result to the host diagnostic aggregate.
   *
   * Only min(records.size(), counters.record_count) records are consumed. Host
   * storage limits and checked accumulation follow AccumulateRunResult().
   *
   * \param[in] records Completed device record storage.
   * \param[in] counters Device capture counts; record_count bounds the
   * readable prefix.
   * \throws GGEMSRecoverable If a logical uint64 counter overflows.
   */
  void Accumulate(std::span<GGEMSObserverRecord const> records,
                  GGEMSObserverCounters const &counters);

  /*!
   * \brief Creates empty staging storage for a new run's diagnostic result.
   *
   * Capture settings are not copied. The candidate is used for result assembly
   * and later publication through CommitRunResult().
   *
   * \return An independently owned observer retaining this host storage limit.
   */
  [[nodiscard]] auto CreateRunResultCandidate() const
    -> std::unique_ptr<GGEMSTransportObserver>;

  /*!
   * \brief Appends completed records and checked logical counts to a
   * candidate.
   *
   * Records beyond the host limit are counted as diagnostic overflow. Public
   * uint32 totals saturate; this method does not roll back partial aggregation
   * on failure, so failed run candidates must be discarded.
   *
   * \param[in] records Completed records to append in supplied order.
   * \param[in] logical_counters Counts for this contribution; record_count
   * must equal records.size().
   * \throws GGEMSInternal If the logical record count disagrees with the span.
   * \throws GGEMSRecoverable If a logical uint64 count overflows.
   */
  auto
  AccumulateRunResult(std::span<GGEMSObserverRecord const> records,
                      GGEMSObserverRunResultCounters const &logical_counters)
    -> void;

  /*!
   * \brief Publishes a completed candidate by swapping result storage and
   * counts.
   *
   * Capture settings and capacity limits on both observers remain unchanged.
   *
   * \param[in,out] candidate Candidate receiving the previously published
   * result.
   */
  auto CommitRunResult(GGEMSTransportObserver &candidate) noexcept -> void;

  /*!
   * \brief Packs the current capture selectors for device upload.
   *
   * \return A value record containing flags and source-local selectors.
   */
  [[nodiscard]] auto BuildConfigRecord() const noexcept
    -> GGEMSObserverConfigRecord;

  /*!
   * \brief Reports whether diagnostic capture is enabled.
   *
   * \return True when subsequent dispatches should evaluate the capture
   * selectors.
   */
  [[nodiscard]] auto IsEnabled() const noexcept -> bool;
  /*!
   * \brief Returns the configured device capture-buffer capacity.
   *
   * \return Maximum records per workload launch.
   */
  [[nodiscard]] auto GetRecordCapacity() const noexcept -> std::uint32_t;
  /*!
   * \brief Returns the number of retained host diagnostic records.
   *
   * \return Current record-storage size.
   */
  [[nodiscard]] auto GetRecordCount() const noexcept -> std::uint32_t;
  /*!
   * \brief Returns the public diagnostic drop count.
   *
   * \return Device and host drops, saturated at UINT32_MAX.
   */
  [[nodiscard]] auto GetOverflowCount() const noexcept -> std::uint32_t;
  /*!
   * \brief Returns the public count of selected primary histories.
   *
   * \return Selected-primary total, saturated at UINT32_MAX.
   */
  [[nodiscard]] auto GetCapturedPrimaryCount() const noexcept -> std::uint32_t;

  /*!
   * \brief Borrows retained records in accumulation order.
   *
   * \return Reference to this observer's record vector. Later mutation can
   * replace its contents and invalidate element references or iterators.
   */
  [[nodiscard]] auto GetRecords() const noexcept
    -> std::vector<GGEMSObserverRecord> const &;

  /*!
   * \brief Formats retained diagnostics by primary, track, time, and event
   * kind.
   *
   * Displayed track labels are local to each primary. Equal keys retain
   * capture order; ordering is for diagnostics and does not establish physical
   * causality.
   *
   * \return A formatted table without reordering the owned records.
   */
  [[nodiscard]] auto BuildDump() const -> std::string;

private:
  /*!
   * \brief Constructs empty storage for public or candidate use.
   *
   * \param[in] reserve_record_capacity Whether to reserve the default capture
   * capacity immediately.
   */
  explicit GGEMSTransportObserver(bool reserve_record_capacity);

  /*! \brief Whether dispatch configuration enables diagnostic capture. */
  bool enabled_{false};

  /*! \brief Device capture-buffer capacity in records. */
  std::uint32_t record_capacity_{65'536U};
  /*! \brief Maximum retained records in the host result. */
  std::uint32_t max_stored_record_count_{262'144U};

  /*! \brief Exclusive source-local primary ID bound for first-N capture. */
  std::uint32_t capture_first_primary_count_per_source_{0U};
  /*! \brief Whether the explicit source/primary selector is active. */
  bool capture_specific_primary_enabled_{false};
  /*! \brief Source slot for the explicit capture selector. */
  std::uint32_t capture_source_index_{particles::k_invalid_id_u32};
  /*! \brief Source-local primary ID for the explicit selector. */
  std::uint64_t capture_source_local_primary_id_{particles::k_invalid_id_u64};

  /*! \brief Checked wide totals used while assembling a run result. */
  GGEMSObserverRunResultCounters run_result_logical_counters_{};
  /*! \brief Public uint32 counts, with saturation of wide diagnostic totals. */
  GGEMSObserverCounters counters_{};
  /*! \brief Owned retained records in host accumulation order. */
  std::vector<GGEMSObserverRecord> records_;
};

} // namespace ggems::core::observer
