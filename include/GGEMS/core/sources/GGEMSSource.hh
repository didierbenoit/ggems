#pragma once

#include <array>
#include <cstdint>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace ggems::core::sources {

class GGEMSSource {
public:
  GGEMSSource();
  ~GGEMSSource() = default;

  GGEMSSource(GGEMSSource const &) = default;
  GGEMSSource(GGEMSSource &&) = default;
  auto operator=(GGEMSSource const &) -> GGEMSSource & = default;
  auto operator=(GGEMSSource &&) -> GGEMSSource & = default;

  auto SetPrimaryCount(std::uint64_t primary_count) noexcept -> GGEMSSource &;

  [[nodiscard]] auto GetPrimaryCount() const noexcept -> std::uint64_t {
    return primary_count_;
  }

  auto SetAnalytic() noexcept -> GGEMSSource &;

  auto SetPointEmission() -> GGEMSSource &;

  auto SetRectangleEmissionPicoMeter(std::uint64_t width_pm,
                                     std::uint64_t height_pm) -> GGEMSSource &;

  auto SetEllipseEmissionPicoMeter(std::uint64_t diameter_x_pm,
                                   std::uint64_t diameter_y_pm)
      -> GGEMSSource &;

  auto SetCircleEmissionPicoMeter(std::uint64_t diameter_pm) -> GGEMSSource &;

  auto SetFixedAngularDistribution() -> GGEMSSource &;
  auto SetIsotropicAngularDistribution() -> GGEMSSource &;

  auto SetFocusedAngularDistributionPicoMeter(std::int64_t focus_x_pm,
                                              std::int64_t focus_y_pm,
                                              std::int64_t focus_z_pm)
      -> GGEMSSource &;

  auto
  SetEmittedParticleType(particles::GGEMSParticleType particle_type) noexcept
      -> GGEMSSource &;

  auto SetEnergyMilliElectronVolt(std::uint64_t energy_milli_eV)
      -> GGEMSSource &;

  auto SetTimeWindowPicoSecond(std::uint64_t time_start_ps,
                               std::uint64_t time_stop_ps) -> GGEMSSource &;

  auto SetPositionPicoMeter(std::int64_t x_pm, std::int64_t y_pm,
                            std::int64_t z_pm) -> GGEMSSource &;

  auto SetDirection(double dir_x, double dir_y, double dir_z) -> GGEMSSource &;

  auto SetOrientation(std::array<double, 3U> const &direction,
                      std::array<double, 3U> const &up_reference)
      -> GGEMSSource &;

  auto SetWeight(float weight) -> GGEMSSource &;

  [[nodiscard]] auto GetRecord() const noexcept -> GGEMSSourceRecord const & {
    return record_;
  }

  [[nodiscard]] auto BuildRecord() const -> GGEMSSourceRecord;

  auto Verbose() const -> void;

private:
  std::uint64_t primary_count_{4096ULL};
  GGEMSSourceRecord record_{};
};

} // namespace ggems::core::sources
