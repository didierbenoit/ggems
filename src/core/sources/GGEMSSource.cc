#include <cmath>
#include <format>
#include <cstdint>
#include <string>

#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace ggems::core::sources {

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

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

  record_.direction_x = 0.0f;
  record_.direction_y = 0.0f;
  record_.direction_z = 1.0f;
  record_.direction_w = 0.0f;

  record_.weight = 1.0f;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSSource &GGEMSSource::SetAnalytic() noexcept {
  record_.source_type = ToKernelSourceType(GGEMSSourceType::Analytic);
  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSSource &GGEMSSource::SetEmittedParticleType(
    particles::GGEMSParticleType particle_type) noexcept {
  record_.emitted_particle_type =
      particles::ToKernelParticleType(particle_type);

  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSSource &
GGEMSSource::SetEnergyMilliElectronVolt(std::uint64_t energy_milli_eV) {
  GGEMS_CHECK_RECOVERABLE(energy_milli_eV > 0ULL,
                          "Source energy must be non-zero.");

  record_.energy_milli_eV = energy_milli_eV;

  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSSource &GGEMSSource::SetTimeWindow(std::uint64_t time_start_ps,
                                        std::uint64_t time_stop_ps) {
  GGEMS_CHECK_RECOVERABLE(time_stop_ps >= time_start_ps,
                          "Source time stop must be greater than or equal to "
                          "source time start.");

  record_.time_start_ps = time_start_ps;
  record_.time_stop_ps = time_stop_ps;

  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSSource &GGEMSSource::SetPositionPM(std::int64_t x_pm, std::int64_t y_pm,
                                        std::int64_t z_pm) noexcept {
  record_.position_x_pm = x_pm;
  record_.position_y_pm = y_pm;
  record_.position_z_pm = z_pm;

  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSSource &GGEMSSource::SetDirection(float x, float y, float z) {
  GGEMS_CHECK_RECOVERABLE(std::isfinite(x) && std::isfinite(y) &&
                              std::isfinite(z),
                          "Source direction must contain finite values.");

  float const norm2 = x * x + y * y + z * z;

  GGEMS_CHECK_RECOVERABLE(norm2 > 0.0F,
                          "Source direction cannot be the zero vector.");

  float const inv_norm = 1.0F / std::sqrt(norm2);

  record_.direction_x = x * inv_norm;
  record_.direction_y = y * inv_norm;
  record_.direction_z = z * inv_norm;
  record_.direction_w = 0.0F;

  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSSource &GGEMSSource::SetWeight(float weight) {
  GGEMS_CHECK_RECOVERABLE(std::isfinite(weight),
                          "Source weight must be finite.");

  GGEMS_CHECK_RECOVERABLE(weight >= 0.0F,
                          "Source weight must be positive or zero.");

  record_.weight = weight;

  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSSource::Verbose() const {
  GGEMSSourceType const source_type = FromKernelSourceType(record_.source_type);

  particles::GGEMSParticleType const particle_type =
      particles::FromKernelParticleType(record_.emitted_particle_type);

  GGEMS_INFO(
      "Source",
      "Source {}: type={}, particle={} ({}), energy={}, time=[{}, {}], "
      "position=({}, {}, {}), direction=({}, {}, {}), weight={}.",
      record_.source_id, ToLongName(source_type),
      particles::ToLongName(particle_type),
      particles::ToShortName(particle_type),
      ggems::units::HumanReadable(
          ggems::units::Energy{record_.energy_milli_eV}),
      ggems::units::HumanReadable(ggems::units::Time{record_.time_start_ps}),
      ggems::units::HumanReadable(ggems::units::Time{record_.time_stop_ps}),
      ggems::units::HumanReadableSignedLength(record_.position_x_pm),
      ggems::units::HumanReadableSignedLength(record_.position_y_pm),
      ggems::units::HumanReadableSignedLength(record_.position_z_pm),
      record_.direction_x, record_.direction_y, record_.direction_z,
      record_.weight);
}
} // namespace ggems::core::sources
