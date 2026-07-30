#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include "GGEMS/core/units/GGEMSActivityUnits.hh"

namespace ggems::core::radioactivity {
class GGEMSRadionuclideDefinition;
}

namespace ggems::core::sources {

enum class GGEMSSourcePopulationMode : std::uint8_t {
  CountDriven = 0U,
  ActivityDriven
};

struct GGEMSCountDrivenSourceConfiguration {
  static constexpr std::uint64_t k_default_primary_count{4096ULL};
  std::uint64_t primary_count{k_default_primary_count};
};

struct GGEMSActivityDrivenSourceConfiguration {
  std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>
      radionuclide;
  units::Activity activity_at_reference_time{};
  std::uint64_t reference_time_ps{0ULL};
};

using GGEMSSourcePopulationConfiguration =
    std::variant<GGEMSCountDrivenSourceConfiguration,
                 GGEMSActivityDrivenSourceConfiguration>;

} // namespace ggems::core::sources
