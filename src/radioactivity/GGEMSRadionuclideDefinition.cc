#include <string>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"

namespace ggems::core::radioactivity {

// =============================================================================
// =============================================================================

GGEMSRadionuclideDefinition::GGEMSRadionuclideDefinition(
  std::string canonical_name, long double half_life_seconds,
  std::vector<GGEMSRadionuclideEmission> emissions)
    : canonical_name_{std::move(canonical_name)},
      half_life_seconds_{half_life_seconds}, emissions_{std::move(emissions)} {
  if (canonical_name_.empty()) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide canonical name must be empty.");
  }

  if (!(half_life_seconds_ > 0.0L)) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide half-life must be strictly positive.");
  }

  if (emissions_.empty()) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide definition requires at least one emission channel.");
  }
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSRadionuclideDefinition::GetTotalYieldPerDecay() const noexcept
  -> long double {
  long double total{0.0L};

  for (auto const &emission : emissions_) {
    total += emission.GetYieldPerDecay();
  }

  return total;
}

} // namespace ggems::core::radioactivity
