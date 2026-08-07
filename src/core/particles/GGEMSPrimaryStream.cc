#include "GGEMS/core/particles/GGEMSPrimaryStream.hh"

#include <limits>
#include <cstdint>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::core::particles {

// =============================================================================
// =============================================================================

auto GGEMSPrimaryStream::SetPrimaryCount(std::uint64_t primary_count) -> void {
  GGEMS_CHECK_RECOVERABLE(primary_count > 0ULL,
                          "Primary stream particle count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(
      !initialized_,
      "Primary stream particle count cannot be changed after initialize.");

  primary_count_ = primary_count;
}

// -----------------------------------------------------------------------------

auto GGEMSPrimaryStream::Initialize() -> void {
  GGEMS_CHECK_RECOVERABLE(
      !initialized_, "Primary stream cannot be initialized more than once.");

  GGEMS_CHECK_RECOVERABLE(primary_count_ > 0ULL,
                          "Cannot initialize an empty primary stream.");

  GGEMS_INFO("Core", "Primary Aionino stream initialized");

  next_global_primary_id_ = 0ULL;
  exhausted_ = false;
  initialized_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSPrimaryStream::PrepareRun(std::uint64_t run_id)
    -> GGEMSPrimaryStreamRunView {
  return PrepareRun(run_id, primary_count_);
}

// -----------------------------------------------------------------------------

auto GGEMSPrimaryStream::PrepareRun(std::uint64_t run_id,
                                    std::uint64_t primary_count)
    -> GGEMSPrimaryStreamRunView {
  GGEMS_CHECK_RECOVERABLE(
      initialized_,
      "Primary stream must be initialized before reserving a range");

  GGEMS_CHECK_RECOVERABLE(primary_count > 0ULL,
                          "Primary stream reservation count must be non-zero");

  GGEMS_CHECK_RECOVERABLE(
      !exhausted_,
      "Primary stream is exhausted; no global primary identifiers remain.");

  constexpr std::uint64_t k_maximum_primary_id{
      std::numeric_limits<std::uint64_t>::max()};

  std::uint64_t begin = next_global_primary_id_;
  std::uint64_t count_minus_one = primary_count - 1ULL;

  GGEMS_CHECK_RECOVERABLE(count_minus_one <= k_maximum_primary_id - begin,
                          "Primary stream requested range is not representable "
                          "with uint64_t global primary identifiers.");

  std::uint64_t last_id = begin + count_minus_one;

  GGEMSPrimaryStreamRunView run_view{.run_id = run_id,
                                     .source_primary_count = primary_count,
                                     .global_history_offset = begin};

  if (last_id == k_maximum_primary_id) {
    exhausted_ = true;
  } else {
    next_global_primary_id_ = last_id + 1ULL;
  }

  return run_view;
}

} // namespace ggems::core::particles
