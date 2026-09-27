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
 * \brief Plans and transactionally commits per-window source populations.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"

namespace ggems::core::sources {

/*!
 * \brief Separates host radioactive population seeds from device transport
 * seeds.
 */
inline constexpr std::uint64_t k_radionuclide_host_random_seed_domain_tag{
  0x524144494F4E5543ULL};

/*! \brief Describes one source slot in a sampled run population. */
struct GGEMSSourcePopulationPlanSource {
  /*! \brief Source slot index in the ordered configuration. */
  std::uint32_t source_index{0U};

  /*! \brief Population law of this source slot. */
  GGEMSSourcePopulationMode population_mode{
    GGEMSSourcePopulationMode::CountDriven};

  /*! \brief Integrated expected parent decays; zero for CountDriven. */
  long double expected_parent_decay_count{0.0L};

  /*! \brief First entry in the plan emission array. */
  std::uint64_t emission_begin{0ULL};

  /*! \brief Number of radioactive groups owned by this source slot. */
  std::uint64_t emission_count{0ULL};

  /*! \brief Inclusive first primary index within the entire run. */
  std::uint64_t run_primary_begin{0ULL};

  /*! \brief Exclusive primary end within the entire run. */
  std::uint64_t run_primary_end{0ULL};
};

/*! \brief Describes one independently sampled radioactive emission group. */
struct GGEMSSourcePopulationPlanEmission {
  /*! \brief Source slot index in the ordered configuration. */
  std::uint32_t source_index{0U};

  /*! \brief Group index within its radionuclide definition. */
  std::uint32_t emission_index{0U};

  /*! \brief Stable host RNG stream index allocated to this emission group. */
  std::uint64_t host_stream_id{0ULL};

  /*! \brief Expected emitted particles in this group per parent decay. */
  long double yield_per_decay{0.0L};

  /*! \brief Parent-decay expectation multiplied by the group yield. */
  long double expected_emission_count{0.0L};

  /*! \brief Poisson realization used as this group primary count. */
  std::uint64_t sampled_primary_count{0ULL};

  /*! \brief Inclusive first primary index within the owning source. */
  std::uint64_t source_local_primary_begin{0ULL};

  /*! \brief Exclusive primary end within the owning source. */
  std::uint64_t source_local_primary_end{0ULL};

  /*! \brief Inclusive first primary index within the entire run. */
  std::uint64_t run_primary_begin{0ULL};

  /*! \brief Exclusive primary end within the entire run. */
  std::uint64_t run_primary_end{0ULL};
};

/*!
 * \brief Owns a per-window population and its source/group primary intervals.
 *
 * Primary intervals are half-open. Run indices concatenate source slots in
 * configuration order; source-local indices concatenate one source's emission
 * groups. Activity expectations are dimensionless counts and yields are
 * particles per parent decay. Count-driven slots have no emission groups.
 * Getter spans borrow this object's storage and remain valid until assignment
 * or destruction.
 */
class GGEMSSourcePopulationPlan {
public:
  /*!
   * \brief Takes ownership of an already coherent population plan.
   *
   * \param[in] time_window Ordered half-open interval in absolute ps.
   * \param[in] sources Owned ordered slot metadata.
   * \param[in] emissions Owned group metadata with matching primary intervals.
   * \param[in] radionuclide_definitions Shared definitions parallel to sources;
   * null for CountDriven.
   * \param[in] total_primary_count Total count consistent with the supplied
   * intervals.
   * \return Value plan; consistency is a caller precondition.
   */
  [[nodiscard]] static auto
  Create(GGEMSTimeWindow time_window,
         std::vector<GGEMSSourcePopulationPlanSource> sources,
         std::vector<GGEMSSourcePopulationPlanEmission> emissions,
         std::vector<
           std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
           radionuclide_definitions,
         std::uint64_t total_primary_count) -> GGEMSSourcePopulationPlan;

  /*! \brief Releases owned storage and retained shared references. */
  ~GGEMSSourcePopulationPlan() = default;

  /*! \brief Copies value storage while retaining shared references. */
  GGEMSSourcePopulationPlan(GGEMSSourcePopulationPlan const &) = default;

