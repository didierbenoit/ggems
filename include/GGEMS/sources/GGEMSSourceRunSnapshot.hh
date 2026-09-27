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
 * \brief Packs stable source assets and per-window execution snapshots.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRecord.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRange.hh"

namespace ggems::core::radioactivity {
class GGEMSRadionuclideDefinition;
} // namespace ggems::core::radioactivity

namespace ggems::core::sources {

class GGEMSSource;
class GGEMSSourceConfigurationSnapshot;
class GGEMSSourcePopulationPlan;
class GGEMSSourceRunSnapshot;

/*! \brief Shares immutable energy, emission, and radionuclide configuration. */
using GGEMSSourceConfigurationSnapshotPtr =
  std::shared_ptr<GGEMSSourceConfigurationSnapshot const>;

/*!
 * \brief Copies stable source energy and emission data into shared storage.
 *
 * \param[in] source Source whose current configuration is copied.
 * \return Shared ownership of an immutable packed configuration.
 * \throws GGEMSRecoverable If a source is null or its configuration cannot be
 * packed.
 */
[[nodiscard]] auto BuildSourceConfigurationSnapshot(GGEMSSource const &source)
  -> GGEMSSourceConfigurationSnapshotPtr;

/*!
 * \brief Copies stable source energy and emission data into shared storage.
 *
 * \param[in] sources Ordered collection of non-null shared sources.
 * \return Shared ownership of an immutable packed configuration.
 * \throws GGEMSRecoverable If a source is null or its configuration cannot be
 * packed.
 */
[[nodiscard]] auto BuildSourceConfigurationSnapshot(
  std::span<std::shared_ptr<GGEMSSource> const> sources)
  -> GGEMSSourceConfigurationSnapshotPtr;

/*!
 * \brief Captures current count-driven sources for one run.
 *
 * Rejects ActivityDriven sources: those require a population plan. Uses the
 * static {0, 0} time window.
 *
 * \param[in] source Source whose current configuration is copied.
 * \return Owning run snapshot with retained immutable configuration.
 * \throws GGEMSRecoverable If time order, source pointers, population mode, or
 * analytic records are invalid.
 * \throws GGEMSInternal If the retained configuration is null or its slot count
 * disagrees.
 */
[[nodiscard]] auto BuildSourceRunSnapshot(GGEMSSource const &source)
  -> GGEMSSourceRunSnapshot;

/*!
 * \brief Captures current count-driven sources for one run.
 *
 * Rejects ActivityDriven sources: those require a population plan. CountDriven
 * births use the supplied window start.
 *
 * \param[in] source Source whose current configuration is copied.
 * \param[in] time_window Ordered half-open absolute source interval in ps.
 * \return Owning run snapshot with retained immutable configuration.
 * \throws GGEMSRecoverable If time order, source pointers, population mode, or
 * analytic records are invalid.
 * \throws GGEMSInternal If the retained configuration is null or its slot count
 * disagrees.
 */
[[nodiscard]] auto BuildSourceRunSnapshot(GGEMSSource const &source,
                                          GGEMSTimeWindow time_window)
  -> GGEMSSourceRunSnapshot;

/*!
 * \brief Captures current count-driven sources for one run.
 *
 * Rejects ActivityDriven sources: those require a population plan. Uses the
 * static {0, 0} time window.
 *
 * \param[in] sources Ordered collection of non-null shared sources.
 * \return Owning run snapshot with retained immutable configuration.
 * \throws GGEMSRecoverable If time order, source pointers, population mode, or
 * analytic records are invalid.
 * \throws GGEMSInternal If the retained configuration is null or its slot count
 * disagrees.
 */
[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources)
  -> GGEMSSourceRunSnapshot;

/*!
 * \brief Captures current count-driven sources for one run.
 *
 * Rejects ActivityDriven sources: those require a population plan. CountDriven
 * births use the supplied window start.
 *
 * \param[in] sources Ordered collection of non-null shared sources.
 * \param[in] time_window Ordered half-open absolute source interval in ps.
 * \return Owning run snapshot with retained immutable configuration.
 * \throws GGEMSRecoverable If time order, source pointers, population mode, or
 * analytic records are invalid.
 * \throws GGEMSInternal If the retained configuration is null or its slot count
 * disagrees.
 */
[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot;

/*!
 * \brief Captures current count-driven sources for one run.
 *
 * Rejects ActivityDriven sources: those require a population plan. Uses the
 * static {0, 0} time window.
 *
 * \param[in] sources Ordered collection of non-null shared sources.
 * \param[in] source_configuration Non-null immutable configuration with
 * matching source slots.
 * \return Owning run snapshot with retained immutable configuration.
 * \throws GGEMSRecoverable If time order, source pointers, population mode, or
 * analytic records are invalid.
 * \throws GGEMSInternal If the retained configuration is null or its slot count
 * disagrees.
 */
[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSSourceConfigurationSnapshotPtr source_configuration)
  -> GGEMSSourceRunSnapshot;

/*!
 * \brief Captures current count-driven sources for one run.
 *
 * Rejects ActivityDriven sources: those require a population plan. CountDriven
 * births use the supplied window start.
 *
 * \param[in] sources Ordered collection of non-null shared sources.
 * \param[in] source_configuration Non-null immutable configuration with
 * matching source slots.
 * \param[in] time_window Ordered half-open absolute source interval in ps.
 * \return Owning run snapshot with retained immutable configuration.
 * \throws GGEMSRecoverable If time order, source pointers, population mode, or
 * analytic records are invalid.
 * \throws GGEMSInternal If the retained configuration is null or its slot count
 * disagrees.
 */
[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSSourceConfigurationSnapshotPtr source_configuration,
                       GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot;

/*!
 * \brief Captures the planned source population and current poses.
 *
 * ActivityDriven populations require this plan-based overload.
 *
 * \param[in] sources Ordered collection of non-null shared sources.
 * \param[in] source_configuration Non-null immutable configuration with
 * matching source slots.
 * \param[in] population_plan Coherent plan matching the source and emission
 * configuration.
 * \return Owning run snapshot with retained immutable configuration.
 * \throws GGEMSRecoverable If time order, source pointers, population mode, or
 * analytic records are invalid.
 * \throws GGEMSInternal If plan slot order, modes, emission counts, or
 * radionuclide identities disagree with the configuration.
 */
[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSSourceConfigurationSnapshotPtr source_configuration,
                       GGEMSSourcePopulationPlan const &population_plan)
  -> GGEMSSourceRunSnapshot;

/*!
 * \brief Owns immutable packed energy and emission configuration for source
 * slots.
 *
 * The first N energy descriptors are indexed by source slot: CountDriven slots
 * have a prepared energy law, while ActivityDriven slots have a placeholder.
 * Additional descriptors belong to radioactive emissions. Tabulated descriptors
 * index parallel energy, authoring-weight, and cumulative-ticket arrays by
 * element offset. Emission descriptors are grouped in source order. Shared
 * radionuclide definitions preserve their lifetime. Getter references borrow
 * this snapshot; retain shared ownership for as long as any view is used.
 */
class GGEMSSourceConfigurationSnapshot {
public:
  /*!
   * \brief Packs stable energy and emission assets for the supplied sources.
   *
   * \param[in] source Source whose current configuration is copied.
   * \return Shared immutable configuration.
   * \throws GGEMSRecoverable If a source is null or its configuration cannot be
   * packed.
   */
  [[nodiscard]] static auto Create(GGEMSSource const &source)
    -> GGEMSSourceConfigurationSnapshotPtr;

  /*!
   * \brief Packs stable energy and emission assets for the supplied sources.
   *
   * \param[in] sources Ordered collection of non-null shared sources.
   * \return Shared immutable configuration.
   * \throws GGEMSRecoverable If a source is null or its configuration cannot be
   * packed.
   */
  [[nodiscard]] static auto
  Create(std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceConfigurationSnapshotPtr;

  /*! \brief Releases owned storage and retained shared references. */
  ~GGEMSSourceConfigurationSnapshot() = default;

  /*! \brief Disallows copy construction of owned execution state. */
  GGEMSSourceConfigurationSnapshot(GGEMSSourceConfigurationSnapshot const &) =
    delete;

  /*! \brief Disallows move construction of owned execution state. */
  GGEMSSourceConfigurationSnapshot(GGEMSSourceConfigurationSnapshot &&) =
    delete;

  /*! \brief Disallows copy assignment of owned execution state. */
  auto operator=(GGEMSSourceConfigurationSnapshot const &)
    -> GGEMSSourceConfigurationSnapshot & = delete;

  /*! \brief Disallows move assignment of owned execution state. */
  auto operator=(GGEMSSourceConfigurationSnapshot &&)
    -> GGEMSSourceConfigurationSnapshot & = delete;

  /*!
   * \brief Borrows packed energy descriptors.
   *
   * \return Source-indexed descriptors followed by radioactive emission
   * descriptors.
   */
  [[nodiscard]] auto GetEnergyDistributionRecords() const noexcept
    -> std::vector<GGEMSEnergyDistributionRecord> const & {
    return energy_distribution_records_;
  }

  /*!
   * \brief Borrows the concatenated canonical energy tables.
   *
   * \return Line energies or bin centers in micro-eV, indexed by descriptor
   * offsets.
   */
  [[nodiscard]] auto GetEnergyValuesMicroElectronVolt() const noexcept
    -> std::vector<std::uint64_t> const & {
    return energy_values_micro_eV_;
  }

  /*!
   * \brief Borrows the concatenated authoring weights.
   *
   * \return Unnormalized line or bin masses parallel to the energy values.
   */
  [[nodiscard]] auto GetRelativeWeights() const noexcept
    -> std::vector<double> const & {
    return relative_weights_;
  }

  /*!
   * \brief Borrows the concatenated energy-selection bounds.
   *
   * \return Exclusive cumulative ticket bounds, restarting at each energy law.
   */
  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
    -> std::vector<std::uint64_t> const & {
    return cumulative_ticket_upper_;
  }

  /*!
   * \brief Borrows immutable radioactive emission descriptors.
   *
   * \return Descriptors grouped in source-slot order.
   */
  [[nodiscard]] auto GetEmissionRecords() const noexcept
    -> std::vector<GGEMSSourceEmissionRecord> const & {
    return source_emission_records_;
  }

  /*!
   * \brief Borrows retained per-source radioactive definitions.
   *
   * \return Source-indexed shared definitions, with null entries for
   * CountDriven.
   */
  [[nodiscard]] auto GetRadionuclideDefinitions() const noexcept -> std::vector<
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>> const & {
    return radionuclide_definitions_;
  }

  /*!
   * \brief Reads the stable source-slot count.
   *
   * \return Number of source slots represented by the configuration.
   */
  [[nodiscard]] auto GetSourceCount() const noexcept -> std::size_t {
    return source_count_;
  }

  /*!
   * \brief Reads the total radioactive emission-group count.
   *
   * \return Number of packed emission descriptors.
   */
  [[nodiscard]] auto GetEmissionCount() const noexcept -> std::size_t {
    return source_emission_records_.size();
  }

private:
  /*!
   * \brief Takes ownership of coherently packed source assets.
   * \param[in] source_count Number of ordered source slots.
   * \param[in] energy_distribution_records Packed energy descriptors.
   * \param[in] energy_values_micro_eV Parallel concatenated canonical energy
   * values.
   * \param[in] relative_weights Parallel concatenated authoring weights.
   * \param[in] cumulative_ticket_upper Parallel concatenated exclusive ticket
   * bounds.
   * \param[in] source_emission_records Immutable radioactive emission
   * descriptors.
   * \param[in] radionuclide_definitions Retained definitions parallel to source
   * slots.
   */
  GGEMSSourceConfigurationSnapshot(
    std::size_t source_count,
    std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records,
    std::vector<std::uint64_t> energy_values_micro_eV,
    std::vector<double> relative_weights,
    std::vector<std::uint64_t> cumulative_ticket_upper,
    std::vector<GGEMSSourceEmissionRecord> source_emission_records,
    std::vector<
      std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
      radionuclide_definitions);

  /*! \brief Number of source slots represented by the immutable assets. */
  std::size_t source_count_{0U};

  /*! \brief Owned descriptors locating each energy law in packed arrays. */
  std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records_;

  /*! \brief Owned concatenated canonical line energies or bin centers. */
  std::vector<std::uint64_t> energy_values_micro_eV_;

  /*! \brief Owned original weights parallel to the energy table. */
  std::vector<double> relative_weights_;

  /*! \brief Owned exclusive energy-selection ticket bounds. */
  std::vector<std::uint64_t> cumulative_ticket_upper_;

  /*! \brief Owned immutable radioactive emission descriptors. */
  std::vector<GGEMSSourceEmissionRecord> source_emission_records_;

  /*!
   * \brief Shared definitions parallel to source slots; null for CountDriven.
   */
  std::vector<std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
    radionuclide_definitions_;
};

/*!
 * \brief Captures per-run poses, sampled ranges, and immutable source assets.
 *
 * Owns the run-specific source records and primary intervals, and shares the
 * stable configuration snapshot. Counts and poses are captured at creation;
 * later source edits do not alter this value. Getter references borrow storage
 * owned by this object or its retained configuration. Copying duplicates the
 * run-specific arrays while retaining the same immutable configuration.
 */
class GGEMSSourceRunSnapshot {
public:
  /*!
   * \brief Captures source records and primary intervals for one run.
   *
   * Only CountDriven sources are accepted by this overload.
   *
   * \param[in] source Source whose current configuration is copied.
   * \param[in] time_window Ordered half-open absolute source interval in ps.
   * \return Owning run snapshot.
   * \throws GGEMSRecoverable If time order, source pointers, population mode,
   * or analytic records are invalid.
   * \throws GGEMSInternal If the retained configuration is null or its slot
   * count disagrees.
   */
  [[nodiscard]] static auto Create(GGEMSSource const &source,
                                   GGEMSTimeWindow time_window)
    -> GGEMSSourceRunSnapshot;

  /*!
   * \brief Captures source records and primary intervals for one run.
   *
   * Only CountDriven sources are accepted by this overload.
   *
   * \param[in] sources Ordered collection of non-null shared sources.
   * \param[in] source_configuration Non-null immutable configuration with
   * matching source slots.
   * \param[in] time_window Ordered half-open absolute source interval in ps.
   * \return Owning run snapshot.
   * \throws GGEMSRecoverable If time order, source pointers, population mode,
   * or analytic records are invalid.
   * \throws GGEMSInternal If the retained configuration is null or its slot
   * count disagrees.
   */
  [[nodiscard]] static auto
  Create(std::span<std::shared_ptr<GGEMSSource> const> sources,
         GGEMSSourceConfigurationSnapshotPtr source_configuration,
         GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot;

  /*!
   * \brief Captures source records and primary intervals for one run.
   *
   * Requires a matching population plan for ActivityDriven sources.
   *
   * \param[in] sources Ordered collection of non-null shared sources.
   * \param[in] source_configuration Non-null immutable configuration with
   * matching source slots.
   * \param[in] population_plan Coherent plan matching the source and emission
   * configuration.
   * \return Owning run snapshot.
   * \throws GGEMSRecoverable If time order, source pointers, population mode,
   * or analytic records are invalid.
   * \throws GGEMSInternal If the retained configuration is null or its slot
   * count disagrees.
   */
  [[nodiscard]] static auto
  Create(std::span<std::shared_ptr<GGEMSSource> const> sources,
         GGEMSSourceConfigurationSnapshotPtr source_configuration,
         GGEMSSourcePopulationPlan const &population_plan)
    -> GGEMSSourceRunSnapshot;

  ~GGEMSSourceRunSnapshot() = default;

  /*! \brief Copies value storage while retaining shared references. */
  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot const &) = default;

  /*! \brief Transfers owned storage and shared references. */
  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot &&) = default;

  /*!
   * \brief Copies value storage while retaining shared references.
   * \return This object after assignment.
   */
  auto operator=(GGEMSSourceRunSnapshot const &)
    -> GGEMSSourceRunSnapshot & = default;

  /*!
   * \brief Transfers owned storage and shared references.
   * \return This object after assignment.
   */
  auto operator=(GGEMSSourceRunSnapshot &&)
    -> GGEMSSourceRunSnapshot & = default;

  /*!
   * \brief Borrows this run source poses and analytic birth laws.
   *
   * \return Source-indexed records carrying the common run window.
   */
  [[nodiscard]] auto GetRecords() const noexcept
    -> std::vector<GGEMSSourceRecord> const & {
    return records_;
  }

  /*!
   * \brief Borrows source intervals within the run population.
   *
   * \return Run-wide half-open intervals represented by begin and count.
   */
  [[nodiscard]] auto GetRanges() const noexcept
    -> std::vector<GGEMSSourceRunRange> const & {
    return ranges_;
  }

  /*!
   * \brief Borrows per-source population and radioactive-time descriptors.
   *
   * \return Records parallel to source slots.
   */
  [[nodiscard]] auto GetPopulationRecords() const noexcept
    -> std::vector<GGEMSSourcePopulationRecord> const & {
    return population_records_;
  }

  /*!
   * \brief Borrows primary intervals for radioactive emission groups.
   *
   * \return Source-local ranges parallel to immutable emission descriptors.
   */
  [[nodiscard]] auto GetGroupRanges() const noexcept
    -> std::vector<GGEMSSourceEmissionRange> const & {
    return emission_ranges_;
  }

  /*!
   * \brief Reads the common source-emission window.
   *
   * \return Half-open absolute interval in ps.
   */
  [[nodiscard]] auto GetTimeWindow() const noexcept -> GGEMSTimeWindow {
    return time_window_;
  }

  /*!
   * \brief Borrows the immutable configuration retained by this snapshot.
   *
   * \return Reference valid while shared configuration ownership remains.
   */
  [[nodiscard]] auto GetSourceConfiguration() const noexcept
    -> GGEMSSourceConfigurationSnapshot const & {
    return *source_configuration_;
  }

  /*!
   * \brief Borrows packed energy descriptors.
   *
   * \return Source-indexed descriptors followed by radioactive emission
   * descriptors.
   */
  [[nodiscard]] auto GetEnergyDistributionRecords() const noexcept
    -> std::vector<GGEMSEnergyDistributionRecord> const & {
    return source_configuration_->GetEnergyDistributionRecords();
  }

  /*!
   * \brief Borrows the concatenated canonical energy tables.
   *
   * \return Line energies or bin centers in micro-eV, indexed by descriptor
   * offsets.
   */
  [[nodiscard]] auto GetEnergyValuesMicroElectronVolt() const noexcept
    -> std::vector<std::uint64_t> const & {
    return source_configuration_->GetEnergyValuesMicroElectronVolt();
  }

  /*!
   * \brief Borrows the concatenated authoring weights.
   *
   * \return Unnormalized line or bin masses parallel to the energy values.
   */
  [[nodiscard]] auto GetRelativeWeights() const noexcept
    -> std::vector<double> const & {
    return source_configuration_->GetRelativeWeights();
  }

  /*!
   * \brief Borrows the concatenated energy-selection bounds.
   *
   * \return Exclusive cumulative ticket bounds, restarting at each energy law.
   */
  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
    -> std::vector<std::uint64_t> const & {
    return source_configuration_->GetCumulativeTicketUpperBounds();
  }

  /*!
   * \brief Borrows immutable radioactive emission descriptors.
   *
   * \return Descriptors grouped in source-slot order.
   */
  [[nodiscard]] auto GetEmissionRecords() const noexcept
    -> std::vector<GGEMSSourceEmissionRecord> const & {
    return source_configuration_->GetEmissionRecords();
  }

  /*!
   * \brief Borrows retained per-source radioactive definitions.
   *
   * \return Source-indexed shared definitions, with null entries for
   * CountDriven.
   */
  [[nodiscard]] auto GetRadionuclideDefinitions() const noexcept -> std::vector<
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>> const & {
    return source_configuration_->GetRadionuclideDefinitions();
  }

  /*!
   * \brief Reads the full explicit and sampled primary population.
   *
   * \return Total number of primaries across all source slots.
   */
  [[nodiscard]] auto GetTotalPrimaryCount() const noexcept -> std::uint64_t {
    return total_primary_count_;
  }

  /*!
   * \brief Reports whether any source uses radioactive population planning.
   *
   * \return True if an ActivityDriven mode appears in the population records.
   */
  [[nodiscard]] auto HasActivityDrivenSource() const noexcept -> bool;

private:
  /*!
   * \brief Takes ownership of prepared per-run arrays and shared assets.
   * \param[in] records Captured source records.
   * \param[in] ranges Source ranges in the run population.
   * \param[in] population_records Per-source population descriptors.
   * \param[in] emission_ranges Source-local ranges for radioactive groups.
   * \param[in] time_window Common half-open interval in ps.
   * \param[in] source_configuration Retained immutable source assets.
   * \param[in] total_primary_count Total primary count across sources.
   */
  GGEMSSourceRunSnapshot(
    std::vector<GGEMSSourceRecord> records,
    std::vector<GGEMSSourceRunRange> ranges,
    std::vector<GGEMSSourcePopulationRecord> population_records,
    std::vector<GGEMSSourceEmissionRange> emission_ranges,
    GGEMSTimeWindow time_window,
    GGEMSSourceConfigurationSnapshotPtr source_configuration,
    std::uint64_t total_primary_count);

  /*! \brief Captured source poses and analytic birth laws. */
  std::vector<GGEMSSourceRecord> records_;

  /*! \brief Run-wide primary intervals for each source slot. */
  std::vector<GGEMSSourceRunRange> ranges_;

  /*! \brief Per-source population law and scaled radioactive decay. */
  std::vector<GGEMSSourcePopulationRecord> population_records_;

  /*! \brief Source-local primary intervals for radioactive groups. */
  std::vector<GGEMSSourceEmissionRange> emission_ranges_;

  /*! \brief Common half-open source-emission window in absolute ps. */
  GGEMSTimeWindow time_window_;

  /*! \brief Retains immutable energy tables and emission definitions. */
  GGEMSSourceConfigurationSnapshotPtr source_configuration_;

  /*! \brief Number of primaries represented by all source ranges. */
  std::uint64_t total_primary_count_{0ULL};
};
} // namespace ggems::core::sources
