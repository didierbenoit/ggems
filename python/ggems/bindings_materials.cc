// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief XXX
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
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

  // === === ===
  py::enum_<materials::GGEMSIsotopeProfile>(module, "IsotopeProfile")
    .value("Nist41Natural", materials::GGEMSIsotopeProfile::Nist41Natural)
    .value("LegacyReferenceIsotope",
           materials::GGEMSIsotopeProfile::LegacyReferenceIsotope);

  // === === ===
  py::enum_<materials::GGEMSMaterialRegistration>(module,
                                                  "MaterialRegistration")
    .value("Unknown", materials::GGEMSMaterialRegistration::Unknown)
    .value("Unregistered", materials::GGEMSMaterialRegistration::Unregistered)
    .value("Registered", materials::GGEMSMaterialRegistration::Registered);

  // === === ===
  py::class_<materials::GGEMSIsotopeConstituent>(module, "IsotopeInspection")
    .def_property_readonly(
      "atomic_number",
      [](materials::GGEMSIsotopeConstituent const &isotope) -> std::uint32_t {
        return isotope.isotope.GetAtomicNumber();
      })

    .def_property_readonly(
      "mass_number",
      [](materials::GGEMSIsotopeConstituent const &isotope) -> std::uint32_t {
        return isotope.isotope.GetMassNumber();
      })

    .def_property_readonly(
      "isomer_state",
      [](materials::GGEMSIsotopeConstituent const &isotope) -> std::uint32_t {
        return isotope.isotope.GetIsomerState();
      })

    .def_property_readonly(
      "atom_fraction_in_element",
      [](materials::GGEMSIsotopeConstituent const &isotope) -> double {
        return static_cast<double>(isotope.atom_fraction_in_element);
      })

    .def_property_readonly(
      "number_density_per_cubic_centimeter",
      [](materials::GGEMSIsotopeConstituent const &isotope) -> double {
        return static_cast<double>(isotope.number_density_per_cubic_centimeter);
      });

  // === === ===
  py::class_<materials::GGEMSElementInspection>(module, "ElementInspection")
    .def_property_readonly(
      "atomic_number",
      [](materials::GGEMSElementInspection const &element) -> std::uint32_t {
        return element.values.atomic_number;
      })

    .def_readonly("symbol", &materials::GGEMSElementInspection::symbol)

    .def_readonly("name", &materials::GGEMSElementInspection::name)

    .def_property_readonly(
      "mass_fraction",
      [](materials::GGEMSElementInspection const &element) -> double {
        return static_cast<double>(element.values.mass_fraction);
      })

    .def_property_readonly(
      "number_density_per_cubic_centimeter",
      [](materials::GGEMSElementInspection const &element) -> double {
        return static_cast<double>(
          element.values.number_density_per_cubic_centimeter);
      })

    .def_property_readonly(
      "electron_density_per_cubic_centimeter",
      [](materials::GGEMSElementInspection const &element) -> double {
        return static_cast<double>(
          element.values.electron_density_per_cubic_centimeter);
      })

    .def_readonly("isotope_profile",
                  &materials::GGEMSElementInspection::isotope_profile);

  // === === ===
  py::class_<materials::GGEMSMaterialInspection>(module, "MaterialInspection")
    .def_readonly("name", &materials::GGEMSMaterialInspection::name)

    .def_property_readonly(
      "density_g_cm3",
      [](materials::GGEMSMaterialInspection const &inspection) -> double {
        return ggems::python::detail::ConvertQuantityToDoubleOrThrow(
          inspection.density, "g/cm3");
      })

    .def_readonly("registration",
                  &materials::GGEMSMaterialInspection::registration)

    .def_readonly("manager_index",
                  &materials::GGEMSMaterialInspection::manager_index)

    .def_readonly("elements", &materials::GGEMSMaterialInspection::elements)

    .def_readonly("isotopes", &materials::GGEMSMaterialInspection::isotopes)

    .def_property_readonly(
      "total_atom_density_per_cubic_centimeter",
      [](materials::GGEMSMaterialInspection const &inspection) -> double {
        return static_cast<double>(
          inspection.total_atom_density_per_cubic_centimeter);
      })

    .def_property_readonly(
      "electron_density_per_cubic_centimeter",
      [](materials::GGEMSMaterialInspection const &inspection) -> double {
        return static_cast<double>(
          inspection.electron_density_per_cubic_centimeter);
      })

    .def(
      "__str__",
      [](materials::GGEMSMaterialInspection const &inspection) -> std::string {
        return materials::DescribeMaterial(inspection);
      });

  // === === ===
  py::class_<materials::GGEMSEMMaterialPackage>(module, "EMMaterialPackage")
    .def_property_readonly("material_ids",
                           [](materials::GGEMSEMMaterialPackage const &package)
                             -> std::vector<std::uint32_t> {
                             auto const ids = package.GetMaterialIds();
                             return {ids.begin(), ids.end()};
                           })

    .def_property_readonly(
      "material_count",
      [](materials::GGEMSEMMaterialPackage const &package) -> std::size_t {
        return package.GetDescriptors().size();
      });

  // === === ===
  module.def("available", [] -> py::tuple {
    auto const names = builtins::GetAvailableMaterialNames();

    py::tuple result{names.size()};

    for (std::size_t index = 0U; index < names.size(); ++index) {
      result[index] = py::str{names[index]};
    }

    return result;
  });

  // === === ===
  module.def("registered", [] -> py::tuple {
    auto const registered =
      materials::GGEMSMaterialManager::GetInstance().GetMaterials();

    py::tuple result{registered.size()};

    for (std::size_t index = 0U; index < registered.size(); ++index) {
      result[index] = py::str{registered[index].GetName()};
    }

    return result;
  });

  // === === ===
  module.def(
    "add",
    [](std::string name, double density,
       std::map<std::string, double> const &elements,
       std::string const &density_unit) -> std::uint32_t {
      auto const material_density =
        ggems::python::detail::MakeQuantityOrThrow<ggems::units::Density>(
          density, density_unit);

      std::vector<materials::GGEMSMaterialComponent> composition;
      composition.reserve(elements.size());

      for (auto const &[symbol, mass_fraction] : elements) {
        auto const &element = materials::RequireElementBySymbol(symbol);

        composition.push_back({
          .atomic_number = element.GetAtomicNumber(),
          .mass_fraction = static_cast<long double>(mass_fraction),
        });
      }

      return materials::GGEMSMaterialManager::GetInstance().AddCustomMaterial(
        materials::GGEMSMaterial{std::move(name), material_density,
                                 composition});
    },
    py::arg("name"), py::arg("density"), py::arg("elements"),
    py::arg("density_unit") = "g/cm3");

  // === === ===
  for (auto const *keyword : {"material_index", "material_id"}) {
    module.def(
      "verbose",
      [](std::uint32_t material_index) -> void {
        materials::VerboseMaterial(materials::InspectMaterial(
          materials::GGEMSMaterialManager::GetInstance(), material_index));
      },
      py::arg(keyword));

    module.def(
      "describe",
      [](std::uint32_t material_index) -> std::string {
        return materials::DescribeMaterial(materials::InspectMaterial(
          materials::GGEMSMaterialManager::GetInstance(), material_index));
      },
      py::arg(keyword));
  }

  // === === ===
  module.def(
    "verbose",
    [](std::string const &name) -> void {
      materials::VerboseMaterial(materials::InspectMaterial(
        materials::GGEMSMaterialManager::GetInstance(), name));
    },
    py::arg("name"));

  // === === ===
  module.def(
    "describe",
    [](std::string const &name) -> std::string {
      return materials::DescribeMaterial(materials::InspectMaterial(
        materials::GGEMSMaterialManager::GetInstance(), name));
    },
    py::arg("name"));

  // === === ===
  module.def(
    "inspect",
    [](std::uint32_t material_index) -> materials::GGEMSMaterialInspection {
      return materials::InspectMaterial(
        materials::GGEMSMaterialManager::GetInstance(), material_index);
    },
    py::arg("material_index"));

  // === === ===
  module.def(
    "inspect",
    [](std::string const &name) -> materials::GGEMSMaterialInspection {
      return materials::InspectMaterial(
        materials::GGEMSMaterialManager::GetInstance(), name);
    },
    py::arg("name"));

  // === === ===
  module.def("compile_registered", [] -> materials::GGEMSEMMaterialPackage {
    return materials::GGEMSEMMaterialPackage{
      materials::GGEMSMaterialManager::GetInstance().GetMaterials()};
  });

  // === === ===
  module.def(
    "register",
    [](std::string const &name) -> std::uint32_t {
      return materials::GGEMSMaterialManager::GetInstance().GetOrAddBuiltIn(
        name);
    },
    py::arg("name"));
}
