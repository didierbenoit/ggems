#pragma once

#include <optional>
#include <string_view>

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::core::radioactivity::builtins {

[[nodiscard]] auto BuildH3Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildC14Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildF18Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildC11Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildO15Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildGa68Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildCo60Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildLu177Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildI131Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildAm241Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildTc99mRadionuclide() -> GGEMSRadionuclideDefinition;

[[nodiscard]] auto BuildBuiltInRadionuclide(std::string_view canonical_name)
    -> std::optional<GGEMSRadionuclideDefinition>;

} // namespace ggems::core::radioactivity::builtins
