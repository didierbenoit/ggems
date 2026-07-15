#include "GGEMS/core/particles/GGEMSPrimaryStream.hh"

#include <limits>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::core::particles {

// =============================================================================
// =============================================================================

void GGEMSPrimaryStream::SetPrimaryCount(std::uint64_t primary_count) {
  GGEMS_CHECK_RECOVERABLE(primary_count > 0ULL,
                          "Primary stream particle count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(
      !initialised_,
      "Primary stream particle count cannot be changed after initialise.");

  primary_count_ = primary_count;
}

// -----------------------------------------------------------------------------

void GGEMSPrimaryStream::Initialise() {
  GGEMS_CHECK_RECOVERABLE(
      !initialised_, "Primary stream cannot be initialised more than once.");

  GGEMS_CHECK_RECOVERABLE(primary_count_ > 0ULL,
                          "Cannot initialise an empty primary stream.");

  next_global_primary_id_ = 0ULL;
  exhausted_ = false;
  initialised_ = true;

  GGEMS_INFO("Core", "Primary Aionino stream initialised");
}

// -----------------------------------------------------------------------------

GGEMSPrimaryStreamRunView GGEMSPrimaryStream::PrepareRun(std::uint64_t run_id) {
  return PrepareRun(run_id, primary_count_);
}

// -----------------------------------------------------------------------------

GGEMSPrimaryStreamRunView
GGEMSPrimaryStream::PrepareRun(std::uint64_t run_id,
                               std::uint64_t primary_count) {
  GGEMS_CHECK_RECOVERABLE(
      initialised_,
      "Primary stream must be initialised before reserving a range");

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
