#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"

namespace ggems::core::sources {

class GGEMSSource;
class GGEMSSourceConfigurationSnapshot;
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

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources,
                       GGEMSSourceConfigurationSnapshotPtr source_configuration)
    -> GGEMSSourceRunSnapshot;

class GGEMSSourceConfigurationSnapshot {
public:
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

  [[nodiscard]] auto GetEnergyValuesMilliElectronVolt() const noexcept
      -> std::vector<std::uint64_t> const & {
    return energy_values_milli_eV_;
  }

  [[nodiscard]] auto GetRelativeWeights() const noexcept
      -> std::vector<double> const & {
    return relative_weights_;
  }

  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
      -> std::vector<std::uint64_t> const & {
    return cumulative_ticket_upper_;
  }

  [[nodiscard]] auto GetSourceCount() const noexcept -> std::size_t {
    return energy_distribution_records_.size();
  }

private:
  friend auto BuildSourceConfigurationSnapshot(GGEMSSource const &source)
      -> GGEMSSourceConfigurationSnapshotPtr;

  friend auto BuildSourceConfigurationSnapshot(
      std::span<std::shared_ptr<GGEMSSource> const> sources)
      -> GGEMSSourceConfigurationSnapshotPtr;

  GGEMSSourceConfigurationSnapshot(
      std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records,
      std::vector<std::uint64_t> energy_values_milli_eV,
      std::vector<double> relative_weights,
      std::vector<std::uint64_t> cumulative_ticket_upper);

  std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records_;
  std::vector<std::uint64_t> energy_values_milli_eV_;
  std::vector<double> relative_weights_;
  std::vector<std::uint64_t> cumulative_ticket_upper_;
};

class GGEMSSourceRunSnapshot {
public:
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

  [[nodiscard]] auto GetSourceConfiguration() const noexcept
      -> GGEMSSourceConfigurationSnapshot const & {
    return *source_configuration_;
  }

  [[nodiscard]] auto GetEnergyDistributionRecords() const noexcept
      -> std::vector<GGEMSEnergyDistributionRecord> const & {
    return source_configuration_->GetEnergyDistributionRecords();
  }

  [[nodiscard]] auto GetEnergyValuesMilliElectronVolt() const noexcept
      -> std::vector<std::uint64_t> const & {
    return source_configuration_->GetEnergyValuesMilliElectronVolt();
  }

  [[nodiscard]] auto GetRelativeWeights() const noexcept
      -> std::vector<double> const & {
    return source_configuration_->GetRelativeWeights();
  }

  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
      -> std::vector<std::uint64_t> const & {
    return source_configuration_->GetCumulativeTicketUpperBounds();
  }

  [[nodiscard]] auto GetTotalPrimaryCount() const noexcept -> std::uint64_t {
    return total_primary_count_;
  }

private:
  friend auto BuildSourceRunSnapshot(GGEMSSource const &source)
      -> GGEMSSourceRunSnapshot;

  friend auto
  BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources)
      -> GGEMSSourceRunSnapshot;

  friend auto BuildSourceRunSnapshot(
      std::span<std::shared_ptr<GGEMSSource> const> sources,
      GGEMSSourceConfigurationSnapshotPtr source_configuration)
      -> GGEMSSourceRunSnapshot;

  GGEMSSourceRunSnapshot(
      std::vector<GGEMSSourceRecord> records,
      std::vector<GGEMSSourceRunRange> ranges,
      GGEMSSourceConfigurationSnapshotPtr source_configuration,
      std::uint64_t total_primary_count);

  std::vector<GGEMSSourceRecord> records_;
  std::vector<GGEMSSourceRunRange> ranges_;
  GGEMSSourceConfigurationSnapshotPtr source_configuration_;
  std::uint64_t total_primary_count_{0ULL};
};
} // namespace ggems::core::sources
