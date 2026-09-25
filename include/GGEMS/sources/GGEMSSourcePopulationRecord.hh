#pragma once

#include <cstdint>

#include "GGEMS/sources/GGEMSSourcePopulation.hh"

namespace ggems::core::sources {

[[nodiscard]] constexpr auto
ToKernelSourcePopulationMode(GGEMSSourcePopulationMode mode) noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(mode);
}

struct GGEMSSourcePopulationRecord {
  std::uint32_t population_mode{
    ToKernelSourcePopulationMode(GGEMSSourcePopulationMode::CountDriven)};
  std::uint32_t first_emission_index{0U};
  std::uint32_t emission_count{0U};
  float scaled_decay{0.0F};
};

} // namespace ggems::core::sources
