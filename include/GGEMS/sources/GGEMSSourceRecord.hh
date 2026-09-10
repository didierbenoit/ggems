#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

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

  // Exact Mono energy; zero for table-backed energy distributions.
  std::uint64_t energy_micro_eV{511'000'000'000ULL};

  std::int64_t position_x_pm{0ULL};
  std::int64_t position_y_pm{0ULL};
  std::int64_t position_z_pm{0ULL};

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

  float weight{1.0F};

  std::uint32_t emission_geometry_type{
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Point)};
  std::uint32_t angular_distribution_type{
      ToKernelAngularDistributionType(GGEMSAngularDistributionType::Fixed)};

  std::uint64_t geometry_size_x_pm{0ULL};
  std::uint64_t geometry_size_y_pm{0ULL};

  std::int64_t focus_position_x_pm{0ULL};
  std::int64_t focus_position_y_pm{0ULL};
  std::int64_t focus_position_z_pm{0ULL};

  std::uint64_t geometry_size_z_pm{0ULL};

  float isotropic_cos_theta_lower{k_isotropic_full_sphere_cos_theta_lower};
  float isotropic_cos_theta_upper{k_isotropic_full_sphere_cos_theta_upper};
  float isotropic_phi_min_rad{k_isotropic_full_sphere_phi_min_rad};
  float isotropic_phi_max_rad{k_isotropic_full_sphere_phi_max_rad};
};

static_assert(std::is_standard_layout_v<GGEMSSourceRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSSourceRecord>);
static_assert(sizeof(GGEMSSourceRecord) == 184U);
static_assert(alignof(GGEMSSourceRecord) == 8U);
static_assert(offsetof(GGEMSSourceRecord, source_id) == 0U);
static_assert(offsetof(GGEMSSourceRecord, time_start_ps) == 8U);
static_assert(offsetof(GGEMSSourceRecord, time_stop_ps) == 16U);
static_assert(offsetof(GGEMSSourceRecord, energy_micro_eV) == 24U);
static_assert(offsetof(GGEMSSourceRecord, position_x_pm) == 32U);
static_assert(offsetof(GGEMSSourceRecord, position_y_pm) == 40U);
static_assert(offsetof(GGEMSSourceRecord, position_z_pm) == 48U);
static_assert(offsetof(GGEMSSourceRecord, source_type) == 56U);
static_assert(offsetof(GGEMSSourceRecord, emitted_particle_type) == 60U);
static_assert(offsetof(GGEMSSourceRecord, flags) == 64U);
static_assert(offsetof(GGEMSSourceRecord, reserved_0) == 68U);
static_assert(offsetof(GGEMSSourceRecord, axis_x_x) == 72U);
static_assert(offsetof(GGEMSSourceRecord, axis_x_y) == 76U);
static_assert(offsetof(GGEMSSourceRecord, axis_x_z) == 80U);
static_assert(offsetof(GGEMSSourceRecord, axis_y_x) == 84U);
static_assert(offsetof(GGEMSSourceRecord, axis_y_y) == 88U);
static_assert(offsetof(GGEMSSourceRecord, axis_y_z) == 92U);
static_assert(offsetof(GGEMSSourceRecord, axis_z_x) == 96U);
static_assert(offsetof(GGEMSSourceRecord, axis_z_y) == 100U);
static_assert(offsetof(GGEMSSourceRecord, axis_z_z) == 104U);
static_assert(offsetof(GGEMSSourceRecord, weight) == 108U);
static_assert(offsetof(GGEMSSourceRecord, emission_geometry_type) == 112U);
static_assert(offsetof(GGEMSSourceRecord, angular_distribution_type) == 116U);
static_assert(offsetof(GGEMSSourceRecord, geometry_size_x_pm) == 120U);
static_assert(offsetof(GGEMSSourceRecord, geometry_size_y_pm) == 128U);
static_assert(offsetof(GGEMSSourceRecord, focus_position_x_pm) == 136U);
static_assert(offsetof(GGEMSSourceRecord, focus_position_y_pm) == 144U);
static_assert(offsetof(GGEMSSourceRecord, focus_position_z_pm) == 152U);
static_assert(offsetof(GGEMSSourceRecord, geometry_size_z_pm) == 160U);
static_assert(offsetof(GGEMSSourceRecord, isotropic_cos_theta_lower) == 168U);
static_assert(offsetof(GGEMSSourceRecord, isotropic_cos_theta_upper) == 172U);
static_assert(offsetof(GGEMSSourceRecord, isotropic_phi_min_rad) == 176U);
static_assert(offsetof(GGEMSSourceRecord, isotropic_phi_max_rad) == 180U);
} // namespace ggems::core::sources
