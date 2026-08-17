#include <cmath>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"

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
      throw ggems::core::GGEMSRecoverable(
          "Radionuclide total emission yield is not finite.");
    }

    if (std::abs(sum) >= std::abs(value)) {
      compensation += (sum - next) + value;
    } else {
      compensation += (value - next) + sum;
    }

    if (!(std::isfinite(compensation))) {
      throw ggems::core::GGEMSRecoverable(
          "Radionuclide total emission yield is not finite.");
    }

    sum = next;
  }

  long double const total = sum + compensation;

  if (!(std::isfinite(total))) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide total emission yield is not finite.");
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

GGEMSRadionuclideDefinition::GGEMSRadionuclideDefinition(
    std::string canonical_name, long double half_life_seconds,
    std::vector<GGEMSRadionuclideEmission> emissions)
    : canonical_name_{std::move(canonical_name)},
      half_life_seconds_{half_life_seconds}, emissions_{std::move(emissions)} {
  if (canonical_name_.empty() ||
      canonical_name_.find_first_not_of(" \t\r\n\f\v") == std::string::npos) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide canonical name must contain non-whitespace text.");
  }
  if (!(std::isfinite(half_life_seconds_))) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide half-life must be finite.");
  }
  if (!(half_life_seconds_ > 0.0L)) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide half-life must be strictly positive.");
  }
  if (emissions_.empty()) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide definition requires at least one emission channel.");
  }

  total_yield_per_decay_ = ComputeTotalYieldPerDecay(emissions_);
  channel_selection_weights_.reserve(emissions_.size());

  for (GGEMSRadionuclideEmission const &emission : emissions_) {
    long double const selection_weight =
        emission.GetYieldPerDecay() / total_yield_per_decay_;
    if (!(std::isfinite(selection_weight))) {
      throw ggems::core::GGEMSInternal(
          "Radionuclide channel selection weight is not finite.");
    }
    channel_selection_weights_.push_back(selection_weight);
  }
}
} // namespace ggems::core::radioactivity
