#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"

namespace ggems::core::radioactivity {

inline constexpr std::uint64_t k_radionuclide_host_random_seed_domain_tag{
    0x524144494F4E5543ULL};

struct GGEMSRadionuclideEmissionPlanSource {
  std::uint32_t source_index{0U};
  sources::GGEMSSourcePopulationMode population_mode{
      sources::GGEMSSourcePopulationMode::CountDriven};
  long double expected_parent_decay_count{0.0L};
  std::uint64_t emission_begin{0ULL};
  std::uint64_t emission_count{0ULL};
  std::uint64_t run_primary_begin{0ULL};
  std::uint64_t run_primary_end{0ULL};
};

struct GGEMSRadionuclideEmissionPlanGroup {
  std::uint32_t source_index{0U};
  std::uint32_t emission_index{0U};
  std::uint64_t host_stream_id{0ULL};
  long double yield_per_decay{0.0L};
  long double expected_emission_count{0.0L};
  std::uint64_t sampled_primary_count{0ULL};
  std::uint64_t source_local_primary_begin{0ULL};
  std::uint64_t source_local_primary_end{0ULL};
  std::uint64_t run_primary_begin{0ULL};
  std::uint64_t run_primary_end{0ULL};
};

class GGEMSRadionuclideEmissionPlan {
public:
  [[nodiscard]] static auto
  Create(GGEMSTimeWindow time_window,
         std::vector<GGEMSRadionuclideEmissionPlanSource> sources,
         std::vector<GGEMSRadionuclideEmissionPlanGroup> groups,
         std::vector<std::shared_ptr<GGEMSRadionuclideDefinition const>>
             radionuclide_definitions,
         std::uint64_t total_primary_count) -> GGEMSRadionuclideEmissionPlan;

  ~GGEMSRadionuclideEmissionPlan() = default;

  GGEMSRadionuclideEmissionPlan(GGEMSRadionuclideEmissionPlan const &) =
      default;
  GGEMSRadionuclideEmissionPlan(GGEMSRadionuclideEmissionPlan &&) = default;
  auto operator=(GGEMSRadionuclideEmissionPlan const &)
      -> GGEMSRadionuclideEmissionPlan & = default;
  auto operator=(GGEMSRadionuclideEmissionPlan &&)
      -> GGEMSRadionuclideEmissionPlan & = default;

  [[nodiscard]] auto GetTimeWindow() const noexcept -> GGEMSTimeWindow {
    return time_window_;
  }

  [[nodiscard]] auto GetSources() const noexcept
      -> std::span<GGEMSRadionuclideEmissionPlanSource const> {
    return sources_;
  }

  [[nodiscard]] auto GetGroups() const noexcept
      -> std::span<GGEMSRadionuclideEmissionPlanGroup const> {
    return groups_;
  }

  [[nodiscard]] auto GetRadionuclideDefinitions() const noexcept
      -> std::span<std::shared_ptr<GGEMSRadionuclideDefinition const> const> {
    return radionuclide_definitions_;
  }

  [[nodiscard]] auto GetTotalPrimaryCount() const noexcept -> std::uint64_t {
    return total_primary_count_;
  }

private:
  GGEMSRadionuclideEmissionPlan(
      GGEMSTimeWindow time_window,
      std::vector<GGEMSRadionuclideEmissionPlanSource> sources,
      std::vector<GGEMSRadionuclideEmissionPlanGroup> groups,
      std::vector<std::shared_ptr<GGEMSRadionuclideDefinition const>>
          radionuclide_definitions,
      std::uint64_t total_primary_count);

  GGEMSTimeWindow time_window_{};
  std::vector<GGEMSRadionuclideEmissionPlanSource> sources_;
  std::vector<GGEMSRadionuclideEmissionPlanGroup> groups_;
  std::vector<std::shared_ptr<GGEMSRadionuclideDefinition const>>
      radionuclide_definitions_;
  std::uint64_t total_primary_count_{0ULL};
};

class GGEMSRadionuclideEmissionPlanCandidate {
public:
  [[nodiscard]] static auto
  Create(std::shared_ptr<void const> owner_identity,
         std::uint64_t base_revision, GGEMSRadionuclideEmissionPlan plan,
         std::vector<random::GGEMSHostRandomStream> candidate_streams)
      -> GGEMSRadionuclideEmissionPlanCandidate;

