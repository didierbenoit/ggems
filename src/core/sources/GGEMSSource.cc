#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
#include <utility>
#include <numbers>
#include <limits>
#include <variant>
#include <memory>

#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourceFrame.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceValidation.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"

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

// =============================================================================
// =============================================================================

auto StoreFullSphereIsotropicDomain(GGEMSSourceRecord &record) noexcept
    -> void {
  record.isotropic_cos_theta_lower = k_isotropic_full_sphere_cos_theta_lower;
  record.isotropic_cos_theta_upper = k_isotropic_full_sphere_cos_theta_upper;
  record.isotropic_phi_min_rad = k_isotropic_full_sphere_phi_min_rad;
  record.isotropic_phi_max_rad = k_isotropic_full_sphere_phi_max_rad;
}

// =============================================================================
// =============================================================================
[[nodiscard]] auto IsFiniteBinary32(long double value) noexcept -> bool {
  constexpr long double k_limit{
      static_cast<long double>(std::numeric_limits<float>::max())};
  return std::isfinite(value) && value >= -k_limit && value <= k_limit;
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
  record_.time_stop_ps = 0ULL;

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
  record_.geometry_size_z_pm = 0ULL;
  record_.focus_position_x_pm = 0ULL;
  record_.focus_position_y_pm = 0ULL;
  record_.focus_position_z_pm = 0ULL;
  StoreFullSphereIsotropicDomain(record_);
}

// -----------------------------------------------------------------------------

auto GGEMSSource::CheckCountDrivenConfiguration() const -> void {
  GGEMS_CHECK_RECOVERABLE(
      GetPopulationMode() == GGEMSSourcePopulationMode::CountDriven,
      "Cannot use single-particle configuration on an ActivityDriven "
      "GGEMSSource.");
}

// -----------------------------------------------------------------------------

auto GGEMSSource::CheckEnergyConfigurationMutable() const -> void {
  CheckCountDrivenConfiguration();
  GGEMS_CHECK_RECOVERABLE(!initialization_finalized_,
                          "Cannot change GGEMSSource energy after successful "
                          "GGEMSRun::Initialise.");
}

// -----------------------------------------------------------------------------

auto GGEMSSource::CheckPopulationConfigurationMutable() const -> void {
  GGEMS_CHECK_RECOVERABLE(
      !initialization_finalized_,
      "Cannot change GGEMSSource population mode after successful "
      "GGEMSRun::Initialise.");
}

// -----------------------------------------------------------------------------

