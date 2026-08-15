#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceEmissionRange.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::sources {
class GGEMSSourceConfigurationSnapshot;
}

namespace ggems::core::transport {

struct GGEMSTransportRunConfig {
  std::uint64_t run_id{0ULL};
  std::uint64_t total_primary_count{4096ULL};
  std::uint64_t projection_history_offset{0ULL};
  std::uint64_t device_primary_offset{0ULL};
  std::vector<sources::GGEMSSourceRecord> source_records;
  std::vector<sources::GGEMSSourcePopulationRecord> source_population_records;
  std::vector<sources::GGEMSSourceRunRange> source_ranges;
  std::vector<sources::GGEMSSourceEmissionRange> source_emission_ranges;
  observer::GGEMSObserverConfigRecord observer_config{};
};

struct GGEMSTransportLogicalCounters {
  std::uint64_t next_primary_id{0ULL};
  std::uint64_t consumed_primary_count{0ULL};
  std::uint64_t completed_history_count{0ULL};
  std::uint64_t terminal_particle_count{0ULL};
  std::uint64_t created_secondary_count{0ULL};
  std::uint64_t aionino_to_gamma_count{0ULL};
  std::uint64_t gamma_to_electron_count{0ULL};
  std::uint64_t electron_to_electron_count{0ULL};
  std::uint64_t overflow_count{0ULL};
  std::uint64_t max_stack_depth{0ULL};
  std::uint64_t total_fake_step_count{0ULL};
};

struct GGEMSObserverLogicalCounters {
  std::uint64_t record_count{0ULL};
  std::uint64_t overflow_count{0ULL};
  std::uint64_t captured_primary_count{0ULL};
};

struct GGEMSTransportRunReport {
  std::uint32_t context_index{0U};
  std::string device_name;
  GGEMSTransportLogicalCounters counters{};
  observer::GGEMSObserverCounters observer_counters{};
  std::vector<observer::GGEMSObserverRecord> observer_records;
  GGEMSObserverLogicalCounters logical_observer_counters{};
  ggems::units::Time host_time{0U};
  ggems::units::Time kernel_time{0U};
  ggems::units::Time command_time{0U};
  double host_histories_per_second{0.0};
  double kernel_histories_per_second{0.0};
  double host_terminal_particles_per_second{0.0};
  double kernel_terminal_particles_per_second{0.0};
};

auto ValidateTransportRunConfig(GGEMSTransportRunConfig const &config,
                                std::uint32_t stable_source_count,
                                std::uint32_t stable_emission_count,
                                std::uint32_t worker_count,
                                std::uint32_t launch_primary_count_limit)
    -> void;

class GGEMSTransportWorkload {
public:
  GGEMSTransportWorkload(
      ggems::ocl::GGEMSOpenCLContext &context,
      std::filesystem::path kernel_root, random::GGEMSRandom const &random,
      std::uint32_t worker_count,
      sources::GGEMSSourceConfigurationSnapshot const &source_configuration,
      std::uint64_t random_stream_offset = 0ULL,
      std::uint32_t context_index = 0U,
      std::uint32_t observer_record_capacity = 1U,
      std::uint32_t launch_primary_count_limit = 0U);

  ~GGEMSTransportWorkload() = default;

  GGEMSTransportWorkload(GGEMSTransportWorkload const &) = delete;
  GGEMSTransportWorkload(GGEMSTransportWorkload &&) = delete;
  auto operator=(GGEMSTransportWorkload const &)
      -> GGEMSTransportWorkload & = delete;
  auto operator=(GGEMSTransportWorkload &&)
      -> GGEMSTransportWorkload & = delete;

  auto Run(GGEMSTransportRunConfig const &config) -> GGEMSTransportRunReport;

  auto ValidateRunConfig(GGEMSTransportRunConfig const &config) const -> void;

  [[nodiscard]] auto ReadCountersFromSVM() -> GGEMSTransportCounters;

  [[nodiscard]] auto ReadObserverCountersFromSVM()
      -> observer::GGEMSObserverCounters;

  [[nodiscard]] auto ReadObserverRecordsFromSVM(std::uint32_t record_count)
      -> std::vector<observer::GGEMSObserverRecord>;

  [[nodiscard]] auto GetWorkerCount() const noexcept -> std::uint32_t {
    return worker_count_;
  }

private:
  auto InitializeRandomStatesInSVM() -> void;
  auto ResetCountersInSVM() -> void;
  auto ResetObserverCountersInSVM() -> void;
  auto WriteObserverConfigToSVM(
      observer::GGEMSObserverConfigRecord const &observer_config) -> void;

  ggems::ocl::GGEMSOpenCLContext *context_{nullptr};
  std::filesystem::path kernel_root_{};
  random::GGEMSRandom const *random_{nullptr};
  std::string random_kernel_build_definition_;

  std::uint32_t worker_count_{0U};
  std::uint32_t source_count_{0U};
  std::uint32_t emission_count_{0U};
  std::uint64_t random_stream_offset_{0ULL};
  std::uint32_t context_index_{0U};
  std::string device_name_;
  std::uint32_t observer_record_capacity_{1U};
  std::uint32_t launch_primary_count_limit_{0U};

  ggems::ocl::GGEMSOpenCLSVMBuffer random_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer counters_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_records_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_population_records_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_ranges_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_emission_records_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_emission_ranges_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer energy_distribution_records_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer energy_values_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer cumulative_ticket_upper_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_config_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_counters_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_records_buffer_;
  std::unique_ptr<ggems::ocl::GGEMSOpenCLKernel> kernel_;
};

} // namespace ggems::core::transport
