#pragma once

#include <cstdint>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

class GGEMSSource {
public:
  GGEMSSource();
  ~GGEMSSource() = default;

  GGEMSSource(GGEMSSource const &) = default;
  GGEMSSource(GGEMSSource &&) = default;
  GGEMSSource &operator=(GGEMSSource const &) = default;
  GGEMSSource &operator=(GGEMSSource &&) = default;

public:
  GGEMSSource &SetPrimaryCount(std::uint64_t primary_count) noexcept;

  [[nodiscard]] std::uint64_t GetPrimaryCount() const noexcept {
    return primary_count_;
  }

  GGEMSSource &SetAnalytic() noexcept;

  GGEMSSource &
  SetEmittedParticleType(particles::GGEMSParticleType particle_type) noexcept;

  GGEMSSource &SetEnergyMilliElectronVolt(std::uint64_t energy_milli_eV);

  GGEMSSource &SetTimeWindowPicoSecond(std::uint64_t time_start_ps,
                                       std::uint64_t time_stop_ps);

  GGEMSSource &SetPositionPicoMeter(std::int64_t x_pm, std::int64_t y_pm,
                                    std::int64_t z_pm) noexcept;

  GGEMSSource &SetDirection(float x, float y, float z);

  GGEMSSource &SetWeight(float weight);

  [[nodiscard]] GGEMSSourceRecord const &GetRecord() const noexcept {
    return record_;
  }

  [[nodiscard]] GGEMSSourceRecord BuildRecord() const noexcept {
    return record_;
  }

  void Verbose() const;

private:
  std::uint64_t primary_count_{4096ULL};
  GGEMSSourceRecord record_{};
};

} // namespace ggems::core::sources
