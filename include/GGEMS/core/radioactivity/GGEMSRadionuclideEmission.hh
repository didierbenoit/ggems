#pragma once

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity {

class GGEMSRadionuclideEmission {
public:
  GGEMSRadionuclideEmission(
      particles::GGEMSParticleType particle_type, long double yield_per_decay,
      sources::GGEMSEnergyDistribution energy_distribution);

  [[nodiscard]] auto GetParticleType() const noexcept
      -> particles::GGEMSParticleType {
    return particle_type_;
  }

  [[nodiscard]] auto GetYieldPerDecay() const noexcept -> long double {
    return yield_per_decay_;
  }

  [[nodiscard]] auto GetEnergyDistribution() const noexcept
      -> sources::GGEMSEnergyDistribution const & {
    return energy_distribution_;
  }

private:
  particles::GGEMSParticleType particle_type_;
  long double yield_per_decay_;
  sources::GGEMSEnergyDistribution energy_distribution_;
};

} // namespace ggems::core::radioactivity
