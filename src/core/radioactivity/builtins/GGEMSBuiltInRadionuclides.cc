#include <optional>
#include <string_view>

#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace ggems::core::radioactivity::builtins {

[[nodiscard]] auto BuildBuiltInRadionuclide(std::string_view canonical_name)
    -> std::optional<GGEMSRadionuclideDefinition> {
  if (canonical_name == "F-18") {
    return BuildF18Radionuclide();
  }
  if (canonical_name == "C-11") {
    return BuildC11Radionuclide();
  }
  if (canonical_name == "O-15") {
    return BuildO15Radionuclide();
  }
  return std::nullopt;
}

} // namespace ggems::core::radioactivity::builtins
