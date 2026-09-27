#include <cmath>
#include <cstdint>
#include <numbers>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/radioactivity/GGEMSDecayStatistics.hh"
#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSPoissonSampler.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"
#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::core::radioactivity {

// =============================================================================
// =============================================================================

auto ComputeExpectedDecayEventCount(units::Activity activity_at_reference,
                                    long double half_life_seconds,
                                    std::uint64_t reference_time_ps,
                                    GGEMSTimeWindow time_window)
  -> long double {
  if (!(activity_at_reference.value >= 0.0L)) {
    throw ggems::core::GGEMSRecoverable(
      "Activity at the reference time must be non-negative.");
  }

  if (reference_time_ps > time_window.start_ps) {
    throw ggems::core::GGEMSRecoverable(
      "Decay reference time must not exceed the window start.");
  }

  if (activity_at_reference.value == 0.0L ||
      time_window.start_ps == time_window.stop_ps) {
    return 0.0L;
  }

  auto const elapsed_start_ps = time_window.start_ps - reference_time_ps;
  auto const duration_ps = time_window.stop_ps - time_window.start_ps;

  auto const elapsed_start_seconds =
    *units::ConvertTo(units::Duration{elapsed_start_ps}, "s");

  auto const duration_seconds =
    *units::ConvertTo(units::Duration{duration_ps}, "s");

  long double const decay_constant =
    std::numbers::ln2_v<long double> / half_life_seconds;

  long double const activity_at_start =
    activity_at_reference.value *
    std::exp(-decay_constant * elapsed_start_seconds);

  long double const decayed_fraction =
    -std::expm1(-decay_constant * duration_seconds);

  return activity_at_start * decayed_fraction / decay_constant;
}

// =============================================================================
// =============================================================================

auto SampleDecayEventCount(units::Activity activity_at_reference,
                           long double half_life_seconds,
                           std::uint64_t reference_time_ps,
                           GGEMSTimeWindow time_window,
                           random::GGEMSHostRandomStream &random_stream)
  -> std::uint64_t {
  return random::SamplePoisson(
    ComputeExpectedDecayEventCount(activity_at_reference, half_life_seconds,
                                   reference_time_ps, time_window),
    random_stream);
}
} // namespace ggems::core::radioactivity
