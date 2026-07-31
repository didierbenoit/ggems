#pragma once

#include <cstdint>
#include <vector>

namespace ggems::core::transport {

struct GGEMSTransportWorkloadPlan {
  std::uint32_t workload_index{0U};
  std::uint32_t context_index{0U};
  std::uint64_t primary_count{0ULL};
  std::uint32_t worker_count{0U};
  std::uint64_t projection_history_offset{0ULL};
  std::uint64_t device_primary_offset{0ULL};
};

struct GGEMSTransportChunk {
  std::uint32_t primary_count{0U};
  std::uint64_t device_primary_offset{0ULL};
};

class GGEMSTransportChunkIterator {
public:
  GGEMSTransportChunkIterator(std::uint64_t device_primary_offset,
                              std::uint64_t primary_count,
                              std::uint32_t launch_primary_count_limit);

  [[nodiscard]] auto HasNext() const noexcept -> bool {
    return remaining_primary_count_ != 0ULL;
  }

  [[nodiscard]] auto Next() -> GGEMSTransportChunk;

private:
  std::uint64_t next_device_primary_offset_{0ULL};
  std::uint64_t remaining_primary_count_{0ULL};
  std::uint32_t launch_primary_count_limit_{0U};
};

[[nodiscard]] auto
ComputeSafeTransportLaunchPrimaryCount(std::uint32_t worker_count)
    -> std::uint32_t;

[[nodiscard]] auto BuildEqualTransportWorkloadPlan(
    std::uint64_t projection_history_offset, std::uint64_t total_primary_count,
    std::uint32_t workload_count, std::uint32_t worker_count_per_workload)
    -> std::vector<GGEMSTransportWorkloadPlan>;

[[nodiscard]] auto CountAssignedPrimaries(
    std::vector<GGEMSTransportWorkloadPlan> const &workload_plan)
    -> std::uint64_t;

} // namespace ggems::core::transport
