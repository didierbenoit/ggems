#include "GGEMS/core/particles/GGEMSPrimaryStream.hh"

#include <limits>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::core::particles {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSPrimaryStream::SetPrimaryCount(std::uint64_t primary_count) {
  GGEMS_CHECK_RECOVERABLE(primary_count > 0ULL,
                          "Primary stream particle count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(
      !initialised_,
      "Primary stream particle count cannot be changed after initialise.");

  primary_count_ = primary_count;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSPrimaryStream::Initialise() {
  GGEMS_CHECK_RECOVERABLE(primary_count_ > 0ULL,
                          "Cannot Initialise an empty primary stream.");

  initialised_ = true;

  GGEMS_INFO("Core", "Primary Aionino stream initialised with {} primaries.",
             primary_count_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSPrimaryStreamRunView
GGEMSPrimaryStream::PrepareRun(std::uint64_t run_id) const {
  GGEMS_CHECK_RECOVERABLE(initialised_,
                          "Primary stream must be initialised before Run.");

  GGEMS_CHECK_RECOVERABLE(run_id <= std::numeric_limits<std::uint64_t>::max() /
                                        primary_count_,
                          "Primary stream global history offset overflow.");

  return GGEMSPrimaryStreamRunView{.run_id = run_id,
                                   .source_primary_count = primary_count_,
                                   .global_history_offset =
                                       run_id * primary_count_};
}

} // namespace ggems::core::particles