  ~GGEMSRadionuclideEmissionPlanCandidate() = default;

  GGEMSRadionuclideEmissionPlanCandidate(
      GGEMSRadionuclideEmissionPlanCandidate const &) = delete;
  GGEMSRadionuclideEmissionPlanCandidate(
      GGEMSRadionuclideEmissionPlanCandidate &&other) noexcept;
  auto operator=(GGEMSRadionuclideEmissionPlanCandidate const &)
      -> GGEMSRadionuclideEmissionPlanCandidate & = delete;
  auto operator=(GGEMSRadionuclideEmissionPlanCandidate &&other) noexcept
      -> GGEMSRadionuclideEmissionPlanCandidate &;

  [[nodiscard]] auto GetPlan() const noexcept
      -> GGEMSRadionuclideEmissionPlan const & {
    return plan_;
  }

  [[nodiscard]] auto GetBaseRevision() const noexcept -> std::uint64_t {
    return base_revision_;
  }

  [[nodiscard]] auto IsCommitted() const noexcept -> bool { return committed_; }

  auto CommitTo(std::shared_ptr<void const> const &owner_identity,
                std::uint64_t &current_revision,
                std::vector<random::GGEMSHostRandomStream> &persistent_streams)
      -> void;

private:
  GGEMSRadionuclideEmissionPlanCandidate(
      std::shared_ptr<void const> owner_identity, std::uint64_t base_revision,
      GGEMSRadionuclideEmissionPlan plan,
      std::vector<random::GGEMSHostRandomStream> candidate_streams);

  std::shared_ptr<void const> owner_identity_;
  std::uint64_t base_revision_{0ULL};
  bool committed_{false};
  GGEMSRadionuclideEmissionPlan plan_;
  std::vector<random::GGEMSHostRandomStream> candidate_streams_;
};

class GGEMSRadionuclideEmissionPlanner {
public:
  GGEMSRadionuclideEmissionPlanner(
      std::span<std::shared_ptr<sources::GGEMSSource> const> sources,
      random::GGEMSRandom const &random);
  ~GGEMSRadionuclideEmissionPlanner() = default;

  GGEMSRadionuclideEmissionPlanner(GGEMSRadionuclideEmissionPlanner const &) =
      delete;
  GGEMSRadionuclideEmissionPlanner(GGEMSRadionuclideEmissionPlanner &&) =
      delete;
  auto operator=(GGEMSRadionuclideEmissionPlanner const &)
      -> GGEMSRadionuclideEmissionPlanner & = delete;
  auto operator=(GGEMSRadionuclideEmissionPlanner &&)
      -> GGEMSRadionuclideEmissionPlanner & = delete;

  [[nodiscard]] auto BuildCandidate(GGEMSTimeWindow time_window) const
      -> GGEMSRadionuclideEmissionPlanCandidate;

  auto CommitCandidate(GGEMSRadionuclideEmissionPlanCandidate &candidate)
      -> void;

  [[nodiscard]] auto GetRevision() const noexcept -> std::uint64_t {
    return revision_;
  }

private:
  struct StableSourceSlot {
    std::shared_ptr<sources::GGEMSSource> source;
    sources::GGEMSSourcePopulationMode population_mode{
        sources::GGEMSSourcePopulationMode::CountDriven};
    std::shared_ptr<GGEMSRadionuclideDefinition const> radionuclide;
    std::uint64_t first_stream_id{0ULL};
    std::uint64_t emission_count{0ULL};
  };

  std::vector<StableSourceSlot> source_slots_;
  std::vector<random::GGEMSHostRandomStream> persistent_streams_;
  std::shared_ptr<void const> owner_identity_;
  std::uint64_t revision_{0ULL};
};

} // namespace ggems::core::radioactivity