  /*! \brief Transfers owned storage and shared references. */
  GGEMSSourcePopulationPlan(GGEMSSourcePopulationPlan &&) = default;

  /*!
   * \brief Copies value storage while retaining shared references.
   * \return This object after assignment.
   */
  auto operator=(GGEMSSourcePopulationPlan const &)
    -> GGEMSSourcePopulationPlan & = default;

  /*!
   * \brief Transfers owned storage and shared references.
   * \return This object after assignment.
   */
  auto operator=(GGEMSSourcePopulationPlan &&)
    -> GGEMSSourcePopulationPlan & = default;

  /*!
   * \brief Reads the population source window.
   *
   * \return Ordered half-open absolute interval in ps.
   */
  [[nodiscard]] auto GetTimeWindow() const noexcept -> GGEMSTimeWindow {
    return time_window_;
  }

  /*!
   * \brief Borrows the ordered source-slot metadata.
   *
   * \return View of source ranges and expectations.
   */
  [[nodiscard]] auto GetSources() const noexcept
    -> std::span<GGEMSSourcePopulationPlanSource const> {
    return sources_;
  }

  /*!
   * \brief Borrows independently sampled emission metadata.
   *
   * \return View in source order, then definition emission order.
   */
  [[nodiscard]] auto GetGroups() const noexcept
    -> std::span<GGEMSSourcePopulationPlanEmission const> {
    return emissions_;
  }

  /*!
   * \brief Borrows retained per-source radioactive definitions.
   *
   * \return View parallel to source slots, with null CountDriven entries.
   */
  [[nodiscard]] auto GetRadionuclideDefinitions() const noexcept -> std::span<
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const> const> {
    return radionuclide_definitions_;
  }

  /*!
   * \brief Reads the total run population.
   *
   * \return Number of primary particles across all sources.
   */
  [[nodiscard]] auto GetTotalPrimaryCount() const noexcept -> std::uint64_t {
    return total_primary_count_;
  }

private:
  /*!
   * \brief Stores prepared population data without revalidation.
   * \param[in] time_window Ordered half-open interval in absolute ps.
   * \param[in] sources Owned ordered slot metadata.
   * \param[in] emissions Owned group metadata with matching primary intervals.
   * \param[in] radionuclide_definitions Shared definitions parallel to sources;
   * null for CountDriven.
   * \param[in] total_primary_count Total count consistent with the supplied
   * intervals.
   */
  GGEMSSourcePopulationPlan(
    GGEMSTimeWindow time_window,
    std::vector<GGEMSSourcePopulationPlanSource> sources,
    std::vector<GGEMSSourcePopulationPlanEmission> emissions,
    std::vector<
      std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
      radionuclide_definitions,
    std::uint64_t total_primary_count);

  /*! \brief Half-open absolute source time interval in picoseconds. */
  GGEMSTimeWindow time_window_{};

  /*! \brief Owned ordered source-slot ranges and expectations. */
  std::vector<GGEMSSourcePopulationPlanSource> sources_;

  /*! \brief Owned independent group counts and primary intervals. */
  std::vector<GGEMSSourcePopulationPlanEmission> emissions_;

  /*! \brief Retained per-source definitions; CountDriven entries are null. */
  std::vector<std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
    radionuclide_definitions_;

  /*! \brief Total sampled and explicit primaries across all source slots. */
  std::uint64_t total_primary_count_{0ULL};
};

/*!
 * \brief Stages a population plan with tentative host random-stream states.
 *
 * Building a candidate does not advance the planner's persistent streams.
 * CommitTo() accepts only the matching owner and current revision, once. Moving
 * a candidate transfers its plan and commit authority; a moved-from candidate
 * must not be used to access the transferred plan or commit state.
 */
class GGEMSSourcePopulationCandidate {
public:
  /*!
   * \brief Stages a plan and tentative stream states for one planner revision.
   *
   * \param[in] owner_identity Shared token of the planner that may commit this
   * candidate.
   * \param[in] base_revision Planner revision used to sample the candidate.
   * \param[in] plan Owned sampled population.
   * \param[in] candidate_streams Owned post-sampling stream states.
   * \return Uncommitted candidate.
   */
  [[nodiscard]] static auto
  Create(std::shared_ptr<void const> owner_identity,
         std::uint64_t base_revision, GGEMSSourcePopulationPlan plan,
         std::vector<random::GGEMSHostRandomStream> candidate_streams)
    -> GGEMSSourcePopulationCandidate;

