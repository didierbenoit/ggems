#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"

namespace ggems::core::sources {

[[nodiscard]] constexpr auto
ToKernelSourcePopulationMode(GGEMSSourcePopulationMode mode) noexcept
    -> std::uint32_t {
  return mode == GGEMSSourcePopulationMode::ActivityDriven ? 1U : 0U;
}

struct GGEMSSourcePopulationRecord {
  std::uint32_t population_mode{
      ToKernelSourcePopulationMode(GGEMSSourcePopulationMode::CountDriven)};
  std::uint32_t first_emission_index{0U};
  std::uint32_t emission_count{0U};
  float scaled_decay{0.0F};
};

static_assert(std::is_standard_layout_v<GGEMSSourcePopulationRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSSourcePopulationRecord>);
static_assert(sizeof(GGEMSSourcePopulationRecord) == 16U);
static_assert(alignof(GGEMSSourcePopulationRecord) == 4U);
static_assert(offsetof(GGEMSSourcePopulationRecord, population_mode) == 0U);
static_assert(offsetof(GGEMSSourcePopulationRecord, first_emission_index) ==
              4U);
static_assert(offsetof(GGEMSSourcePopulationRecord, emission_count) == 8U);
static_assert(offsetof(GGEMSSourcePopulationRecord, scaled_decay) == 12U);

} // namespace ggems::core::sources