auto GGEMSSource::FinalizeInitialization() noexcept -> void {
  initialization_finalized_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::BuildExecutionRecord() const -> GGEMSSourceRecord {
  GGEMSSourceRecord record = record_;

  if (GetPopulationMode() == GGEMSSourcePopulationMode::ActivityDriven) {
    record.emitted_particle_type =
        particles::ToKernelParticleType(particles::GGEMSParticleType::Unknown);
    record.energy_milli_eV = 0ULL;
  }

  ValidateAnalyticSourceRecord(record);
  return record;
}

// -----------------------------------------------------------------------------

GGEMSSource::GGEMSSource(GGEMSSource &&other) {
  other.CheckPopulationConfigurationMutable();

  population_configuration_ = std::move(other.population_configuration_);
  record_ = other.record_;
  energy_distribution_ = std::move(other.energy_distribution_);
}

// -----------------------------------------------------------------------------

auto GGEMSSource::operator=(GGEMSSource const &other) -> GGEMSSource & {
  if (this == &other) {
    return *this;
  }

  CheckPopulationConfigurationMutable();
  GGEMSSource candidate{other};

  population_configuration_ = std::move(candidate.population_configuration_);
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

  CheckPopulationConfigurationMutable();
  other.CheckPopulationConfigurationMutable();

  population_configuration_ = std::move(other.population_configuration_);
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

auto GGEMSSource::SetPrimaryCount(std::uint64_t primary_count)
    -> GGEMSSource & {
  CheckCountDrivenConfiguration();
  std::get<GGEMSCountDrivenSourceConfiguration>(population_configuration_)
      .primary_count = primary_count;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::GetPrimaryCount() const -> std::uint64_t {
  CheckCountDrivenConfiguration();
  return std::get<GGEMSCountDrivenSourceConfiguration>(
             population_configuration_)
      .primary_count;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetCountDrivenPopulation(std::uint64_t primary_count)
    -> GGEMSSource & {
  CheckPopulationConfigurationMutable();
  population_configuration_ =
      GGEMSCountDrivenSourceConfiguration{.primary_count = primary_count};
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetActivityDrivenRadionuclide(
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>
        radionuclide,
    units::Activity activity_at_reference_time, std::uint64_t reference_time_ps)
    -> GGEMSSource & {
  CheckPopulationConfigurationMutable();
  GGEMS_CHECK_RECOVERABLE(
      radionuclide != nullptr,
      "ActivityDriven GGEMSSource requires a radionuclide definition.");
  GGEMS_CHECK_RECOVERABLE(
      std::isfinite(activity_at_reference_time.value),
      "ActivityDriven GGEMSSource activity must be finite.");
  GGEMS_CHECK_RECOVERABLE(
      activity_at_reference_time.value >= 0.0L,
      "ActivityDriven GGEMSSource activity must be non-negative.");

  population_configuration_ = GGEMSActivityDrivenSourceConfiguration{
      .radionuclide = std::move(radionuclide),
      .activity_at_reference_time = activity_at_reference_time,
      .reference_time_ps = reference_time_ps};
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::GetPopulationMode() const noexcept
    -> GGEMSSourcePopulationMode {
  return std::holds_alternative<GGEMSCountDrivenSourceConfiguration>(
             population_configuration_)
             ? GGEMSSourcePopulationMode::CountDriven
             : GGEMSSourcePopulationMode::ActivityDriven;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::GetActivityDrivenConfiguration() const
    -> GGEMSActivityDrivenSourceConfiguration const & {
  GGEMS_CHECK_RECOVERABLE(
      GetPopulationMode() == GGEMSSourcePopulationMode::ActivityDriven,
      "GGEMSSource is not configured in ActivityDriven mode.");
  return std::get<GGEMSActivityDrivenSourceConfiguration>(
      population_configuration_);
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
  candidate.geometry_size_z_pm = 0ULL;
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
  candidate.geometry_size_z_pm = 0ULL;
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
  candidate.geometry_size_z_pm = 0ULL;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetCircleEmissionPicoMeter(std::uint64_t diameter_pm)
    -> GGEMSSource & {
  return SetEllipseEmissionPicoMeter(diameter_pm, diameter_pm);
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetBoxEmissionPicoMeter(std::uint64_t width_pm,
                                          std::uint64_t height_pm,
                                          std::uint64_t depth_pm)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(width_pm > 0ULL && height_pm > 0ULL &&
                              depth_pm > 0ULL,
                          "Box dimensions must be strictly positive.");

  GGEMSSourceRecord candidate = record_;
  candidate.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Box);
  candidate.geometry_size_x_pm = width_pm;
  candidate.geometry_size_y_pm = height_pm;
  candidate.geometry_size_z_pm = depth_pm;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetSphereEmissionPicoMeter(std::uint64_t diameter_pm)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(diameter_pm > 0ULL,
                          "Sphere diameter must be strictly positive.");

  GGEMSSourceRecord candidate = record_;
  candidate.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Sphere);
  candidate.geometry_size_x_pm = diameter_pm;
  candidate.geometry_size_y_pm = diameter_pm;
  candidate.geometry_size_z_pm = diameter_pm;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetCylinderEmissionPicoMeter(std::uint64_t diameter_pm,
                                               std::uint64_t height_pm)
    -> GGEMSSource & {
  GGEMS_CHECK_RECOVERABLE(
      diameter_pm > 0ULL && height_pm > 0ULL,
      "Cylinder diameter and height must be strictly positive.");

  GGEMSSourceRecord candidate = record_;
  candidate.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Cylinder);
  candidate.geometry_size_x_pm = diameter_pm;
  candidate.geometry_size_y_pm = diameter_pm;
  candidate.geometry_size_z_pm = height_pm;
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetFixedAngularDistribution() -> GGEMSSource & {
  GGEMSSourceRecord candidate = record_;
  candidate.angular_distribution_type =
      ToKernelAngularDistributionType(GGEMSAngularDistributionType::Fixed);
  candidate.focus_position_x_pm = 0ULL;
  candidate.focus_position_y_pm = 0ULL;
  candidate.focus_position_z_pm = 0ULL;
  StoreFullSphereIsotropicDomain(candidate);
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
  StoreFullSphereIsotropicDomain(candidate);
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetIsotropicAngularDistribution(ggems::units::Angle theta_min,
                                                  ggems::units::Angle theta_max,
                                                  ggems::units::Angle phi_min,
                                                  ggems::units::Angle phi_max)
    -> GGEMSSource & {
  constexpr long double k_pi{std::numbers::pi_v<long double>};
  constexpr long double k_two_pi{2.0L * k_pi};

  long double const theta_min_rad = ggems::units::ToRadians(theta_min);
  long double const theta_max_rad = ggems::units::ToRadians(theta_max);
  long double const phi_min_rad = ggems::units::ToRadians(phi_min);
  long double const phi_max_rad = ggems::units::ToRadians(phi_max);

  GGEMS_CHECK_RECOVERABLE(
      std::isfinite(theta_min_rad) && std::isfinite(theta_max_rad) &&
          std::isfinite(phi_min_rad) && std::isfinite(phi_max_rad),
      "Isotropic angular bounds must be finite.");

  GGEMS_CHECK_RECOVERABLE(
      theta_min_rad >= 0.0L && theta_min_rad < theta_max_rad &&
          theta_max_rad <= k_pi,
      "Isotropic theta bounds must satisfy 0 <= min < max <= pi.");

  long double const phi_width_rad = phi_max_rad - phi_min_rad;
  GGEMS_CHECK_RECOVERABLE(phi_max_rad > phi_min_rad &&
                              std::isfinite(phi_width_rad) &&
                              phi_width_rad <= k_two_pi,
                          "Isotropic phi bounds must have width in (0, 2*pi].");

  GGEMSSourceRecord candidate = record_;
  candidate.angular_distribution_type =
      ToKernelAngularDistributionType(GGEMSAngularDistributionType::Isotropic);
  candidate.focus_position_x_pm = 0LL;
  candidate.focus_position_y_pm = 0LL;
  candidate.focus_position_z_pm = 0LL;

  long double const cos_theta_lower = std::cos(theta_max_rad);
  long double const cos_theta_upper = std::cos(theta_min_rad);

  GGEMS_CHECK_RECOVERABLE(
      IsFiniteBinary32(cos_theta_lower) && IsFiniteBinary32(cos_theta_upper) &&
          IsFiniteBinary32(phi_min_rad) && IsFiniteBinary32(phi_max_rad),
      "Isotropic angular bounds cannot be represented in binary32.");

  auto const stored_cos_lower = static_cast<float>(cos_theta_lower);
  auto const stored_cos_upper = static_cast<float>(cos_theta_upper);
  auto const stored_phi_min = static_cast<float>(phi_min_rad);
  auto const stored_phi_max = static_cast<float>(phi_max_rad);
  long double const stored_phi_width =
      static_cast<long double>(stored_phi_max) -
      static_cast<long double>(stored_phi_min);

  GGEMS_CHECK_RECOVERABLE(
      stored_cos_lower < stored_cos_upper && stored_phi_min < stored_phi_max &&
          std::isfinite(stored_phi_width) &&
          stored_phi_width <=
              static_cast<long double>(k_isotropic_full_sphere_phi_max_rad),
      "Isotropic angular domain collapses or exceeds its binary32 contract.");

  candidate.isotropic_cos_theta_lower = stored_cos_lower;
  candidate.isotropic_cos_theta_upper = stored_cos_upper;
  candidate.isotropic_phi_min_rad = stored_phi_min;
  candidate.isotropic_phi_max_rad = stored_phi_max;

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
  StoreFullSphereIsotropicDomain(candidate);
  CommitValidatedRecord(record_, candidate);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::SetEmittedParticleType(
    particles::GGEMSParticleType particle_type) -> GGEMSSource & {
  CheckCountDrivenConfiguration();
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

auto GGEMSSource::GetEnergyDistribution() const
    -> GGEMSEnergyDistribution const & {
  CheckCountDrivenConfiguration();
  return energy_distribution_;
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
  GGEMSSourceRecord candidate = record_;
  StoreSourceFrame(candidate, frame);
  CommitValidatedRecord(record_, candidate);
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
  CheckCountDrivenConfiguration();
  GGEMSSourceRecord const record = BuildExecutionRecord();
  std::uint64_t const expected_energy =
      energy_distribution_.GetType() == GGEMSEnergyDistributionType::Mono
          ? energy_distribution_.GetMonoEnergyMilliElectronVolt()
          : 0ULL;

  GGEMS_CHECK_INTERNAL(
      record.energy_milli_eV == expected_energy,
      "GGEMSSource record and energy distribution are inconsistent.");

  return record;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::GetRecord() const -> GGEMSSourceRecord const & {
  CheckCountDrivenConfiguration();
  return record_;
}

// -----------------------------------------------------------------------------

auto GGEMSSource::Verbose() const -> void {
  GGEMSSourceRecord const source_record = BuildRecord();
  GGEMSEnergyDistributionRecord const energy_record =
      energy_distribution_.BuildRecord(0ULL);

  GGEMS_INFO(
      "Source", "{}",
      DescribeSource(source_record, GetPrimaryCount(), energy_record,
                     energy_distribution_.GetEnergyValuesMilliElectronVolt()));
}
} // namespace ggems::core::sources
