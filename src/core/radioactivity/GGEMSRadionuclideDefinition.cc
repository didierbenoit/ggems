#include <cmath>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/detail/GGEMSRadionuclideLookupPolicy.hh"

namespace ggems::core::radioactivity {
namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ComputeTotalYieldPerDecay(std::span<GGEMSRadionuclideEmission const> emissions)
    -> long double {
  long double sum{0.0L};
  long double compensation{0.0L};

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    long double const value = emission.GetYieldPerDecay();
    long double const next = sum + value;

    if (!(std::isfinite(next))) {
      throw ggems::core::GGEMSRecoverable("Radionuclide total emission yield is not finite.");
    }

    if (std::abs(sum) >= std::abs(value)) {
      compensation += (sum - next) + value;
    } else {
      compensation += (value - next) + sum;
    }

    if (!(std::isfinite(compensation))) {
      throw ggems::core::GGEMSRecoverable("Radionuclide total emission yield is not finite.");
    }

    sum = next;
  }

  long double const total = sum + compensation;

  if (!(std::isfinite(total))) {
    throw ggems::core::GGEMSRecoverable("Radionuclide total emission yield is not finite.");
  }
  if (!(total > 0.0L)) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide total emission yield must be strictly positive.");
  }

  return total;
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSRadionuclideDefinition::BuildLookupKeys(
    std::string_view canonical_name, std::span<std::string const> aliases)
    -> std::vector<std::string> {
  std::vector<std::string> keys;
  std::unordered_set<std::string> unique_keys;

  auto add_key = [&](std::string_view name, std::string_view kind) -> void {
    std::string normalized = detail::NormalizeRadionuclideLookupName(name);

    if (normalized.empty()) {
      throw ggems::core::GGEMSRecoverable(
          std::format("Radionuclide {} must contain non-whitespace text.", kind));
    }
    if (!(unique_keys.insert(normalized).second)) {
      throw ggems::core::GGEMSRecoverable(
          std::format("Duplicate radionuclide {} '{}' after ASCII lookup "
                    "normalization.",
                    kind, name));
    }

    keys.push_back(std::move(normalized));
  };

  add_key(canonical_name, "canonical name");

  for (std::string const &alias : aliases) {
    add_key(alias, "alias");
  }

  return keys;
}

// -----------------------------------------------------------------------------

GGEMSRadionuclideDefinition::GGEMSRadionuclideDefinition(
    std::string canonical_name, std::vector<std::string> aliases,
    long double half_life_seconds,
    std::vector<GGEMSRadionuclideEmission> emissions)
    : canonical_name_{std::move(canonical_name)}, aliases_{std::move(aliases)},
      half_life_seconds_{half_life_seconds}, emissions_{std::move(emissions)} {
  if (!(std::isfinite(half_life_seconds_))) {
    throw ggems::core::GGEMSRecoverable("Radionuclide half-life must be finite.");
  }
  if (!(half_life_seconds_ > 0.0L)) {
    throw ggems::core::GGEMSRecoverable("Radionuclide half-life must be strictly positive.");
  }
  if (emissions_.empty()) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide definition requires at least one emission channel.");
  }

  lookup_keys_ = BuildLookupKeys(canonical_name_, aliases_);
  total_yield_per_decay_ = ComputeTotalYieldPerDecay(emissions_);
  channel_selection_weights_.reserve(emissions_.size());

  for (GGEMSRadionuclideEmission const &emission : emissions_) {
    long double const selection_weight =
        emission.GetYieldPerDecay() / total_yield_per_decay_;
    if (!(std::isfinite(selection_weight))) {
      throw ggems::core::GGEMSInternal("Radionuclide channel selection weight is not finite.");
    }
    channel_selection_weights_.push_back(selection_weight);
  }
}
} // namespace ggems::core::radioactivity
