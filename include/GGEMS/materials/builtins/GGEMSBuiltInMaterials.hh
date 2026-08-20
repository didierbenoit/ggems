#pragma once

#include <span>
#include <string_view>

#include "GGEMS/materials/GGEMSMaterial.hh"

namespace ggems::core::materials::builtins {

[[nodiscard]] auto GetAvailableMaterialNames() noexcept
    -> std::span<std::string_view const>;

[[nodiscard]] auto BuildBuiltInMaterial(std::string_view canonical_name)
    -> GGEMSMaterial;

} // namespace ggems::core::materials::builtins
