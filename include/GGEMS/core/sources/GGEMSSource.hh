#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <memory>
#include <string_view>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"

namespace ggems::core {
class GGEMSRun;
}

namespace ggems::core::sources {

class GGEMSSource {
public:
  GGEMSSource();
  ~GGEMSSource() = default;

  GGEMSSource(GGEMSSource const &) = default;
  GGEMSSource(GGEMSSource &&other);
  auto operator=(GGEMSSource const &other) -> GGEMSSource &;
  auto operator=(GGEMSSource &&other) -> GGEMSSource &;

  auto SetPrimaryCount(std::uint64_t primary_count) -> GGEMSSource &;

  [[nodiscard]] auto GetPrimaryCount() const -> std::uint64_t;

  auto SetCountDrivenPopulation(std::uint64_t primary_count) -> GGEMSSource &;

  auto SetActivityDrivenRadionuclide(
      std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>
          radionuclide,
      units::Activity activity_at_reference_time,
      std::uint64_t reference_time_ps) -> GGEMSSource &;

  [[nodiscard]] auto GetPopulationMode() const noexcept
      -> GGEMSSourcePopulationMode;

  [[nodiscard]] auto GetPopulationConfiguration() const noexcept
      -> GGEMSSourcePopulationConfiguration const & {
    return population_configuration_;
  }

  [[nodiscard]] auto GetActivityDrivenConfiguration() const
      -> GGEMSActivityDrivenSourceConfiguration const &;

  auto SetAnalytic() noexcept -> GGEMSSource &;

  auto SetPointEmission() -> GGEMSSource &;

  auto SetRectangleEmissionPicoMeter(std::uint64_t width_pm,
                                     std::uint64_t height_pm) -> GGEMSSource &;

  auto SetEllipseEmissionPicoMeter(std::uint64_t diameter_x_pm,
                                   std::uint64_t diameter_y_pm)
      -> GGEMSSource &;

  auto SetCircleEmissionPicoMeter(std::uint64_t diameter_pm) -> GGEMSSource &;

  auto SetBoxEmissionPicoMeter(std::uint64_t width_pm, std::uint64_t height_pm,
                               std::uint64_t depth_pm) -> GGEMSSource &;

  auto SetSphereEmissionPicoMeter(std::uint64_t diameter_pm) -> GGEMSSource &;

  auto SetCylinderEmissionPicoMeter(std::uint64_t diameter_pm,
                                    std::uint64_t height_pm) -> GGEMSSource &;

  auto SetFixedAngularDistribution() -> GGEMSSource &;
  auto SetIsotropicAngularDistribution() -> GGEMSSource &;
  auto SetIsotropicAngularDistribution(ggems::units::Angle theta_min,
                                       ggems::units::Angle theta_max,
                                       ggems::units::Angle phi_min,
                                       ggems::units::Angle phi_max)
      -> GGEMSSource &;

  auto SetFocusedAngularDistributionPicoMeter(std::int64_t focus_x_pm,
                                              std::int64_t focus_y_pm,
                                              std::int64_t focus_z_pm)
      -> GGEMSSource &;

  auto SetEmittedParticleType(particles::GGEMSParticleType particle_type)
      -> GGEMSSource &;

  auto SetEnergyMilliElectronVolt(std::uint64_t energy_milli_eV)
      -> GGEMSSource &;

  auto SetDiscreteEnergyLines(std::span<double const> energies,
                              std::span<double const> relative_weights,
                              std::string_view unit) -> GGEMSSource &;

  auto SetRegularEnergySpectrum(std::span<double const> bin_centers,
                                std::span<double const> relative_bin_weights,
                                std::string_view unit) -> GGEMSSource &;

  auto LoadRegularEnergySpectrum(std::filesystem::path const &filename,
                                 std::string_view unit) -> GGEMSSource &;

  [[nodiscard]] auto GetEnergyDistribution() const
      -> GGEMSEnergyDistribution const &;

  auto SetPositionPicoMeter(std::int64_t x_pm, std::int64_t y_pm,
                            std::int64_t z_pm) -> GGEMSSource &;

  auto SetDirection(double dir_x, double dir_y, double dir_z) -> GGEMSSource &;

  auto SetOrientation(std::array<double, 3U> const &direction,
                      std::array<double, 3U> const &up_reference)
      -> GGEMSSource &;

  auto SetWeight(float weight) -> GGEMSSource &;

  [[nodiscard]] auto GetRecord() const -> GGEMSSourceRecord const &;

  [[nodiscard]] auto BuildExecutionRecord() const -> GGEMSSourceRecord;

  [[nodiscard]] auto BuildRecord() const -> GGEMSSourceRecord;

  auto Verbose() const -> void;

private:
  friend class ggems::core::GGEMSRun;

  auto CheckCountDrivenConfiguration() const -> void;
  auto CheckEnergyConfigurationMutable() const -> void;
  auto CheckPopulationConfigurationMutable() const -> void;
  auto FinalizeInitialization() noexcept -> void;

  auto CommitEnergyDistribution(GGEMSEnergyDistribution distribution) noexcept
      -> void;

  GGEMSSourcePopulationConfiguration population_configuration_{
      GGEMSCountDrivenSourceConfiguration{}};
  GGEMSSourceRecord record_{};
  GGEMSEnergyDistribution energy_distribution_;
  bool initialization_finalized_{false};
};

} // namespace ggems::core::sources
