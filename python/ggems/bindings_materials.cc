#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/materials/GGEMSMaterialDescription.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace py = pybind11;

// =============================================================================
// =============================================================================

auto BindMaterials(py::module_ &module) -> void {
  namespace materials = ggems::core::materials;
  namespace builtins = ggems::core::materials::builtins;

  module.def("available", []() -> py::tuple {
    auto const names = builtins::GetAvailableMaterialNames();

    py::tuple result{names.size()};

    for (std::size_t index = 0U; index < names.size(); ++index) {
      result[index] = py::str{names[index]};
    }

    return result;
  });

  module.def("registered", []() -> py::tuple {
    auto const registered =
        materials::GGEMSMaterialManager::GetInstance().GetMaterials();

    py::tuple result{registered.size()};

    for (std::size_t index = 0U; index < registered.size(); ++index) {
      result[index] = py::str{registered[index].GetName()};
    }

    return result;
  });

  module.def(
      "add",
      [](std::string name, double density,
         std::map<std::string, double> const &elements,
         std::string const &density_unit) -> std::uint32_t {
        auto const material_density =
            ggems::python::detail::MakeQuantityOrThrow<ggems::units::Density>(
                density, density_unit,
                {.quantity_name = "Material density",
                 .unsupported_unit_subject = "GGEMS density"});

        std::vector<materials::GGEMSMaterialComponent> composition;
        composition.reserve(elements.size());

        for (auto const &[symbol, mass_fraction] : elements) {
          auto const &element = materials::RequireElementBySymbol(symbol);

          composition.push_back(
              {.atomic_number = element.GetAtomicNumber(),
               .mass_fraction = static_cast<long double>(mass_fraction)});
        }

        return materials::GGEMSMaterialManager::GetInstance().AddCustomMaterial(
            materials::GGEMSMaterial{std::move(name), material_density,
                                     std::move(composition)});
      },
      py::arg("name"), py::arg("density"), py::arg("elements"),
      py::arg("density_unit") = "g/cm3");

  module.def(
      "verbose",
      [](std::string const &name) -> void {
        auto const &manager = materials::GGEMSMaterialManager::GetInstance();

        if (auto const *material = manager.Find(name); material != nullptr) {
          materials::VerboseMaterial(*material);
          return;
        }

        materials::VerboseMaterial(builtins::BuildBuiltInMaterial(name));
      },
      py::arg("name"));

  module.def(
      "verbose",
      [](std::uint32_t material_index) -> void {
        auto const &material =
            materials::GGEMSMaterialManager::GetInstance().Require(
                material_index);

        materials::VerboseMaterial(material);
      },
      py::arg("material_id"));

  module.def(
      "describe",
      [](std::string const &name) -> std::string {
        auto const &material =
            materials::GGEMSMaterialManager::GetInstance().Require(name);

        return materials::DescribeMaterial(material);
      },
      py::arg("name"));

  module.def(
      "describe",
      [](std::uint32_t material_index) -> std::string {
        auto const &material =
            materials::GGEMSMaterialManager::GetInstance().Require(
                material_index);

        return materials::DescribeMaterial(material);
      },
      py::arg("material_id"));

  module.def(
      "register",
      [](std::string const &name) -> std::uint32_t {
        return materials::GGEMSMaterialManager::GetInstance().GetOrAddBuiltIn(
            name);
      },
      py::arg("name"));
}
