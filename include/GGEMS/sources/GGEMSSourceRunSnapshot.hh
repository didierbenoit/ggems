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

using GGEMSSourceConfigurationSnapshotPtr =
    std::shared_ptr<GGEMSSourceConfigurationSnapshot const>;

[[nodiscard]] auto BuildSourceConfigurationSnapshot(GGEMSSource const &source)
    -> GGEMSSourceConfigurationSnapshotPtr;

[[nodiscard]] auto BuildSourceConfigurationSnapshot(
    std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceConfigurationSnapshotPtr;

[[nodiscard]] auto BuildSourceRunSnapshot(GGEMSSource const &source)
    -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto BuildSourceRunSnapshot(GGEMSSource const &source,
                                          GGEMSTimeWindow time_window)
    -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSSourceConfigurationSnapshotPtr source_configuration)
    -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSSourceConfigurationSnapshotPtr source_configuration,
                       GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSSourceConfigurationSnapshotPtr source_configuration,
                       GGEMSSourcePopulationPlan const &population_plan)
    -> GGEMSSourceRunSnapshot;

class GGEMSSourceConfigurationSnapshot {
public:
  [[nodiscard]] static auto Create(GGEMSSource const &source)
      -> GGEMSSourceConfigurationSnapshotPtr;

  [[nodiscard]] static auto
  Create(std::span<std::shared_ptr<GGEMSSource> const> sources)
      -> GGEMSSourceConfigurationSnapshotPtr;

  ~GGEMSSourceConfigurationSnapshot() = default;

  GGEMSSourceConfigurationSnapshot(GGEMSSourceConfigurationSnapshot const &) =
      delete;
  GGEMSSourceConfigurationSnapshot(GGEMSSourceConfigurationSnapshot &&) =
      delete;
  auto operator=(GGEMSSourceConfigurationSnapshot const &)
      -> GGEMSSourceConfigurationSnapshot & = delete;
  auto operator=(GGEMSSourceConfigurationSnapshot &&)
      -> GGEMSSourceConfigurationSnapshot & = delete;

  [[nodiscard]] auto GetEnergyDistributionRecords() const noexcept
      -> std::vector<GGEMSEnergyDistributionRecord> const & {
    return energy_distribution_records_;
  }

  [[nodiscard]] auto GetEnergyValuesMicroElectronVolt() const noexcept
      -> std::vector<std::uint64_t> const & {
    return energy_values_micro_eV_;
  }

  [[nodiscard]] auto GetRelativeWeights() const noexcept
      -> std::vector<double> const & {
    return relative_weights_;
  }

  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
      -> std::vector<std::uint64_t> const & {
    return cumulative_ticket_upper_;
  }

  [[nodiscard]] auto GetEmissionRecords() const noexcept
      -> std::vector<GGEMSSourceEmissionRecord> const & {
    return source_emission_records_;
  }

  [[nodiscard]] auto GetRadionuclideDefinitions() const noexcept -> std::vector<
      std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>> const
      & {
    return radionuclide_definitions_;
  }

  [[nodiscard]] auto GetSourceCount() const noexcept -> std::size_t {
    return source_count_;
  }

  [[nodiscard]] auto GetEmissionCount() const noexcept -> std::size_t {
    return source_emission_records_.size();
  }

private:
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

  std::size_t source_count_{0U};
  std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records_;
  std::vector<std::uint64_t> energy_values_micro_eV_;
  std::vector<double> relative_weights_;
  std::vector<std::uint64_t> cumulative_ticket_upper_;
  std::vector<GGEMSSourceEmissionRecord> source_emission_records_;
  std::vector<std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
      radionuclide_definitions_;
};

class GGEMSSourceRunSnapshot {
public:
  [[nodiscard]] static auto Create(GGEMSSource const &source,
                                   GGEMSTimeWindow time_window)
      -> GGEMSSourceRunSnapshot;

