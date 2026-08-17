#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::core::radioactivity {

class GGEMSRadionuclideLibrary {
public:
  using DefinitionPointer = std::shared_ptr<GGEMSRadionuclideDefinition const>;

  [[nodiscard]] auto Add(GGEMSRadionuclideDefinition definition)
      -> DefinitionPointer;

  [[nodiscard]] auto Find(std::string_view name) const -> DefinitionPointer;

  [[nodiscard]] auto GetDefinitions() const noexcept
      -> std::span<DefinitionPointer const> {
    return definitions_;
  }

  [[nodiscard]] auto GetCount() const noexcept -> std::size_t {
    return definitions_.size();
  }

private:
  std::vector<DefinitionPointer> definitions_;
};

} // namespace ggems::core::radioactivity
