#pragma once

#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"

namespace ggems::core::materials {

[[nodiscard]] auto GetIsotopeMassAuthority()
    -> GGEMSResolvedIsotopeTable const &;
} // namespace ggems::core::materials
