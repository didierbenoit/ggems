#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ggems::core::particles {
constexpr std::uint32_t k_invalid_id_u32{0xFFFFFFFFU};
constexpr std::uint64_t k_invalid_id_u64{0xFFFFFFFFFFFFFFFFULL};

enum class GGEMSParticleType : std::uint32_t {
  Unknown = 0U,
  Aionino = 1U,
  Gamma = 2U,
  Electron = 3U,
  Positron = 4U,
  Proton = 5U,
  Neutron = 6U,
  Alpha = 7U
};

enum class GGEMSParticleStatus : std::uint32_t {
  Inactive = 0U,
  Alive = 1U,
  Killed = 2U,
  EscapedWorld = 3U,
  Absorbed = 4U
};

constexpr std::uint32_t
ToKernelParticleType(GGEMSParticleType particle_type) noexcept {
  return static_cast<std::uint32_t>(particle_type);
}

constexpr GGEMSParticleType
FromKernelParticleType(std::uint32_t particle_type) noexcept {
  switch (particle_type) {
  case 1U:
    return GGEMSParticleType::Aionino;
  case 2U:
    return GGEMSParticleType::Gamma;
  case 3U:
    return GGEMSParticleType::Electron;
  case 4U:
    return GGEMSParticleType::Positron;
  case 5U:
    return GGEMSParticleType::Proton;
  case 6U:
    return GGEMSParticleType::Neutron;
  case 7U:
    return GGEMSParticleType::Alpha;
  default:
    return GGEMSParticleType::Unknown;
  }
}

constexpr std::uint32_t
ToKernelParticleStatus(GGEMSParticleStatus status) noexcept {
  return static_cast<std::uint32_t>(status);
}

std::string ToLongName(GGEMSParticleType particle_type);

std::string ToShortName(GGEMSParticleType particle_type);

GGEMSParticleType ParseParticleType(std::string_view particle_name);

} // namespace ggems::core::particles
