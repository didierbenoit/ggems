#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <utility>

#include <pybind11/pybind11.h>

#include "detail/GGEMSPythonRadionuclideDefinition.hh"

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace py = pybind11;

// =============================================================================
// =============================================================================

auto BindRadionuclide(py::module_ &module) -> void {
  using Definition = ggems::core::radioactivity::GGEMSRadionuclideDefinition;
  using Handle = ggems::python::detail::RadionuclideDefinitionHandle;

  py::class_<Handle>(module, "RadionuclideDefinition")
      .def_property_readonly("name",
                             [](Handle const &handle) -> std::string {
                               return std::string{
                                   handle.GetDefinition()->GetCanonicalName()};
                             })
      .def_property_readonly(
          "half_life_seconds",
          [](Handle const &handle) -> double {
            return static_cast<double>(
                handle.GetDefinition()->GetHalfLifeSeconds());
          })
      .def_property_readonly(
          "emission_count", [](Handle const &handle) -> std::size_t {
            return handle.GetDefinition()->GetEmissions().size();
          });

  module.def(
      "radionuclide",
      [](std::string const &canonical_name) -> Handle {
        auto definition =
            ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(
                canonical_name);
        if (!definition.has_value()) {
          throw py::value_error(std::format(
              "Unknown built-in radionuclide '{}'. Supported canonical names "
              "are F-18, C-11, O-15 and Lu-177.",
              canonical_name));
        }

        return Handle{
            std::make_shared<Definition const>(std::move(*definition))};
      },
      py::arg("name"));
}
