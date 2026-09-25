#include <cstdint>

#include "GGEMS/particles/GGEMSPrimaryStream.hh"

namespace ggems::core::particles {

// =============================================================================
// =============================================================================

auto GGEMSPrimaryStream::PrepareRun(std::uint64_t run_id,
                                    std::uint64_t primary_count)
  -> GGEMSPrimaryStreamRunView {
  GGEMSPrimaryStreamRunView const run_view{
    .run_id = run_id,
    .source_primary_count = primary_count,
    .global_history_offset = next_global_primary_id_,
  };

  next_global_primary_id_ += primary_count;

  return run_view;
}

} // namespace ggems::core::particles
