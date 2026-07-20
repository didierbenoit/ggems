#include <cctype>
#include <format>
#include <string>
#include <string_view>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::core::particles {
namespace {

// =============================================================================
// =============================================================================

auto NormaliseParticleName(std::string_view particle_name) -> std::string {
  std::string normalised;
  normalised.reserve(particle_name.size());

  for (char character : particle_name) {
    if (character == ' ' || character == '_') {
      continue;
    }

    normalised.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
  }

  return normalised;
}

} // namespace

// =============================================================================
// =============================================================================

auto ToLongName(GGEMSParticleType particle_type) -> std::string {
  switch (particle_type) {
  case GGEMSParticleType::Unknown:
    return "Unknown";
  case GGEMSParticleType::Aionino:
    return "Aionino";
  case GGEMSParticleType::Gamma:
    return "Gamma";
  case GGEMSParticleType::Electron:
    return "Electron";
  case GGEMSParticleType::Positron:
    return "Positron";
  case GGEMSParticleType::Proton:
    return "Proton";
  case GGEMSParticleType::Neutron:
    return "Neutron";
  case GGEMSParticleType::Alpha:
    return "Alpha";
  }

  GGEMS_INTERNAL("Unsupported GGEMS particle type.");
  return "Unknown";
}

// =============================================================================
// =============================================================================

auto ToShortName(GGEMSParticleType particle_type) -> std::string {
  switch (particle_type) {
  case GGEMSParticleType::Unknown:
    return "?";
  case GGEMSParticleType::Aionino:
    return "l";
  case GGEMSParticleType::Gamma:
    return "g";
  case GGEMSParticleType::Electron:
    return "b-";
  case GGEMSParticleType::Positron:
    return "b+";
  case GGEMSParticleType::Proton:
    return "p";
  case GGEMSParticleType::Neutron:
    return "n";
  case GGEMSParticleType::Alpha:
    return "a";
  }

  GGEMS_INTERNAL("Unsupported GGEMS particle type.");
  return "?";
}

// =============================================================================
// =============================================================================

auto ParseParticleType(std::string_view particle_name) -> GGEMSParticleType {
  std::string normalised = NormaliseParticleName(particle_name);

  if (normalised == "aionino" || normalised == "l") {
    return GGEMSParticleType::Aionino;
  }

  if (normalised == "gamma" || normalised == "photon" || normalised == "g") {
    return GGEMSParticleType::Gamma;
  }

  if (normalised == "electron" || normalised == "e-" || normalised == "b-") {
    return GGEMSParticleType::Electron;
  }

  if (normalised == "positron" || normalised == "e+" || normalised == "b+") {
    return GGEMSParticleType::Positron;
  }

  if (normalised == "proton" || normalised == "p") {
    return GGEMSParticleType::Proton;
  }

  if (normalised == "neutron" || normalised == "n") {
    return GGEMSParticleType::Neutron;
  }

  if (normalised == "alpha" || normalised == "a") {
    return GGEMSParticleType::Alpha;
  }

  GGEMS_RECOVERABLE(
      std::format("Unsupported GGEMS particle type '{}'.", particle_name));

  return GGEMSParticleType::Unknown;
}

} // namespace ggems::core::particles
