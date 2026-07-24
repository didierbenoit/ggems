#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::transport {

struct GGEMSTransportRunConfig {
  std::uint64_t run_id{0ULL};
  std::uint32_t total_primary_count{4096U};
  std::uint64_t projection_history_offset{0ULL};
  std::uint64_t device_primary_offset{0ULL};
  std::vector<sources::GGEMSSourceRecord> source_records;
  std::vector<sources::GGEMSSourceRunRange> source_ranges;
  observer::GGEMSObserverConfigRecord observer_config{};
};

struct GGEMSTransportRunReport {
  std::uint32_t context_index{0U};
  std::string device_name;
  GGEMSTransportCounters counters{};
  observer::GGEMSObserverCounters observer_counters{};
  std::vector<observer::GGEMSObserverRecord> observer_records;
  ggems::units::Time host_time{0U};
  ggems::units::Time kernel_time{0U};
  ggems::units::Time command_time{0U};
  double host_histories_per_second{0.0};
  double kernel_histories_per_second{0.0};
  double host_terminal_particles_per_second{0.0};
  double kernel_terminal_particles_per_second{0.0};
};

class GGEMSTransportWorkload {
public:
  GGEMSTransportWorkload(ggems::ocl::GGEMSOpenCLContext &context,
                         std::filesystem::path kernel_root,
                         random::GGEMSRandom const &random,
                         std::uint32_t worker_count, std::uint32_t source_count,
                         std::uint64_t random_stream_offset = 0ULL,
                         std::uint32_t context_index = 0U,
                         std::uint32_t observer_record_capacity = 1U);

  ~GGEMSTransportWorkload() = default;

  GGEMSTransportWorkload(GGEMSTransportWorkload const &) = delete;
  GGEMSTransportWorkload(GGEMSTransportWorkload &&) = delete;
  auto operator=(GGEMSTransportWorkload const &)
      -> GGEMSTransportWorkload & = delete;
  auto operator=(GGEMSTransportWorkload &&)
      -> GGEMSTransportWorkload & = delete;

  auto Run(GGEMSTransportRunConfig const &config) -> GGEMSTransportRunReport;

  [[nodiscard]] auto ReadCountersFromSVM() -> GGEMSTransportCounters;

  [[nodiscard]] auto ReadObserverCountersFromSVM()
      -> observer::GGEMSObserverCounters;

  [[nodiscard]] auto ReadObserverRecordsFromSVM(std::uint32_t record_count)
      -> std::vector<observer::GGEMSObserverRecord>;

  [[nodiscard]] auto GetWorkerCount() const noexcept -> std::uint32_t {
    return worker_count_;
  }

private:
  auto InitialiseRandomStatesInSVM() -> void;
  auto ResetCountersInSVM() -> void;
  auto ResetObserverInSVM() -> void;
  auto WriteObserverConfigToSVM(
      observer::GGEMSObserverConfigRecord const &observer_config) -> void;

  ggems::ocl::GGEMSOpenCLContext *context_{nullptr};
  std::filesystem::path kernel_root_{};
  random::GGEMSRandom const *random_{nullptr};

  std::uint32_t worker_count_{0U};
  std::uint32_t source_count_{0U};
  std::uint64_t random_stream_offset_{0ULL};
  std::uint32_t context_index_{0U};
  std::string device_name_;
  std::uint32_t observer_record_capacity_{1U};

  ggems::ocl::GGEMSOpenCLSVMBuffer random_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer counters_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_records_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_ranges_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_config_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_counters_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_records_buffer_;
};

} // namespace ggems::core::transport
