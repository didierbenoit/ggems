#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
#include <utility>

#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourceFrame.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceValidation.hh"

namespace ggems::core::sources {
namespace {

// =============================================================================
// =============================================================================

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

// =============================================================================
// =============================================================================

auto CommitValidatedRecord(GGEMSSourceRecord &record,
                           GGEMSSourceRecord candidate) -> void {
  ValidateAnalyticSourceRecord(candidate);
  record = candidate;
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

  record_.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Point);
  record_.angular_distribution_type =
      ToKernelAngularDistributionType(GGEMSAngularDistributionType::Fixed);
  record_.geometry_size_x_pm = 0ULL;
  record_.geometry_size_y_pm = 0ULL;
  record_.focus_position_x_pm = 0ULL;
  record_.focus_position_y_pm = 0ULL;
  record_.focus_position_z_pm = 0ULL;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::CheckEnergyConfigurationMutable() const -> void {
  GGEMS_CHECK_RECOVERABLE(!initialization_finalized_,
                          "Cannot change GGEMSSource energy after successful "
                          "GGEMSRun::Initialise.");
}

// -----------------------------------------------------------------------------

auto GGEMSSource::FinalizeInitialization() noexcept -> void {
  initialization_finalized_ = true;
}

// -----------------------------------------------------------------------------

GGEMSSource::GGEMSSource(GGEMSSource &&other) {
  other.CheckEnergyConfigurationMutable();

  primary_count_ = other.primary_count_;
  record_ = other.record_;
  energy_distribution_ = std::move(other.energy_distribution_);
}

// -----------------------------------------------------------------------------

auto GGEMSSource::operator=(GGEMSSource const &other) -> GGEMSSource & {
  if (this == &other) {
    return *this;
  }

  CheckEnergyConfigurationMutable();
  GGEMSSource candidate{other};

  primary_count_ = candidate.primary_count_;
  record_ = candidate.record_;
  energy_distribution_ = std::move(candidate.energy_distribution_);
  initialization_finalized_ = candidate.initialization_finalized_;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::operator=(GGEMSSource &&other) -> GGEMSSource & {
  if (this == &other) {
    return *this;
  }

  CheckEnergyConfigurationMutable();
  other.CheckEnergyConfigurationMutable();

  primary_count_ = other.primary_count_;
  record_ = other.record_;
  energy_distribution_ = std::move(other.energy_distribution_);
  initialization_finalized_ = false;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::CommitEnergyDistribution(
    GGEMSEnergyDistribution distribution) noexcept -> void {
  std::uint64_t const source_record_energy =
      distribution.GetType() == GGEMSEnergyDistributionType::Mono
          ? distribution.GetMonoEnergyMilliElectronVolt()
          : 0ULL;

  energy_distribution_ = std::move(distribution);
  record_.energy_milli_eV = source_record_energy;
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

auto GGEMSSource::SetPointEmission() -> GGEMSSource & {
  GGEMSSourceRecord candidate = record_;
  candidate.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Point);
  candidate.geometry_size_x_pm = 0ULL;
  candidate.geometry_size_y_pm = 0ULL;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetRectangleEmissionPicoMeter(std::uint64_t width_pm,
                                                std::uint64_t height_pm)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(width_pm > 0ULL && height_pm > 0ULL,
                          "Rectangle width and height must be non-zero.");

  GGEMSSourceRecord candidate = record_;
  candidate.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Rectangle);
  candidate.geometry_size_x_pm = width_pm;
  candidate.geometry_size_y_pm = height_pm;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetEllipseEmissionPicoMeter(std::uint64_t diameter_x_pm,
                                              std::uint64_t diameter_y_pm)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(diameter_x_pm > 0ULL && diameter_y_pm > 0ULL,
                          "Ellipse diameters must be non-zero");

  GGEMSSourceRecord candidate = record_;
  candidate.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Ellipse);
  candidate.geometry_size_x_pm = diameter_x_pm;
  candidate.geometry_size_y_pm = diameter_y_pm;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetCircleEmissionPicoMeter(std::uint64_t diameter_pm)
    -> GGEMSSource & {
  return SetEllipseEmissionPicoMeter(diameter_pm, diameter_pm);
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetFixedAngularDistribution() -> GGEMSSource & {
  GGEMSSourceRecord candidate = record_;
  candidate.angular_distribution_type =
      ToKernelAngularDistributionType(GGEMSAngularDistributionType::Fixed);
  candidate.focus_position_x_pm = 0ULL;
  candidate.focus_position_y_pm = 0ULL;
  candidate.focus_position_z_pm = 0ULL;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetIsotropicAngularDistribution() -> GGEMSSource & {
  GGEMSSourceRecord candidate = record_;
  candidate.angular_distribution_type =
      ToKernelAngularDistributionType(GGEMSAngularDistributionType::Isotropic);
  candidate.focus_position_x_pm = 0ULL;
  candidate.focus_position_y_pm = 0ULL;
  candidate.focus_position_z_pm = 0ULL;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetFocusedAngularDistributionPicoMeter(
    std::int64_t focus_x_pm, std::int64_t focus_y_pm, std::int64_t focus_z_pm)
    -> GGEMSSource & {
  GGEMSSourceRecord candidate = record_;
  candidate.angular_distribution_type =
      ToKernelAngularDistributionType(GGEMSAngularDistributionType::Focused);
  candidate.focus_position_x_pm = focus_x_pm;
  candidate.focus_position_y_pm = focus_y_pm;
  candidate.focus_position_z_pm = focus_z_pm;
  CommitValidatedRecord(record_, candidate);
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
  CheckEnergyConfigurationMutable();
  CommitEnergyDistribution(GGEMSEnergyDistribution::BuildMono(energy_milli_eV));
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetDiscreteEnergyLines(
    std::span<double const> energies, std::span<double const> relative_weights,
    std::string_view unit) -> GGEMSSource & {
  CheckEnergyConfigurationMutable();
  CommitEnergyDistribution(GGEMSEnergyDistribution::BuildDiscreteLines(
      energies, relative_weights, unit));
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetRegularEnergySpectrum(
    std::span<double const> bin_centers,
    std::span<double const> relative_bin_weights, std::string_view unit)
    -> GGEMSSource & {
  CheckEnergyConfigurationMutable();
  CommitEnergyDistribution(GGEMSEnergyDistribution::BuildRegularSpectrum(
      bin_centers, relative_bin_weights, unit));
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::LoadRegularEnergySpectrum(
    std::filesystem::path const &filename, std::string_view unit)
    -> GGEMSSource & {
  CheckEnergyConfigurationMutable();
  CommitEnergyDistribution(
      GGEMSEnergyDistribution::LoadRegularSpectrum(filename, unit));
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
                                       std::int64_t z_pm) -> GGEMSSource & {
  GGEMSSourceRecord candidate = record_;
  candidate.position_x_pm = x_pm;
  candidate.position_y_pm = y_pm;
  candidate.position_z_pm = z_pm;
  CommitValidatedRecord(record_, candidate);

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
  GGEMSSourceRecord candidate = record_;
  StoreSourceFrame(candidate, frame);
  CommitValidatedRecord(record_, candidate);
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

auto GGEMSSource::BuildRecord() const -> GGEMSSourceRecord {
  std::uint64_t const expected_energy =
      energy_distribution_.GetType() == GGEMSEnergyDistributionType::Mono
          ? energy_distribution_.GetMonoEnergyMilliElectronVolt()
          : 0ULL;

  GGEMS_CHECK_INTERNAL(
      record_.energy_milli_eV == expected_energy,
      "GGEMSSource record and energy distribution are inconsistent.");

  ValidateAnalyticSourceRecord(record_);
  return record_;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::Verbose() const -> void {
  GGEMSSourceRecord const source_record = BuildRecord();
  GGEMSEnergyDistributionRecord const energy_record =
      energy_distribution_.BuildRecord(0ULL);

  GGEMS_INFO(
      "Source", "{}",
      DescribeSource(source_record, primary_count_, energy_record,
                     energy_distribution_.GetEnergyValuesMilliElectronVolt()));
}
} // namespace ggems::core::sources
