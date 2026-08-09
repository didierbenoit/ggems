#include <cmath>
#include <utility>

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity {
namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsPhysicalParticleType(particles::GGEMSParticleType particle_type) noexcept
    -> bool {
  switch (particle_type) {
  case particles::GGEMSParticleType::Gamma:
  case particles::GGEMSParticleType::Electron:
  case particles::GGEMSParticleType::Positron:
  case particles::GGEMSParticleType::Proton:
  case particles::GGEMSParticleType::Neutron:
  case particles::GGEMSParticleType::Alpha:
    return true;
  case particles::GGEMSParticleType::Unknown:
  case particles::GGEMSParticleType::Aionino:
    return false;
  }

  return false;
}
} // namespace

// =============================================================================
// =============================================================================

GGEMSRadionuclideEmission::GGEMSRadionuclideEmission(
    particles::GGEMSParticleType particle_type, long double yield_per_decay,
    sources::GGEMSEnergyDistribution energy_distribution)
    : particle_type_{particle_type}, yield_per_decay_{yield_per_decay},
      energy_distribution_{std::move(energy_distribution)} {
  if (!(IsPhysicalParticleType(particle_type_))) {
    throw ggems::core::GGEMSRecoverable(
        "Radionuclide emission particle type must be a physical particle.");
  }
  if (!(std::isfinite(yield_per_decay_))) {
    throw ggems::core::GGEMSRecoverable("Radionuclide emission yield must be finite.");
  }
  if (!(yield_per_decay_ > 0.0L)) {
    throw ggems::core::GGEMSRecoverable("Radionuclide emission yield must be strictly positive.");
  }
}

} // namespace ggems::core::radioactivity
