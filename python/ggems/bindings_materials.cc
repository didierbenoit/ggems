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
  module.def("available", [] -> void {
    auto const &manager = materials::GGEMSMaterialManager::GetInstance();

    for (auto const name : builtins::GetAvailableMaterialNames()) {
      materials::VerboseMaterial(materials::InspectMaterial(manager, name));
    }

    for (auto const &material : manager.GetCustomMaterials()) {
      materials::VerboseMaterial(
        materials::InspectMaterial(manager, material.GetName()));
    }
  });

  // === === ===
  module.def("registered", [] -> void {
    auto const &manager = materials::GGEMSMaterialManager::GetInstance();
    auto const registered = manager.GetMaterials();

    for (std::size_t index = 0U; index < registered.size(); ++index) {
      materials::VerboseMaterial(
        materials::InspectMaterial(manager, static_cast<std::uint32_t>(index)));
    }
  });

  // === === ===
  module.def(
    "add",
    [](std::string name, double density,
       std::map<std::string, double> const &elements,
       std::string const &density_unit) -> void {
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

      materials::GGEMSMaterialManager::GetInstance().AddCustomMaterial(
        materials::GGEMSMaterial{std::move(name), material_density,
                                 composition});
    },
    py::arg("name"), py::arg("density"), py::arg("elements"),
    py::arg("density_unit") = "g/cm3");

  // === === ===
  module.def(
    "verbose",
    [](std::string const &name) -> void {
      materials::VerboseMaterial(materials::InspectMaterial(
        materials::GGEMSMaterialManager::GetInstance(), name));
    },
    py::arg("name"));
}
