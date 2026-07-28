#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideLibrary.hh"

namespace ggems::core::radioactivity {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSRadionuclideLibrary::Add(GGEMSRadionuclideDefinition definition)
    -> DefinitionPointer {
  auto const definition_keys = definition.GetLookupKeys();
  std::vector<std::string> keys{definition_keys.begin(), definition_keys.end()};

  for (std::string const &key : keys) {
    GGEMS_CHECK_RECOVERABLE(
        !lookup_.contains(key),
        std::format("Radionuclide lookup name '{}' is already registered.",
                    key));
  }

  DefinitionPointer definition_pointer =
      std::make_shared<GGEMSRadionuclideDefinition const>(
          std::move(definition));

  std::vector<DefinitionPointer> updated_definitions = definitions_;
  std::unordered_map<std::string, DefinitionPointer> updated_lookup = lookup_;

  updated_definitions.push_back(definition_pointer);

  for (std::string &key : keys) {
    auto const [iterator, inserted] =
        updated_lookup.emplace(std::move(key), definition_pointer);
    static_cast<void>(iterator);
    GGEMS_CHECK_INTERNAL(inserted,
                         "Validated radionuclide lookup insertion failed.");
  }

  static_assert(noexcept(definitions_.swap(updated_definitions)));
  static_assert(noexcept(lookup_.swap(updated_lookup)));

  definitions_.swap(updated_definitions);
  lookup_.swap(updated_lookup);

  return definition_pointer;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSRadionuclideLibrary::Find(std::string_view name) const
    -> DefinitionPointer {
  auto const iterator =
      lookup_.find(GGEMSRadionuclideDefinition::NormalizeLookupName(name));

  if (iterator == lookup_.end()) {
    return {};
  }

  return iterator->second;
}

} // namespace ggems::core::radioactivity
