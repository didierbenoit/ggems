#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/radioactivity/GGEMSDecayStatistics.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSPoissonSampler.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulationPlan.hh"

namespace ggems::core::sources {
namespace {

// =============================================================================
// =============================================================================

auto CheckedAdd(std::uint64_t lhs, std::uint64_t rhs,
                std::string const &diagnostic) -> std::uint64_t {
  if (!(rhs <= std::numeric_limits<std::uint64_t>::max() - lhs)) {
    throw ggems::core::GGEMSRecoverable(diagnostic);
  }
  return lhs + rhs;
}

// =============================================================================
// =============================================================================

auto CheckedVectorOffset(std::size_t offset, std::string const &diagnostic)
    -> std::uint64_t {
  if (!(std::in_range<std::uint64_t>(offset))) {
    throw ggems::core::GGEMSRecoverable(diagnostic);
  }
  return static_cast<std::uint64_t>(offset);
}

} // namespace

// =============================================================================
// =============================================================================

auto GGEMSSourcePopulationPlan::Create(
    GGEMSTimeWindow time_window,
    std::vector<GGEMSSourcePopulationPlanSource> sources,
    std::vector<GGEMSSourcePopulationPlanEmission> emissions,
    std::vector<
        std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
        radionuclide_definitions,
    std::uint64_t total_primary_count) -> GGEMSSourcePopulationPlan {
  return {time_window, std::move(sources), std::move(emissions),
          std::move(radionuclide_definitions), total_primary_count};
}

// -----------------------------------------------------------------------------

GGEMSSourcePopulationPlan::GGEMSSourcePopulationPlan(
    GGEMSTimeWindow time_window,
    std::vector<GGEMSSourcePopulationPlanSource> sources,
    std::vector<GGEMSSourcePopulationPlanEmission> emissions,
    std::vector<
        std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
        radionuclide_definitions,
    std::uint64_t total_primary_count)
    : time_window_{time_window}, sources_{std::move(sources)},
      emissions_{std::move(emissions)},
      radionuclide_definitions_{std::move(radionuclide_definitions)},
      total_primary_count_{total_primary_count} {}

// -----------------------------------------------------------------------------

auto GGEMSSourcePopulationCandidate::Create(
    std::shared_ptr<void const> owner_identity, std::uint64_t base_revision,
    GGEMSSourcePopulationPlan plan,
    std::vector<random::GGEMSHostRandomStream> candidate_streams)
    -> GGEMSSourcePopulationCandidate {
  return {std::move(owner_identity), base_revision, std::move(plan),
          std::move(candidate_streams)};
}

// -----------------------------------------------------------------------------

GGEMSSourcePopulationCandidate::GGEMSSourcePopulationCandidate(
    std::shared_ptr<void const> owner_identity, std::uint64_t base_revision,
    GGEMSSourcePopulationPlan plan,
    std::vector<random::GGEMSHostRandomStream> candidate_streams)
    : owner_identity_{std::move(owner_identity)}, base_revision_{base_revision},
      plan_{std::move(plan)}, candidate_streams_{std::move(candidate_streams)} {
}

// -----------------------------------------------------------------------------

GGEMSSourcePopulationCandidate::GGEMSSourcePopulationCandidate(
    GGEMSSourcePopulationCandidate &&other) noexcept
    : owner_identity_{std::move(other.owner_identity_)},
      base_revision_{other.base_revision_}, committed_{other.committed_},
      plan_{std::move(other.plan_)},
      candidate_streams_{std::move(other.candidate_streams_)} {}

// -----------------------------------------------------------------------------

auto GGEMSSourcePopulationCandidate::operator=(
    GGEMSSourcePopulationCandidate &&other) noexcept
    -> GGEMSSourcePopulationCandidate & {
  if (this == &other) {
    return *this;
  }

  owner_identity_ = std::move(other.owner_identity_);
  base_revision_ = other.base_revision_;
  committed_ = other.committed_;
  plan_ = std::move(other.plan_);
  candidate_streams_ = std::move(other.candidate_streams_);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSourcePopulationCandidate::CommitTo(
    std::shared_ptr<void const> const &owner_identity,
    std::uint64_t &current_revision,
    std::vector<random::GGEMSHostRandomStream> &persistent_streams) -> void {
  if (!(owner_identity_ == owner_identity)) {
    throw ggems::core::GGEMSRecoverable(
        "Cannot commit an emission-plan candidate from another planner.");
  }
  if (committed_) {
    throw ggems::core::GGEMSRecoverable("Emission-plan candidate was already committed.");
  }
  if (!(base_revision_ == current_revision)) {
    throw ggems::core::GGEMSRecoverable(
        "Emission-plan candidate is stale for the current planner revision.");
  }
  if (!(current_revision < std::numeric_limits<std::uint64_t>::max())) {
    throw ggems::core::GGEMSRecoverable("Emission-plan planner revision overflows uint64 storage.");
  }
  if (!(candidate_streams_.size() == persistent_streams.size())) {
    throw ggems::core::GGEMSInternal(
        "Emission-plan candidate stream count does not match its planner.");
  }

  persistent_streams.swap(candidate_streams_);
  ++current_revision;
  committed_ = true;
}

// =============================================================================
// =============================================================================

GGEMSSourcePopulationPlanner::GGEMSSourcePopulationPlanner(
    std::span<std::shared_ptr<GGEMSSource> const> sources,
    random::GGEMSRandom const &random)
    : owner_identity_{std::make_shared<std::uint8_t const>(0U)} {
  if (sources.empty()) {
    throw ggems::core::GGEMSRecoverable(
        "GGEMSSourcePopulationPlanner requires at least one source slot.");
  }
  if (!(sources.size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))) {
    throw ggems::core::GGEMSRecoverable("Emission-plan source slot count exceeds uint32 storage.");
  }

  source_slots_.reserve(sources.size());
  std::uint64_t total_stream_count{0ULL};

  for (std::size_t source_index = 0U; source_index < sources.size();
       ++source_index) {
    auto const &source = sources[source_index];
    if (!(source != nullptr)) {
      throw ggems::core::GGEMSRecoverable(
          std::format("Emission-plan source slot {} is null.", source_index));
    }

    StableSourceSlot slot{.source = source,
                          .population_mode = source->GetPopulationMode(),
                          .radionuclide = nullptr,
                          .first_stream_id = total_stream_count,
                          .emission_count = 0ULL};

    if (slot.population_mode == GGEMSSourcePopulationMode::ActivityDriven) {
      auto const configuration =
          source->BuildActivityDrivenPopulationConfiguration();
      if (!(configuration.radionuclide != nullptr)) {
        throw ggems::core::GGEMSInternal(
            "ActivityDriven source has a null radionuclide definition.");
      }

      auto const emissions = configuration.radionuclide->GetEmissions();
      if (!(emissions.size() <= static_cast<std::size_t>(
                                  std::numeric_limits<std::uint32_t>::max()))) {
        throw ggems::core::GGEMSRecoverable("Radionuclide emission count exceeds uint32 storage.");
      }
      if (!(std::in_range<std::uint64_t>(emissions.size()))) {
        throw ggems::core::GGEMSRecoverable("Radionuclide emission count exceeds uint64 storage.");
      }

      slot.radionuclide = configuration.radionuclide;
      slot.emission_count = static_cast<std::uint64_t>(emissions.size());
      total_stream_count = CheckedAdd(
          total_stream_count, slot.emission_count,
          "Host radionuclide stream count overflows uint64 storage.");
    }

    source_slots_.push_back(std::move(slot));
  }

  if (!(std::in_range<std::size_t>(total_stream_count))) {
    throw ggems::core::GGEMSRecoverable(
        "Host radionuclide stream count exceeds host vector storage.");
  }
  auto const stream_count = static_cast<std::size_t>(total_stream_count);
  if (!(stream_count <= persistent_streams_.max_size())) {
    throw ggems::core::GGEMSRecoverable("Host radionuclide stream count exceeds host vector "
                          "storage.");
  }

  random::GGEMSRandom host_random = random;
  host_random.SetSeed(random.GetSeed() ^
                      k_radionuclide_host_random_seed_domain_tag);
  host_random.ValidateStateRange(0ULL, stream_count);

  persistent_streams_.reserve(stream_count);
  for (std::uint64_t stream_id = 0ULL; stream_id < total_stream_count;
       ++stream_id) {
    persistent_streams_.emplace_back(host_random, stream_id);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSSourcePopulationPlanner::BuildCandidate(
    GGEMSTimeWindow time_window) const -> GGEMSSourcePopulationCandidate {
  if (!(time_window.start_ps <= time_window.stop_ps)) {
    throw ggems::core::GGEMSRecoverable("Emission-plan time window stop precedes its start.");
  }

  bool const has_activity_source = !persistent_streams_.empty();
  if (!(!has_activity_source || time_window.start_ps < time_window.stop_ps)) {
    throw ggems::core::GGEMSRecoverable(
        "ActivityDriven sources require a non-empty GGEMSRun time window.");
  }

  std::vector<random::GGEMSHostRandomStream> candidate_streams =
      persistent_streams_;
  std::vector<GGEMSSourcePopulationPlanSource> plan_sources;
  std::vector<GGEMSSourcePopulationPlanEmission> plan_emissions;
  std::vector<std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
      radionuclide_definitions;

  plan_sources.reserve(source_slots_.size());
  plan_emissions.reserve(candidate_streams.size());
  radionuclide_definitions.reserve(source_slots_.size());

  std::uint64_t total_primary_count{0ULL};

  for (std::size_t source_index = 0U; source_index < source_slots_.size();
       ++source_index) {
    StableSourceSlot const &slot = source_slots_[source_index];
    if (!(slot.source != nullptr)) {
      throw ggems::core::GGEMSInternal("Stable emission-plan source slot is null.");
    }
    if (!(slot.source->GetPopulationMode() == slot.population_mode)) {
      throw ggems::core::GGEMSRecoverable(
          std::format("Emission-plan source slot {} changed population mode "
                    "after planner construction.",
                    source_index));
    }

    auto const source_index_u32 = static_cast<std::uint32_t>(source_index);
    std::uint64_t const emission_begin =
        CheckedVectorOffset(plan_emissions.size(),
                            "Population-plan emission offset exceeds uint64.");
    std::uint64_t const source_primary_begin = total_primary_count;
    long double expected_parent_decay_count{0.0L};

    if (slot.population_mode == GGEMSSourcePopulationMode::CountDriven) {
      std::uint64_t const primary_count = slot.source->GetPrimaryCount();
      total_primary_count =
          CheckedAdd(total_primary_count, primary_count,
                     "Emission-plan total primary count overflows uint64.");
      radionuclide_definitions.push_back(nullptr);
    } else {
      auto const configuration =
          slot.source->BuildActivityDrivenPopulationConfiguration();
      if (!(configuration.radionuclide == slot.radionuclide)) {
        throw ggems::core::GGEMSRecoverable(
            std::format("Emission-plan source slot {} changed radionuclide "
                      "definition after planner construction.",
                      source_index));
      }

      radionuclide_definitions.push_back(slot.radionuclide);
      expected_parent_decay_count =
          radioactivity::ComputeExpectedDecayEventCount(
              configuration.activity_at_reference_time,
              slot.radionuclide->GetHalfLifeSeconds(),
              configuration.reference_time_ps, time_window);

      auto const emissions = slot.radionuclide->GetEmissions();
      if (!(emissions.size() ==
                               static_cast<std::size_t>(slot.emission_count))) {
        throw ggems::core::GGEMSInternal("Stable radionuclide emission count changed.");
      }

      std::uint64_t source_local_primary_begin{0ULL};

      for (std::size_t emission_index = 0U; emission_index < emissions.size();
           ++emission_index) {
        radioactivity::GGEMSRadionuclideEmission const &emission =
            emissions[emission_index];
        long double const yield_per_decay = emission.GetYieldPerDecay();
        long double const expected_emission_count =
            expected_parent_decay_count * yield_per_decay;

        if (!(std::isfinite(expected_emission_count) &&
                expected_emission_count >= 0.0L)) {
          throw ggems::core::GGEMSRecoverable(
              std::format("Expected emission count is invalid for source slot "
                        "{}, emission {}.",
                        source_index, emission_index));
        }

        std::uint64_t const stream_id = CheckedAdd(
            slot.first_stream_id, static_cast<std::uint64_t>(emission_index),
            "Host radionuclide stream identifier overflows.");
        if (!(std::in_range<std::size_t>(stream_id) &&
                static_cast<std::size_t>(stream_id) < candidate_streams.size())) {
          throw ggems::core::GGEMSInternal(
              "Host radionuclide stream identifier is outside candidate state.");
        }
        auto &stream = candidate_streams[static_cast<std::size_t>(stream_id)];

        std::uint64_t const sampled_primary_count =
            random::SamplePoisson(expected_emission_count, stream);
        std::uint64_t const source_local_primary_end =
            CheckedAdd(source_local_primary_begin, sampled_primary_count,
                       "Source-local emission range overflows uint64.");
        std::uint64_t const run_primary_begin = total_primary_count;
        std::uint64_t const run_primary_end =
            CheckedAdd(run_primary_begin, sampled_primary_count,
                       "Emission-plan total primary count overflows uint64.");

        plan_emissions.push_back(
            {.source_index = source_index_u32,
             .emission_index = static_cast<std::uint32_t>(emission_index),
             .host_stream_id = stream_id,
             .yield_per_decay = yield_per_decay,
             .expected_emission_count = expected_emission_count,
             .sampled_primary_count = sampled_primary_count,
             .source_local_primary_begin = source_local_primary_begin,
             .source_local_primary_end = source_local_primary_end,
             .run_primary_begin = run_primary_begin,
             .run_primary_end = run_primary_end});

        source_local_primary_begin = source_local_primary_end;
        total_primary_count = run_primary_end;
      }
    }

    std::uint64_t const emission_end =
        CheckedVectorOffset(plan_emissions.size(),
                            "Population-plan emission offset exceeds uint64.");

    plan_sources.push_back(
        {.source_index = source_index_u32,
         .population_mode = slot.population_mode,
         .expected_parent_decay_count = expected_parent_decay_count,
         .emission_begin = emission_begin,
         .emission_count = emission_end - emission_begin,
         .run_primary_begin = source_primary_begin,
         .run_primary_end = total_primary_count});
  }

  GGEMSSourcePopulationPlan plan = GGEMSSourcePopulationPlan::Create(
      time_window, std::move(plan_sources), std::move(plan_emissions),
      std::move(radionuclide_definitions), total_primary_count);

  return GGEMSSourcePopulationCandidate::Create(owner_identity_, revision_,
                                                std::move(plan),
                                                std::move(candidate_streams));
}

// -----------------------------------------------------------------------------

auto GGEMSSourcePopulationPlanner::CommitCandidate(
    GGEMSSourcePopulationCandidate &candidate) -> void {
  candidate.CommitTo(owner_identity_, revision_, persistent_streams_);
}

} // namespace ggems::core::sources
