#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"

namespace ggems::core::radioactivity {

class GGEMSRadionuclideDefinition {
public:
  GGEMSRadionuclideDefinition(std::string canonical_name,
                              std::vector<std::string> aliases,
                              long double half_life_seconds,
                              std::vector<GGEMSRadionuclideEmission> emissions);

  [[nodiscard]] auto GetCanonicalName() const noexcept -> std::string_view {
    return canonical_name_;
  }

  [[nodiscard]] auto GetAliases() const noexcept
      -> std::span<std::string const> {
    return aliases_;
  }

  [[nodiscard]] auto GetHalfLifeSeconds() const noexcept -> long double {
    return half_life_seconds_;
  }

  [[nodiscard]] auto GetEmissions() const noexcept
      -> std::span<GGEMSRadionuclideEmission const> {
    return emissions_;
  }

  [[nodiscard]] auto GetTotalYieldPerDecay() const noexcept -> long double {
    return total_yield_per_decay_;
  }

  [[nodiscard]] auto GetChannelSelectionWeights() const noexcept
      -> std::span<long double const> {
    return channel_selection_weights_;
  }

  [[nodiscard]] auto BuildLookupKeys() const -> std::vector<std::string> {
    return lookup_keys_;
  }

private:
  [[nodiscard]] static auto
  BuildLookupKeys(std::string_view canonical_name,
                  std::span<std::string const> aliases)
      -> std::vector<std::string>;

  std::string canonical_name_;
  std::vector<std::string> aliases_;
  std::vector<std::string> lookup_keys_;
  long double half_life_seconds_;
  std::vector<GGEMSRadionuclideEmission> emissions_;
  long double total_yield_per_decay_{0.0L};
  std::vector<long double> channel_selection_weights_;
};
} // namespace ggems::core::radioactivity
