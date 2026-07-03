#pragma once

#include <cstdint>
#include <vector>

namespace ggems::core::transport {

struct GGEMSTransportWorkloadPlan {
  std::uint32_t workload_index{0U};
  std::uint32_t context_index{0U};
  std::uint32_t primary_count{0U};
  std::uint32_t worker_count{0U};
  std::uint64_t projection_history_offset{0ULL};
  std::uint64_t device_primary_offset{0ULL};
};

[[nodiscard]] std::vector<GGEMSTransportWorkloadPlan>
BuildEqualTransportWorkloadPlan(std::uint64_t projection_history_offset,
                                std::uint32_t total_primary_count,
                                std::uint32_t workload_count,
                                std::uint32_t worker_count_per_workload);

[[nodiscard]] std::uint64_t CountAssignedPrimaries(
    std::vector<GGEMSTransportWorkloadPlan> const &workload_plan) noexcept;

} // namespace ggems::core::transport
