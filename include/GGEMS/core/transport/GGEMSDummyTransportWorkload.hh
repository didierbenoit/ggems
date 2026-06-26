#pragma once

#include <cstdint>
#include <filesystem>

#include "GGEMS/core/particles/GGEMSParticleState.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::transport {

struct GGEMSDummyTransportRunConfig {
  std::uint32_t total_primary_count{4096U};

  std::uint64_t initial_energy_milli_eV{511000000ULL};
  std::uint64_t min_energy_milli_eV{10000000ULL};

  std::uint32_t max_generation{6U};
  std::uint32_t max_steps_per_track{12U};
};

class GGEMSDummyTransportWorkload {
public:
  GGEMSDummyTransportWorkload(ggems::ocl::GGEMSOpenCLContext &context,
                              std::filesystem::path kernel_root,
                              random::GGEMSRandom const &random,
                              std::uint32_t worker_count);

  ~GGEMSDummyTransportWorkload() = default;

  GGEMSDummyTransportWorkload(GGEMSDummyTransportWorkload const &) = delete;
  GGEMSDummyTransportWorkload(GGEMSDummyTransportWorkload &&) = delete;
  GGEMSDummyTransportWorkload &
  operator=(GGEMSDummyTransportWorkload const &) = delete;
  GGEMSDummyTransportWorkload &
  operator=(GGEMSDummyTransportWorkload &&) = delete;

public:
  void Run(GGEMSDummyTransportRunConfig const &config);

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

  ggems::ocl::GGEMSOpenCLSVMBuffer random_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer worker_final_states_buffer_;
  ggems::ocl::GGEMSOpenCLSVMBuffer counters_buffer_;
};

} // namespace ggems::core::transport
