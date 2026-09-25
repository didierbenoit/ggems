#pragma once

#include <cstdint>
#include <numbers>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

inline constexpr float k_isotropic_full_sphere_cos_theta_lower{-1.0F};
inline constexpr float k_isotropic_full_sphere_cos_theta_upper{1.0F};
inline constexpr float k_isotropic_full_sphere_phi_min_rad{0.0F};
inline constexpr float k_isotropic_full_sphere_phi_max_rad{
  6.28318530717958647692F};

struct GGEMSSourceRecord {
  std::uint64_t source_id{0ULL};

  std::uint64_t time_start_ps{0ULL};
  std::uint64_t time_stop_ps{0ULL};

  std::uint64_t energy_micro_eV{511'000'000'000ULL};

  std::int64_t position_x_pm{0LL};
  std::int64_t position_y_pm{0LL};
  std::int64_t position_z_pm{0LL};

  std::uint32_t source_type{ToKernelSourceType(GGEMSSourceType::Analytic)};

  std::uint32_t emitted_particle_type{
    particles::ToKernelParticleType(particles::GGEMSParticleType::Gamma)};

  std::uint32_t flags{0U};
  std::uint32_t reserved_0{0U};

  float axis_x_x{1.0F};
  float axis_x_y{0.0F};
  float axis_x_z{0.0F};

  float axis_y_x{0.0F};
  float axis_y_y{1.0F};
  float axis_y_z{0.0F};

  float axis_z_x{0.0F};
  float axis_z_y{0.0F};
  float axis_z_z{1.0F};

  std::uint32_t emission_geometry_type{
    ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Point)};
  std::uint32_t angular_distribution_type{
    ToKernelAngularDistributionType(GGEMSAngularDistributionType::Fixed)};

  std::uint64_t geometry_size_x_pm{0ULL};
  std::uint64_t geometry_size_y_pm{0ULL};

  std::int64_t focus_position_x_pm{0LL};
  std::int64_t focus_position_y_pm{0LL};
  std::int64_t focus_position_z_pm{0LL};

  std::uint64_t geometry_size_z_pm{0ULL};

  float isotropic_cos_theta_lower{-1.0F};
  float isotropic_cos_theta_upper{1.0F};
  float isotropic_phi_min_rad{0.0F};
  float isotropic_phi_max_rad{2.0F * std::numbers::pi_v<float>};
};

} // namespace ggems::core::sources
