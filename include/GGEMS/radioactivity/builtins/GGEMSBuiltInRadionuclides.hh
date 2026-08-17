#pragma once

#include <optional>
#include <string_view>

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::core::radioactivity::builtins {

[[nodiscard]] auto BuildF18Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildC11Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildO15Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildLu177Radionuclide() -> GGEMSRadionuclideDefinition;

[[nodiscard]] auto BuildBuiltInRadionuclide(std::string_view canonical_name)
    -> std::optional<GGEMSRadionuclideDefinition>;

} // namespace ggems::core::radioactivity::builtins
