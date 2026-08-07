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

auto NormalizeParticleName(std::string_view particle_name) -> std::string {
  std::string normalized;
  normalized.reserve(particle_name.size());

  for (char character : particle_name) {
    if (character == ' ' || character == '_') {
      continue;
    }

    normalized.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
  }

  return normalized;
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
  std::string normalized = NormalizeParticleName(particle_name);

  if (normalized == "aionino" || normalized == "l") {
    return GGEMSParticleType::Aionino;
  }

  if (normalized == "gamma" || normalized == "photon" || normalized == "g") {
    return GGEMSParticleType::Gamma;
  }

  if (normalized == "electron" || normalized == "e-" || normalized == "b-") {
    return GGEMSParticleType::Electron;
  }

  if (normalized == "positron" || normalized == "e+" || normalized == "b+") {
    return GGEMSParticleType::Positron;
  }

  if (normalized == "proton" || normalized == "p") {
    return GGEMSParticleType::Proton;
  }

  if (normalized == "neutron" || normalized == "n") {
    return GGEMSParticleType::Neutron;
  }

  if (normalized == "alpha" || normalized == "a") {
    return GGEMSParticleType::Alpha;
  }

  GGEMS_RECOVERABLE(
      std::format("Unsupported GGEMS particle type '{}'.", particle_name));

  return GGEMSParticleType::Unknown;
}

} // namespace ggems::core::particles
