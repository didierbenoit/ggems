#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "GGEMS/core/particles/GGEMSParticleState.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

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

  std::uint64_t initial_energy_milli_eV{511000000ULL};
  std::uint64_t min_energy_milli_eV{10000000ULL};

  std::uint32_t max_generation{6U};
  std::uint32_t max_steps_per_track{12U};
};

struct GGEMSDummyTransportRunReport {
  std::uint32_t context_index{0U};
  std::string device_name{};

  GGEMSTransportCounters counters{};

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
                              std::uint32_t context_index = 0U);

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

  [[nodiscard]] std::uint32_t GetWorkerCount() const noexcept {
    return worker_count_;
  }

private:
  void InitialiseRandomStatesOnHost();
  void ResetCountersOnHost();
  void ClearWorkerFinalStatesOnHost();

private:
  ggems::ocl::GGEMSOpenCLContext *context_{nullptr};
  std::filesystem::path kernel_root_{};
  random::GGEMSRandom const *random_{nullptr};

  std::uint32_t worker_count_{0U};
  std::uint64_t random_stream_offset_{0ULL};

  std::uint32_t context_index_{0U};
  std::string device_name_{};

  ggems::ocl::GGEMSOpenCLSVMBuffer random_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer worker_final_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer counters_buffer_;
};

} // namespace ggems::core::transport