  [[nodiscard]] static auto
  Create(std::span<std::shared_ptr<GGEMSSource> const> sources,
         GGEMSSourceConfigurationSnapshotPtr source_configuration,
         GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot;

  [[nodiscard]] static auto
  Create(std::span<std::shared_ptr<GGEMSSource> const> sources,
         GGEMSSourceConfigurationSnapshotPtr source_configuration,
         GGEMSSourcePopulationPlan const &population_plan)
      -> GGEMSSourceRunSnapshot;

  ~GGEMSSourceRunSnapshot() = default;

  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot const &) = default;
  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot &&) = default;
  auto operator=(GGEMSSourceRunSnapshot const &)
      -> GGEMSSourceRunSnapshot & = default;
  auto operator=(GGEMSSourceRunSnapshot &&)
      -> GGEMSSourceRunSnapshot & = default;

  [[nodiscard]] auto GetRecords() const noexcept
      -> std::vector<GGEMSSourceRecord> const & {
    return records_;
  }

  [[nodiscard]] auto GetRanges() const noexcept
      -> std::vector<GGEMSSourceRunRange> const & {
    return ranges_;
  }

  [[nodiscard]] auto GetPopulationRecords() const noexcept
      -> std::vector<GGEMSSourcePopulationRecord> const & {
    return population_records_;
  }

  [[nodiscard]] auto GetGroupRanges() const noexcept
      -> std::vector<GGEMSSourceEmissionRange> const & {
    return emission_ranges_;
  }

  [[nodiscard]] auto GetTimeWindow() const noexcept -> GGEMSTimeWindow {
    return time_window_;
  }

  [[nodiscard]] auto GetSourceConfiguration() const noexcept
      -> GGEMSSourceConfigurationSnapshot const & {
    return *source_configuration_;
  }

  [[nodiscard]] auto GetEnergyDistributionRecords() const noexcept
      -> std::vector<GGEMSEnergyDistributionRecord> const & {
    return source_configuration_->GetEnergyDistributionRecords();
  }

  [[nodiscard]] auto GetEnergyValuesMicroElectronVolt() const noexcept
      -> std::vector<std::uint64_t> const & {
    return source_configuration_->GetEnergyValuesMicroElectronVolt();
  }

  [[nodiscard]] auto GetRelativeWeights() const noexcept
      -> std::vector<double> const & {
    return source_configuration_->GetRelativeWeights();
  }

  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
      -> std::vector<std::uint64_t> const & {
    return source_configuration_->GetCumulativeTicketUpperBounds();
  }

  [[nodiscard]] auto GetEmissionRecords() const noexcept
      -> std::vector<GGEMSSourceEmissionRecord> const & {
    return source_configuration_->GetEmissionRecords();
  }

  [[nodiscard]] auto GetRadionuclideDefinitions() const noexcept -> std::vector<
      std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>> const
      & {
    return source_configuration_->GetRadionuclideDefinitions();
  }

  [[nodiscard]] auto GetTotalPrimaryCount() const noexcept -> std::uint64_t {
    return total_primary_count_;
  }

  [[nodiscard]] auto HasActivityDrivenSource() const noexcept -> bool;

private:
  GGEMSSourceRunSnapshot(
      std::vector<GGEMSSourceRecord> records,
      std::vector<GGEMSSourceRunRange> ranges,
      std::vector<GGEMSSourcePopulationRecord> population_records,
      std::vector<GGEMSSourceEmissionRange> emission_ranges,
      GGEMSTimeWindow time_window,
      GGEMSSourceConfigurationSnapshotPtr source_configuration,
      std::uint64_t total_primary_count);

  std::vector<GGEMSSourceRecord> records_;
  std::vector<GGEMSSourceRunRange> ranges_;
  std::vector<GGEMSSourcePopulationRecord> population_records_;
  std::vector<GGEMSSourceEmissionRange> emission_ranges_;
  GGEMSTimeWindow time_window_;
  GGEMSSourceConfigurationSnapshotPtr source_configuration_;
  std::uint64_t total_primary_count_{0ULL};
};
} // namespace ggems::core::sources
