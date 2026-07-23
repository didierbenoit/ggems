#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

struct GGEMSSourceRecord {
  std::uint64_t source_id{0ULL};

  std::uint64_t time_start_ps{0ULL};
  std::uint64_t time_stop_ps{0ULL};

  std::uint64_t energy_milli_eV{511'000'000ULL};

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
};

static_assert(std::is_standard_layout_v<GGEMSSourceRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSSourceRecord>);
static_assert(sizeof(GGEMSSourceRecord) == 112U);
static_assert(alignof(GGEMSSourceRecord) == 8U);
static_assert(offsetof(GGEMSSourceRecord, source_id) == 0U);
static_assert(offsetof(GGEMSSourceRecord, time_start_ps) == 8U);
static_assert(offsetof(GGEMSSourceRecord, time_stop_ps) == 16U);
static_assert(offsetof(GGEMSSourceRecord, energy_milli_eV) == 24U);
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

} // namespace ggems::core::sources
