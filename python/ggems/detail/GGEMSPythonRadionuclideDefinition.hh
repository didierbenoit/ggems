#pragma once

#include <memory>
#include <utility>

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::python::detail {

class RadionuclideDefinitionHandle {
public:
  explicit RadionuclideDefinitionHandle(
      std::shared_ptr<core::radioactivity::GGEMSRadionuclideDefinition const>
          definition) noexcept
      : definition_{std::move(definition)} {}

  [[nodiscard]] auto GetDefinition() const noexcept -> std::shared_ptr<
      core::radioactivity::GGEMSRadionuclideDefinition const> {
    return definition_;
  }

private:
  std::shared_ptr<core::radioactivity::GGEMSRadionuclideDefinition const>
      definition_;
};

} // namespace ggems::python::detail
