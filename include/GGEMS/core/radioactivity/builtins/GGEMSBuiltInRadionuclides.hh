#pragma once

#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::core::radioactivity::builtins {

[[nodiscard]] auto BuildF18PositronSpectrum() -> GGEMSBetaSpectrumBuildResult;

[[nodiscard]] auto BuildF18Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildC11Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildO15Radionuclide() -> GGEMSRadionuclideDefinition;

} // namespace ggems::core::radioactivity::builtins
