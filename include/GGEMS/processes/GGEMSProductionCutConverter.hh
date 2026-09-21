#pragma once

#include <cstdint>

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

[[nodiscard]] auto
ConvertProductionCutLength(GGEMSProductionCutChannel channel,
                           units::Length length,
                           materials::GGEMSEMMaterialPackage const &materials,
                           std::uint32_t material_id) -> units::Energy;

} // namespace ggems::core::processes
