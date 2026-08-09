#include "GGEMS/core/particles/GGEMSPrimaryStream.hh"

#include <limits>
#include <cstdint>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"

namespace ggems::core::particles {

// =============================================================================
// =============================================================================

auto GGEMSPrimaryStream::SetPrimaryCount(std::uint64_t primary_count) -> void {
  if (!(primary_count > 0ULL)) {
    throw ggems::core::GGEMSRecoverable("Primary stream particle count must be non-zero.");
  }

  if (initialized_) {
    throw ggems::core::GGEMSRecoverable(
        "Primary stream particle count cannot be changed after initialize.");
  }

  primary_count_ = primary_count;
}

// -----------------------------------------------------------------------------

auto GGEMSPrimaryStream::Initialize() -> void {
  if (initialized_) {
    throw ggems::core::GGEMSRecoverable("Primary stream cannot be initialized more than once.");
  }

  if (!(primary_count_ > 0ULL)) {
    throw ggems::core::GGEMSRecoverable("Cannot initialize an empty primary stream.");
  }

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
  if (!(initialized_)) {
    throw ggems::core::GGEMSRecoverable(
        "Primary stream must be initialized before reserving a range");
  }

  if (!(primary_count > 0ULL)) {
    throw ggems::core::GGEMSRecoverable("Primary stream reservation count must be non-zero");
  }

  if (exhausted_) {
    throw ggems::core::GGEMSRecoverable(
        "Primary stream is exhausted; no global primary identifiers remain.");
  }

  constexpr std::uint64_t k_maximum_primary_id{
      std::numeric_limits<std::uint64_t>::max()};

  std::uint64_t begin = next_global_primary_id_;
  std::uint64_t count_minus_one = primary_count - 1ULL;

  if (!(count_minus_one <= k_maximum_primary_id - begin)) {
    throw ggems::core::GGEMSRecoverable("Primary stream requested range is not representable "
                          "with uint64_t global primary identifiers.");
  }

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
