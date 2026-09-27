#include <algorithm>
#include <limits>
#include <cstdint>
#include <vector>

#include "GGEMS/transport/GGEMSTransportWorkloadPlan.hh"
#include "GGEMS/GGEMSException.hh"

namespace ggems::core::transport {

// =============================================================================
// =============================================================================

GGEMSTransportChunkIterator::GGEMSTransportChunkIterator(
  std::uint64_t device_primary_offset, std::uint64_t primary_count,
  std::uint32_t launch_primary_count_limit)
    : next_device_primary_offset_{device_primary_offset},
      remaining_primary_count_{primary_count},
      launch_primary_count_limit_{launch_primary_count_limit} {}

// ----------------------------------------------------------------------------

auto GGEMSTransportChunkIterator::Next() -> GGEMSTransportChunk {
  if (!HasNext()) {
    throw ggems::core::GGEMSRecoverable(
      "Transport chunk iterator is exhausted.");
  }

  std::uint64_t const chunk_count_u64 =
    std::min(remaining_primary_count_,
             static_cast<std::uint64_t>(launch_primary_count_limit_));

  auto const chunk_count = static_cast<std::uint32_t>(chunk_count_u64);

  GGEMSTransportChunk chunk{
    .primary_count = chunk_count,
    .device_primary_offset = next_device_primary_offset_,
  };

  remaining_primary_count_ -= chunk_count_u64;

  if (remaining_primary_count_ != 0ULL) {
    next_device_primary_offset_ += chunk_count_u64;
  }

  return chunk;
}

// =============================================================================
// =============================================================================

auto ComputeSafeTransportLaunchPrimaryCount(std::uint32_t worker_count) noexcept
  -> std::uint32_t {
  return std::numeric_limits<std::uint32_t>::max() - worker_count;
}

// =============================================================================
// =============================================================================

auto BuildEqualTransportWorkloadPlan(std::uint64_t projection_history_offset,
                                     std::uint64_t total_primary_count,
                                     std::uint32_t workload_count,
                                     std::uint32_t worker_count_per_workload)
  -> std::vector<GGEMSTransportWorkloadPlan> {
  std::vector<GGEMSTransportWorkloadPlan> workload_plan;
  workload_plan.reserve(workload_count);

  std::uint64_t const workload_count_u64 = workload_count;
  std::uint64_t const base_primary_count =
    total_primary_count / workload_count_u64;
  std::uint64_t const remainder = total_primary_count % workload_count_u64;

  std::uint64_t device_primary_offset{0ULL};

  for (std::uint32_t workload_index = 0U; workload_index < workload_count;
       ++workload_index) {
    std::uint64_t const workload_primary_count =
      base_primary_count +
      (static_cast<std::uint64_t>(workload_index) < remainder ? 1ULL : 0ULL);

    workload_plan.push_back(GGEMSTransportWorkloadPlan{
      .workload_index = workload_index,
      .context_index = workload_index,
      .primary_count = workload_primary_count,
      .worker_count = worker_count_per_workload,
      .projection_history_offset = projection_history_offset,
      .device_primary_offset = device_primary_offset,
    });

    device_primary_offset += workload_primary_count;
  }

  return workload_plan;
}

// =============================================================================
// =============================================================================

auto CountAssignedPrimaries(
  std::vector<GGEMSTransportWorkloadPlan> const &workload_plan)
  -> std::uint64_t {
  std::uint64_t assigned_primary_count{0ULL};

  for (GGEMSTransportWorkloadPlan const &workload : workload_plan) {
    assigned_primary_count += workload.primary_count;
  }

  return assigned_primary_count;
}

} // namespace ggems::core::transport
