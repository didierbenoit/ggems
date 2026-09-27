#pragma once

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::render {

constexpr auto
GetParticleColorKey(core::particles::GGEMSParticleType particle_type) noexcept
  -> ColorKey {
  using core::particles::GGEMSParticleType;

  switch (particle_type) {
  case GGEMSParticleType::Aionino:
    return MAGENTA_Electric;
  case GGEMSParticleType::Gamma:
    return GREEN_SeaGreen;
  case GGEMSParticleType::Electron:
    return CYAN_Cryo;
  case GGEMSParticleType::Positron:
    return MAGENTA_LightMagenta;
  case GGEMSParticleType::Proton:
    return RED_Tomato;
  case GGEMSParticleType::Neutron:
    return GRAY_Silver;
  case GGEMSParticleType::Alpha:
    return YELLOW_Gold;
  case GGEMSParticleType::Unknown:
  default:
    return GRAY_Neutral200_F;
  }
}

constexpr auto
GetParticleRGB(core::particles::GGEMSParticleType particle_type) noexcept
  -> RGB {
  ColorKey const color = GetParticleColorKey(particle_type);
  return GetColorRGB(color.family, color.shade, color.variant);
}

} // namespace ggems::render