  ~GGEMSSourcePopulationCandidate() = default;

  /*! \brief Disallows copy construction of owned execution state. */
  GGEMSSourcePopulationCandidate(GGEMSSourcePopulationCandidate const &) =
    delete;

  /*! \brief Transfers owned storage and shared references. */
  GGEMSSourcePopulationCandidate(GGEMSSourcePopulationCandidate &&) noexcept =
    default;

  /*! \brief Disallows copy assignment of owned execution state. */
  auto operator=(GGEMSSourcePopulationCandidate const &)
    -> GGEMSSourcePopulationCandidate & = delete;

  /*!
   * \brief Transfers a candidate plan, tentative streams, and commit state.
   * \param[in,out] other Candidate whose state is moved; self-assignment is a
   * no-op.
   * \return This candidate.
   */
  auto operator=(GGEMSSourcePopulationCandidate &&other) noexcept
    -> GGEMSSourcePopulationCandidate &;

  /*!
   * \brief Borrows the candidate population plan.
   *
   * \return Reference owned by this candidate.
   */
  [[nodiscard]] auto GetPlan() const noexcept
    -> GGEMSSourcePopulationPlan const & {
    return plan_;
  }

  /*!
   * \brief Reads the candidate planning revision.
   *
   * \return Revision that must still match the planner at commit.
   */
  [[nodiscard]] auto GetBaseRevision() const noexcept -> std::uint64_t {
    return base_revision_;
  }

  /*!
   * \brief Reports whether the candidate stream state was committed.
   *
   * \return True after a successful CommitTo().
   */
  [[nodiscard]] auto IsCommitted() const noexcept -> bool { return committed_; }

  /*!
   * \brief Publishes tentative streams to the matching planner exactly once.
   *
   * \param[in] owner_identity Destination planner identity token.
   * \param[in,out] current_revision Destination revision, incremented on
   * success.
   * \param[in,out] persistent_streams Persistent states exchanged with
   * candidate states.
   * \throws GGEMSRecoverable If owner mismatches, the candidate is stale, or it
   * already committed.
   * \throws GGEMSInternal If stream counts disagree.
   */
  auto CommitTo(std::shared_ptr<void const> const &owner_identity,
                std::uint64_t &current_revision,
                std::vector<random::GGEMSHostRandomStream> &persistent_streams)
    -> void;

private:
  /*!
   * \brief Stores a plan and tentative random-stream ownership.
   * \param[in] owner_identity Shared token of the planner that may commit this
   * candidate.
   * \param[in] base_revision Planner revision used to sample the candidate.
   * \param[in] plan Owned sampled population.
   * \param[in] candidate_streams Owned post-sampling stream states.
   */
  GGEMSSourcePopulationCandidate(
    std::shared_ptr<void const> owner_identity, std::uint64_t base_revision,
    GGEMSSourcePopulationPlan plan,
    std::vector<random::GGEMSHostRandomStream> candidate_streams);

  /*! \brief Shared identity token distinguishing the owning planner. */
  std::shared_ptr<void const> owner_identity_;

  /*! \brief Planner revision from which this candidate was built. */
  std::uint64_t base_revision_{0ULL};

  /*! \brief Whether this candidate has already committed its streams. */
  bool committed_{false};

  /*! \brief Owned source and emission plan for the proposed run. */
  GGEMSSourcePopulationPlan plan_;

