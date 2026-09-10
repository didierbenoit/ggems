#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

inline constexpr std::uint64_t k_energy_ticket_space_size{std::uint64_t{1U}
                                                          << 32U};

class GGEMSEnergyDistribution {
public:
  GGEMSEnergyDistribution() = default;
  ~GGEMSEnergyDistribution() = default;

  GGEMSEnergyDistribution(GGEMSEnergyDistribution const &) = default;
  GGEMSEnergyDistribution(GGEMSEnergyDistribution &&) noexcept = default;
  auto operator=(GGEMSEnergyDistribution const &)
      -> GGEMSEnergyDistribution & = default;
  auto operator=(GGEMSEnergyDistribution &&) noexcept
      -> GGEMSEnergyDistribution & = default;

  [[nodiscard]] static auto BuildMono(std::uint64_t energy_micro_eV)
      -> GGEMSEnergyDistribution;

  [[nodiscard]] static auto
  BuildDiscreteLines(std::span<double const> energies,
                     std::span<double const> relative_weights,
                     std::string_view unit) -> GGEMSEnergyDistribution;

  [[nodiscard]] static auto
  BuildRegularSpectrum(std::span<double const> bin_centers,
                       std::span<double const> relative_weights,
                       std::string_view unit) -> GGEMSEnergyDistribution;

  // Exact canonical micro-electronvolt tables; no runtime unit token needed.
  [[nodiscard]] static auto
  BuildDiscreteLines(std::span<std::uint64_t const> energies_micro_eV,
                     std::span<double const> relative_weights)
      -> GGEMSEnergyDistribution;

  [[nodiscard]] static auto
  BuildRegularSpectrum(std::span<std::uint64_t const> bin_centers_micro_eV,
                       std::span<double const> relative_weights)
      -> GGEMSEnergyDistribution;

  [[nodiscard]] static auto
  LoadRegularSpectrum(std::filesystem::path const &filename,
                      std::string_view unit) -> GGEMSEnergyDistribution;

  [[nodiscard]] auto GetType() const noexcept -> GGEMSEnergyDistributionType {
    return type_;
  }

  [[nodiscard]] auto GetMonoEnergyMicroElectronVolt() const noexcept
      -> std::uint64_t {
    return mono_energy_micro_eV_;
  }

  [[nodiscard]] auto GetRegularBinWidthMicroElectronVolt() const noexcept
      -> std::uint64_t {
    return regular_bin_width_micro_eV_;
  }

  [[nodiscard]] auto GetTableCount() const noexcept -> std::uint32_t {
    return static_cast<std::uint32_t>(energy_values_micro_eV_.size());
  }

  [[nodiscard]] auto BuildRecord(std::uint64_t table_offset) const noexcept
      -> GGEMSEnergyDistributionRecord;

  [[nodiscard]] auto GetEnergyValuesMicroElectronVolt() const noexcept
      -> std::span<std::uint64_t const> {
    return energy_values_micro_eV_;
  }

  [[nodiscard]] auto GetRelativeWeights() const noexcept
      -> std::span<double const> {
    return relative_weights_;
  }

  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
      -> std::span<std::uint64_t const> {
    return cumulative_ticket_upper_;
  }

private:
  GGEMSEnergyDistribution(GGEMSEnergyDistributionType type,
                          std::uint64_t mono_energy_micro_eV,
                          std::uint64_t regular_bin_width_micro_eV,
                          std::vector<std::uint64_t> energy_values_micro_eV,
                          std::vector<double> relative_weights,
                          std::vector<std::uint64_t> cumulative_ticket_upper);

  [[nodiscard]] static auto BuildRegularSpectrumWithContext(
      std::span<double const> bin_centers,
      std::span<double const> relative_bin_weights, std::string_view unit,
      std::string_view filename, std::span<std::size_t const> line_numbers)
      -> GGEMSEnergyDistribution;

  [[nodiscard]] static auto
  BuildDiscreteLinesFromValues(std::vector<std::uint64_t> energy_values,
                               std::span<double const> relative_weights)
      -> GGEMSEnergyDistribution;

  [[nodiscard]] static auto BuildRegularSpectrumFromValues(
      std::vector<std::uint64_t> energy_values,
      std::span<double const> relative_bin_weights, std::string_view filename,
      std::span<std::size_t const> line_numbers) -> GGEMSEnergyDistribution;

  GGEMSEnergyDistributionType type_{GGEMSEnergyDistributionType::Mono};
  std::uint64_t mono_energy_micro_eV_{511'000'000'000ULL};
  std::uint64_t regular_bin_width_micro_eV_{0ULL};
  std::vector<std::uint64_t> energy_values_micro_eV_;
  std::vector<double> relative_weights_;
  std::vector<std::uint64_t> cumulative_ticket_upper_;
};
} // namespace ggems::core::sources
