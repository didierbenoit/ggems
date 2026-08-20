#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"

namespace ggems::core::radioactivity {

class GGEMSRadionuclideDefinition {
public:
  GGEMSRadionuclideDefinition(std::string canonical_name,
                              long double half_life_seconds,
                              std::vector<GGEMSRadionuclideEmission> emissions);

  [[nodiscard]] auto GetCanonicalName() const noexcept -> std::string_view {
    return canonical_name_;
  }

  [[nodiscard]] auto GetHalfLifeSeconds() const noexcept -> long double {
    return half_life_seconds_;
  }

  [[nodiscard]] auto GetEmissions() const noexcept
      -> std::span<GGEMSRadionuclideEmission const> {
    return emissions_;
  }

  [[nodiscard]] auto GetTotalYieldPerDecay() const -> long double;

private:
  std::string canonical_name_;
  long double half_life_seconds_;
  std::vector<GGEMSRadionuclideEmission> emissions_;
};
} // namespace ggems::core::radioactivity