  /*!
   * \brief Tentative post-sampling states, exchanged with persistent state on
   * commit.
   */
  std::vector<random::GGEMSHostRandomStream> candidate_streams_;
};

/*!
 * \brief Plans run populations using stable source slots and per-group RNG.
 *
 * Count-driven sources contribute their current requested counts. Each
 * radioactive group independently samples Poisson(expected parent decays *
 * yield). This is a marginal-emission model, not a correlated decay-event
 * generator. One host stream belongs to each radioactive group and advances
 * only on candidate commit. Source modes and radionuclide identities must stay
 * stable after construction; callers serialize planning, commits, and source
 * configuration changes.
 */
class GGEMSSourcePopulationPlanner {
public:
  /*!
   * \brief Allocates stable host random streams for the supplied source slots.
   * \param[in] sources Nonempty ordered collection of non-null shared sources.
   * \param[in] random Engine and seed used to derive the host population
   * domain.
   * \throws GGEMSRecoverable If slots are empty or null, or the random engine
   * cannot provide the required stream range.
   */
  GGEMSSourcePopulationPlanner(
    std::span<std::shared_ptr<GGEMSSource> const> sources,
    random::GGEMSRandom const &random);
  ~GGEMSSourcePopulationPlanner() = default;

  /*! \brief Disallows copy construction of owned execution state. */
  GGEMSSourcePopulationPlanner(GGEMSSourcePopulationPlanner const &) = delete;

  /*! \brief Disallows move construction of owned execution state. */
  GGEMSSourcePopulationPlanner(GGEMSSourcePopulationPlanner &&) = delete;

  /*! \brief Disallows copy assignment of owned execution state. */
  auto operator=(GGEMSSourcePopulationPlanner const &)
    -> GGEMSSourcePopulationPlanner & = delete;

  /*! \brief Disallows move assignment of owned execution state. */
  auto operator=(GGEMSSourcePopulationPlanner &&)
    -> GGEMSSourcePopulationPlanner & = delete;

  /*!
   * \brief Samples a proposed population using copies of persistent streams.
   *
   * \param[in] time_window Ordered source interval in ps; positive width for
   * radioactive groups.
   * \return Uncommitted population and post-sampling stream states.
   * \throws GGEMSRecoverable If the window, stable mode/definition,
   * expectation, or Poisson admission is invalid.
   * \throws GGEMSInternal If an immutable emission count changed.
   */
  [[nodiscard]] auto BuildCandidate(GGEMSTimeWindow time_window) const
    -> GGEMSSourcePopulationCandidate;

  /*!
   * \brief Accepts the candidate and advances persistent population state.
   *
   * \param[in,out] candidate Candidate built for this planner and current
   * revision.
   * \throws GGEMSRecoverable If candidate ownership, revision, or committed
   * state is invalid.
   * \throws GGEMSInternal If stream counts disagree.
   */
  auto CommitCandidate(GGEMSSourcePopulationCandidate &candidate) -> void;

  /*!
   * \brief Reads the current population-planner revision.
   *
   * \return Number of successful candidate commits.
   */
  [[nodiscard]] auto GetRevision() const noexcept -> std::uint64_t {
    return revision_;
  }

private:
  /*! \brief Retains source identity and its fixed host-stream allocation. */
  struct StableSourceSlot {
    /*! \brief Shared source inspected for each candidate population. */
    std::shared_ptr<GGEMSSource const> source;

    /*! \brief Population mode frozen at planner construction. */
    GGEMSSourcePopulationMode population_mode{
      GGEMSSourcePopulationMode::CountDriven};

    /*! \brief Shared fixed radioactive definition, or null for CountDriven. */
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>
      radionuclide;

    /*! \brief First persistent host-stream index for this source. */
    std::uint64_t first_stream_id{0ULL};

    /*! \brief Fixed number of streams, one per radioactive emission. */
    std::uint64_t emission_count{0ULL};
  };

  /*! \brief Ordered stable source identities and stream ranges. */
  std::vector<StableSourceSlot> source_slots_;

  /*! \brief Host population states after the last successful commit. */
  std::vector<random::GGEMSHostRandomStream> persistent_streams_;

  /*! \brief Shared identity token distinguishing the owning planner. */
  std::shared_ptr<void const> owner_identity_;

  /*! \brief Revision incremented once per committed candidate. */
  std::uint64_t revision_{0ULL};
};

} // namespace ggems::core::sources
