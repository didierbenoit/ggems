#include <array>
#include <cmath>
#include <cstdint>

#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourceFrame.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace ggems::core::sources {
namespace {
auto StoreSourceFrame(GGEMSSourceRecord &record,
                      GGEMSSourceFrame const &frame) noexcept -> void {
  record.axis_x_x = frame.axis_x.x;
  record.axis_x_y = frame.axis_x.y;
  record.axis_x_z = frame.axis_x.z;

  record.axis_y_x = frame.axis_y.x;
  record.axis_y_y = frame.axis_y.y;
  record.axis_y_z = frame.axis_y.z;

  record.axis_z_x = frame.axis_z.x;
  record.axis_z_y = frame.axis_z.y;
  record.axis_z_z = frame.axis_z.z;
}
} // namespace

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

  StoreSourceFrame(record_, GGEMSSourceFrame{});

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

auto GGEMSSource::SetDirection(double dir_x, double dir_y, double dir_z)
    -> GGEMSSource & {

  GGEMSSourceFrame const frame =
      BuildSourceFrameWithAutomaticUp({dir_x, dir_y, dir_z});
  StoreSourceFrame(record_, frame);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetOrientation(std::array<double, 3U> const &direction,
                                 std::array<double, 3U> const &up_reference)
    -> GGEMSSource & {
  GGEMSSourceFrame const frame = BuildSourceFrame(direction, up_reference);
  StoreSourceFrame(record_, frame);
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
