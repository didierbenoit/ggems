#pragma once

#include <cstdint>

#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace ggems::core::sources {

struct GGEMSEmissionBounds {
  std::uint64_t component_radius_pm{0ULL};
  long double half_extent_x_pm{0.0L};
  long double half_extent_y_pm{0.0L};
  long double half_extent_z_pm{0.0L};
};

[[nodiscard]] auto HasSignedPicoMeterEnvelope(std::int64_t center_pm,
                                              std::uint64_t radius_pm) noexcept
    -> bool;

[[nodiscard]] auto BuildEmissionBounds(GGEMSSourceRecord const &record)
    -> GGEMSEmissionBounds;

[[nodiscard]] auto
IsDefaultFullSphereIsotropicDomain(GGEMSSourceRecord const &record) noexcept
    -> bool;

auto ValidateAnalyticSourceRecord(GGEMSSourceRecord const &record) -> void;

} // namespace ggems::core::sources
