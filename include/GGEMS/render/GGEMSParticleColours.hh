#pragma once

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::render {

constexpr ColourKey GetParticleColourKey(
    core::particles::GGEMSParticleType particle_type) noexcept {
  using core::particles::GGEMSParticleType;

  switch (particle_type) {
  case GGEMSParticleType::Aionino:
    return MAGENTA_Electric;
  case GGEMSParticleType::Gamma:
    return GREEN_Neon;
  case GGEMSParticleType::Electron:
    return CYAN_Cryo;
  case GGEMSParticleType::Positron:
    return MAGENTA_Fuchsia;
  case GGEMSParticleType::Proton:
    return RED_Tomato;
  case GGEMSParticleType::Neutron:
    return GREY_Silver;
  case GGEMSParticleType::Alpha:
    return YELLOW_Gold;
  case GGEMSParticleType::Unknown:
  default:
    return WHITE_Pale_F;
  }
}

constexpr RGB
GetParticleRGB(core::particles::GGEMSParticleType particle_type) noexcept {
  ColourKey colour = GetParticleColourKey(particle_type);
  return GetColourRGB(colour.family, colour.shade, colour.variant);
}

} // namespace ggems::render
