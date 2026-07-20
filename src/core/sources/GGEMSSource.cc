#include <cmath>
#include <cstdint>

#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"

namespace ggems::core::sources {

// =============================================================================
// =============================================================================

GGEMSSource::GGEMSSource() {
  record_.source_id = 0ULL;

  record_.source_type = ToKernelSourceType(GGEMSSourceType::Analytic);

  record_.emitted_particle_type =
      particles::ToKernelParticleType(particles::GGEMSParticleType::Gamma);

  record_.time_start_ps = 0ULL;
  record_.time_stop_ps = 1'000'000ULL;

  record_.energy_milli_eV = 511'000'000ULL;

  record_.position_x_pm = 0LL;
  record_.position_y_pm = 0LL;
  record_.position_z_pm = 0LL;

  record_.direction_x = 0.0F;
  record_.direction_y = 0.0F;
  record_.direction_z = 1.0F;
  record_.direction_w = 0.0F;

  record_.weight = 1.0F;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetPrimaryCount(std::uint64_t primary_count) noexcept
    -> GGEMSSource & {
  primary_count_ = primary_count;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetAnalytic() noexcept -> GGEMSSource & {
  record_.source_type = ToKernelSourceType(GGEMSSourceType::Analytic);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetEmittedParticleType(
    particles::GGEMSParticleType particle_type) noexcept -> GGEMSSource & {
  record_.emitted_particle_type =
      particles::ToKernelParticleType(particle_type);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetEnergyMilliElectronVolt(std::uint64_t energy_milli_eV)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(energy_milli_eV > 0ULL,
                          "Source energy must be non-zero.");

  record_.energy_milli_eV = energy_milli_eV;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetTimeWindowPicoSecond(std::uint64_t time_start_ps,
                                          std::uint64_t time_stop_ps)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(time_stop_ps >= time_start_ps,
                          "Source time stop must be greater than or equal to "
                          "source time start.");

  record_.time_start_ps = time_start_ps;
  record_.time_stop_ps = time_stop_ps;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetPositionPicoMeter(std::int64_t x_pm, std::int64_t y_pm,
                                       std::int64_t z_pm) noexcept
    -> GGEMSSource & {
  record_.position_x_pm = x_pm;
  record_.position_y_pm = y_pm;
  record_.position_z_pm = z_pm;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetDirection(float dir_x, float dir_y, float dir_z)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(std::isfinite(dir_x) && std::isfinite(dir_y) &&
                              std::isfinite(dir_z),
                          "Source direction must contain finite values.");

  float const norm2 = (dir_x * dir_x) + (dir_y * dir_y) + (dir_z * dir_z);

  GGEMS_CHECK_RECOVERABLE(norm2 > 0.0F,
                          "Source direction cannot be the zero vector.");

  float const inv_norm = 1.0F / std::sqrt(norm2);

  record_.direction_x = dir_x * inv_norm;
  record_.direction_y = dir_y * inv_norm;
  record_.direction_z = dir_z * inv_norm;
  record_.direction_w = 0.0F;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetWeight(float weight) -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(std::isfinite(weight),
                          "Source weight must be finite.");

  GGEMS_CHECK_RECOVERABLE(weight >= 0.0F,
                          "Source weight must be positive or zero.");

  record_.weight = weight;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::Verbose() const -> void {
  GGEMS_INFO("Source", "{}", DescribeSource(record_, primary_count_));
}
} // namespace ggems::core::sources
