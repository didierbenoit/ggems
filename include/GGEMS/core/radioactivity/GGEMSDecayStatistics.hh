#pragma once

#include <cstdint>

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"

namespace ggems::core::random {
class GGEMSHostRandomStream;
}

namespace ggems::core::radioactivity {

[[nodiscard]] auto
ComputeExpectedDecayEventCount(units::Activity activity_at_reference,
                               long double half_life_seconds,
                               std::uint64_t reference_time_ps,
                               GGEMSTimeWindow time_window) -> long double;

[[nodiscard]] auto SampleDecayEventCount(
    units::Activity activity_at_reference, long double half_life_seconds,
    std::uint64_t reference_time_ps, GGEMSTimeWindow time_window,
    random::GGEMSHostRandomStream &random_stream) -> std::uint64_t;

} // namespace ggems::core::radioactivity
