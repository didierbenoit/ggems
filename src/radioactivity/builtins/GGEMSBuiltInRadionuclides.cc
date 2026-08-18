#include <optional>
#include <string_view>

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace ggems::core::radioactivity::builtins {

[[nodiscard]] auto BuildBuiltInRadionuclide(std::string_view canonical_name)
    -> std::optional<GGEMSRadionuclideDefinition> {
  if (canonical_name == "H-3") {
    return BuildH3Radionuclide();
  }

  if (canonical_name == "C-14") {
    return BuildC14Radionuclide();
  }

  if (canonical_name == "F-18") {
    return BuildF18Radionuclide();
  }

  if (canonical_name == "C-11") {
    return BuildC11Radionuclide();
  }

  if (canonical_name == "O-15") {
    return BuildO15Radionuclide();
  }

  if (canonical_name == "Ga-68") {
    return BuildGa68Radionuclide();
  }

  if (canonical_name == "Co-60") {
    return BuildCo60Radionuclide();
  }

  if (canonical_name == "Lu-177") {
    return BuildLu177Radionuclide();
  }

  if (canonical_name == "I-131") {
    return BuildI131Radionuclide();
  }

  if (canonical_name == "Am-241") {
    return BuildAm241Radionuclide();
  }

  if (canonical_name == "Tc-99m") {
    return BuildTc99mRadionuclide();
  }

  return std::nullopt;
}

} // namespace ggems::core::radioactivity::builtins
