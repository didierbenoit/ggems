#include "GGEMS/core/transport/GGEMSTransportWorkloadPlan.hh"

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::core::transport {

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::vector<GGEMSTransportWorkloadPlan> BuildEqualTransportWorkloadPlan(
    std::uint64_t projection_history_offset, std::uint32_t total_primary_count,
    std::uint32_t workload_count, std::uint32_t worker_count_per_workload) {
  GGEMS_CHECK_RECOVERABLE(total_primary_count > 0U,
                          "Cannot build a transport workload plan with zero"
                          "primary particles.");

  GGEMS_CHECK_RECOVERABLE(workload_count > 0U,
                          "Cannot build a transport workload plan with zero "
                          "workloads.");

  GGEMS_CHECK_RECOVERABLE(worker_count_per_workload > 0U,
                          "Cannot build a transport workload plan with zero "
                          "workers per workload.");

  std::vector<GGEMSTransportWorkloadPlan> workload_plan;
  workload_plan.reserve(workload_count);

  std::uint32_t base_primary_count = total_primary_count / workload_count;
  std::uint32_t remainder = total_primary_count % workload_count;

  std::uint64_t device_primary_offset{0ULL};

  for (std::uint32_t workload_index = 0U; workload_index < workload_count;
       ++workload_index) {
    std::uint32_t workload_primary_count =
        base_primary_count + (workload_index < remainder ? 1U : 0U);

    workload_plan.push_back(GGEMSTransportWorkloadPlan{
        .workload_index = workload_index,
        .context_index = workload_index,
        .primary_count = workload_primary_count,
        .worker_count = worker_count_per_workload,
        .projection_history_offset = projection_history_offset,
        .device_primary_offset = device_primary_offset});

    device_primary_offset += workload_primary_count;
  }

  GGEMS_CHECK_RECOVERABLE(CountAssignedPrimaries(workload_plan) ==
                              total_primary_count,
                          "Transport workload plan assigned primary count does "
                          "not match requested primary count.");

  return workload_plan;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint64_t CountAssignedPrimaries(
    std::vector<GGEMSTransportWorkloadPlan> const &workload_plan) noexcept {
  std::uint64_t assigned_primary_count{0ULL};

  for (GGEMSTransportWorkloadPlan const &workload : workload_plan) {
    assigned_primary_count += workload.primary_count;
  }

  return assigned_primary_count;
}

} // namespace ggems::core::transport
