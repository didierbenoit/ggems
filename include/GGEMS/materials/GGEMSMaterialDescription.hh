#pragma once

#include <string>

#include "GGEMS/materials/GGEMSMaterial.hh"

namespace ggems::core::materials {

[[nodiscard]] auto DescribeMaterial(GGEMSMaterial const &material)
    -> std::string;

[[nodiscard]] auto DescribeAvailableMaterials() -> std::string;

auto VerboseMaterial(GGEMSMaterial const &Material) -> void;

auto VerboseAvailableMaterials() -> void;

} // namespace ggems::core::materials
