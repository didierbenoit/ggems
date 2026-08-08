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

namespace ggems::core::sources {

inline constexpr std::uint64_t k_radionuclide_host_random_seed_domain_tag{
    0x524144494F4E5543ULL};

struct GGEMSSourcePopulationPlanSource {
  std::uint32_t source_index{0U};
  GGEMSSourcePopulationMode population_mode{
      GGEMSSourcePopulationMode::CountDriven};
  long double expected_parent_decay_count{0.0L};
  std::uint64_t emission_begin{0ULL};
  std::uint64_t emission_count{0ULL};
  std::uint64_t run_primary_begin{0ULL};
  std::uint64_t run_primary_end{0ULL};
};

struct GGEMSSourcePopulationPlanEmission {
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

class GGEMSSourcePopulationPlan {
public:
  [[nodiscard]] static auto
  Create(GGEMSTimeWindow time_window,
         std::vector<GGEMSSourcePopulationPlanSource> sources,
         std::vector<GGEMSSourcePopulationPlanEmission> emissions,
         std::vector<
             std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
             radionuclide_definitions,
         std::uint64_t total_primary_count) -> GGEMSSourcePopulationPlan;

  ~GGEMSSourcePopulationPlan() = default;

  GGEMSSourcePopulationPlan(GGEMSSourcePopulationPlan const &) = default;
  GGEMSSourcePopulationPlan(GGEMSSourcePopulationPlan &&) = default;
  auto operator=(GGEMSSourcePopulationPlan const &)
      -> GGEMSSourcePopulationPlan & = default;
  auto operator=(GGEMSSourcePopulationPlan &&)
      -> GGEMSSourcePopulationPlan & = default;

  [[nodiscard]] auto GetTimeWindow() const noexcept -> GGEMSTimeWindow {
    return time_window_;
  }

  [[nodiscard]] auto GetSources() const noexcept
      -> std::span<GGEMSSourcePopulationPlanSource const> {
    return sources_;
  }

  [[nodiscard]] auto GetGroups() const noexcept
      -> std::span<GGEMSSourcePopulationPlanEmission const> {
    return emissions_;
  }

  [[nodiscard]] auto GetRadionuclideDefinitions() const noexcept -> std::span<
      std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const> const> {
    return radionuclide_definitions_;
  }

  [[nodiscard]] auto GetTotalPrimaryCount() const noexcept -> std::uint64_t {
    return total_primary_count_;
  }

private:
  GGEMSSourcePopulationPlan(
      GGEMSTimeWindow time_window,
      std::vector<GGEMSSourcePopulationPlanSource> sources,
      std::vector<GGEMSSourcePopulationPlanEmission> emissions,
      std::vector<
          std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
          radionuclide_definitions,
      std::uint64_t total_primary_count);

  GGEMSTimeWindow time_window_{};
  std::vector<GGEMSSourcePopulationPlanSource> sources_;
  std::vector<GGEMSSourcePopulationPlanEmission> emissions_;
  std::vector<std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
      radionuclide_definitions_;
  std::uint64_t total_primary_count_{0ULL};
};

class GGEMSSourcePopulationCandidate {
public:
  [[nodiscard]] static auto
  Create(std::shared_ptr<void const> owner_identity,
         std::uint64_t base_revision, GGEMSSourcePopulationPlan plan,
         std::vector<random::GGEMSHostRandomStream> candidate_streams)
      -> GGEMSSourcePopulationCandidate;

  ~GGEMSSourcePopulationCandidate() = default;

  GGEMSSourcePopulationCandidate(GGEMSSourcePopulationCandidate const &) =
      delete;
  GGEMSSourcePopulationCandidate(
      GGEMSSourcePopulationCandidate &&other) noexcept;
  auto operator=(GGEMSSourcePopulationCandidate const &)
      -> GGEMSSourcePopulationCandidate & = delete;
  auto operator=(GGEMSSourcePopulationCandidate &&other) noexcept
      -> GGEMSSourcePopulationCandidate &;

  [[nodiscard]] auto GetPlan() const noexcept
      -> GGEMSSourcePopulationPlan const & {
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
  GGEMSSourcePopulationCandidate(
      std::shared_ptr<void const> owner_identity, std::uint64_t base_revision,
      GGEMSSourcePopulationPlan plan,
      std::vector<random::GGEMSHostRandomStream> candidate_streams);

  std::shared_ptr<void const> owner_identity_;
  std::uint64_t base_revision_{0ULL};
  bool committed_{false};
  GGEMSSourcePopulationPlan plan_;
  std::vector<random::GGEMSHostRandomStream> candidate_streams_;
};

class GGEMSSourcePopulationPlanner {
public:
  GGEMSSourcePopulationPlanner(
      std::span<std::shared_ptr<sources::GGEMSSource> const> sources,
      random::GGEMSRandom const &random);
  ~GGEMSSourcePopulationPlanner() = default;

  GGEMSSourcePopulationPlanner(GGEMSSourcePopulationPlanner const &) = delete;
  GGEMSSourcePopulationPlanner(GGEMSSourcePopulationPlanner &&) = delete;
  auto operator=(GGEMSSourcePopulationPlanner const &)
      -> GGEMSSourcePopulationPlanner & = delete;
  auto operator=(GGEMSSourcePopulationPlanner &&)
      -> GGEMSSourcePopulationPlanner & = delete;

  [[nodiscard]] auto BuildCandidate(GGEMSTimeWindow time_window) const
      -> GGEMSSourcePopulationCandidate;

  auto CommitCandidate(GGEMSSourcePopulationCandidate &candidate) -> void;

  [[nodiscard]] auto GetRevision() const noexcept -> std::uint64_t {
    return revision_;
  }

private:
  struct StableSourceSlot {
    std::shared_ptr<GGEMSSource> source;
    GGEMSSourcePopulationMode population_mode{
        GGEMSSourcePopulationMode::CountDriven};
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>
        radionuclide;
    std::uint64_t first_stream_id{0ULL};
    std::uint64_t emission_count{0ULL};
  };

  std::vector<StableSourceSlot> source_slots_;
  std::vector<random::GGEMSHostRandomStream> persistent_streams_;
  std::shared_ptr<void const> owner_identity_;
  std::uint64_t revision_{0ULL};
};

} // namespace ggems::core::sources
