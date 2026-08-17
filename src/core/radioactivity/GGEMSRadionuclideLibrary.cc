#include <format>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideLibrary.hh"

namespace ggems::core::radioactivity {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSRadionuclideLibrary::Add(GGEMSRadionuclideDefinition definition)
    -> DefinitionPointer {
  for (DefinitionPointer const &registered : definitions_) {
    if (registered->GetCanonicalName() == definition.GetCanonicalName()) {
      throw ggems::core::GGEMSRecoverable(
          std::format("Radionuclide '{}' is already registered.",
                      definition.GetCanonicalName()));
    }
  }

  DefinitionPointer definition_pointer =
      std::make_shared<GGEMSRadionuclideDefinition const>(
          std::move(definition));

  definitions_.push_back(definition_pointer);

  return definition_pointer;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSRadionuclideLibrary::Find(std::string_view name) const
    -> DefinitionPointer {
  for (DefinitionPointer const &definition : definitions_) {
    if (definition->GetCanonicalName() == name) {
      return definition;
    }
  }

  return {};
}

} // namespace ggems::core::radioactivity
