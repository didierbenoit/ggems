#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "GGEMS/core/particles/GGEMSParticleState.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::transport {

struct GGEMSDummyTransportRunConfig {
  std::uint32_t total_primary_count{4096U};

  std::uint64_t projection_history_offset{0ULL};
  std::uint64_t device_primary_offset{0ULL};

  sources::GGEMSSourceRecord source_record{};
  observer::GGEMSObserverConfigRecord observer_config{};

  std::uint64_t min_energy_milli_eV{10'000'000ULL};

  std::uint32_t max_generation{6U};
  std::uint32_t max_steps_per_track{12U};
};

struct GGEMSDummyTransportRunReport {
  std::uint32_t context_index{0U};
  std::string device_name{};

  GGEMSTransportCounters counters{};
  observer::GGEMSObserverCounters observer_counters{};

  ggems::units::Time host_time{0U};
  ggems::units::Time kernel_time{0U};
  ggems::units::Time command_time{0U};

  double host_histories_per_second{0.0};
  double kernel_histories_per_second{0.0};

  double host_terminal_particles_per_second{0.0};
  double kernel_terminal_particles_per_second{0.0};
};

class GGEMSDummyTransportWorkload {
public:
  GGEMSDummyTransportWorkload(ggems::ocl::GGEMSOpenCLContext &context,
                              std::filesystem::path kernel_root,
                              random::GGEMSRandom const &random,
                              std::uint32_t worker_count,
                              std::uint64_t random_stream_offset = 0ULL,
                              std::uint32_t context_index = 0U,
                              std::uint32_t observer_record_capacity = 1U);

  ~GGEMSDummyTransportWorkload() = default;

  GGEMSDummyTransportWorkload(GGEMSDummyTransportWorkload const &) = delete;
  GGEMSDummyTransportWorkload(GGEMSDummyTransportWorkload &&) = delete;
  GGEMSDummyTransportWorkload &
  operator=(GGEMSDummyTransportWorkload const &) = delete;
  GGEMSDummyTransportWorkload &
  operator=(GGEMSDummyTransportWorkload &&) = delete;

public:
  GGEMSDummyTransportRunReport Run(GGEMSDummyTransportRunConfig const &config);

  [[nodiscard]] GGEMSTransportCounters ReadCountersOnHost();

  [[nodiscard]] observer::GGEMSObserverCounters ReadObserverCountersOnHost();

  [[nodiscard]] std::uint32_t GetWorkerCount() const noexcept {
    return worker_count_;
  }

private:
  void InitialiseRandomStatesOnHost();
  void ResetCountersOnHost();
  void ClearWorkerFinalStatesOnHost();
  void WriteSourceRecordOnHost(sources::GGEMSSourceRecord const &source_record);
  void ResetObserverOnHost();
  void WriteObserverConfigOnHost(
      observer::GGEMSObserverConfigRecord const &observer_config);

private:
  ggems::ocl::GGEMSOpenCLContext *context_{nullptr};
  std::filesystem::path kernel_root_{};
  random::GGEMSRandom const *random_{nullptr};

  std::uint32_t worker_count_{0U};
  std::uint64_t random_stream_offset_{0ULL};

  std::uint32_t context_index_{0U};
  std::string device_name_{};

  std::uint32_t observer_record_capacity_{1U};

  ggems::ocl::GGEMSOpenCLSVMBuffer random_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer worker_final_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer counters_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer source_record_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_config_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_counters_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_records_buffer_;
};

} // namespace ggems::core::transport
