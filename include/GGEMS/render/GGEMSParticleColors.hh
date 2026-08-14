#pragma once

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColor.hh"

namespace ggems::render {

constexpr auto
GetParticleColorKey(core::particles::GGEMSParticleType particle_type) noexcept
    -> ColorKey {
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
    return GRAY_Silver;
  case GGEMSParticleType::Alpha:
    return YELLOW_Gold;
  case GGEMSParticleType::Unknown:
  default:
    return WHITE_Pale_F;
  }
}

constexpr auto
GetParticleRGB(core::particles::GGEMSParticleType particle_type) noexcept
    -> RGB {
  ColorKey color = GetParticleColorKey(particle_type);
  return GetColorRGB(color.family, color.shade, color.variant);
}

} // namespace ggems::render
