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
 * \brief Defines Python bindings for GGEMS materials.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/materials/GGEMSMaterialDescription.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/processes/GGEMSProductionCutDescription.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace py = pybind11;

namespace {

namespace materials = ggems::core::materials;
namespace builtins = ggems::core::materials::builtins;
namespace processes = ggems::core::processes;

using ggems::units::operator""_mm;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeIsotopicComposition(std::uint32_t atomic_number,
                                           py::dict const &definition)
  -> materials::GGEMSIsotopicComposition {
  if (!definition.contains("isotopes")) {
    return materials::BuildDefaultIsotopicComposition(atomic_number);
  }

  std::vector<materials::GGEMSIsotopeFraction> fractions;

  for (auto const item : definition["isotopes"].cast<py::iterable>()) {
    auto const isotope = py::cast<py::dict>(item);

    fractions.push_back({
      .isotope =
        materials::GGEMSIsotope{
          atomic_number,
          isotope["mass_number"].cast<std::uint32_t>(),
          isotope.contains("isomer_state")
            ? isotope["isomer_state"].cast<std::uint32_t>()
            : 0U,
        },
      .fraction = isotope["fraction"].cast<long double>(),
    });
  }

  return materials::GGEMSIsotopicComposition{
    materials::GGEMSFractionBasis::AtomFraction, std::move(fractions)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeElementalShare(std::string const &symbol,
                                      py::handle definition)
  -> materials::GGEMSElementalShare {
  auto const &element = materials::RequireElementBySymbol(symbol);
  auto const atomic_number = element.GetAtomicNumber();

  if (!py::isinstance<py::dict>(definition)) {
    return {
      .mass_fraction = py::cast<long double>(definition),
      .isotopic_composition =
        materials::BuildDefaultIsotopicComposition(atomic_number),
    };
  }

  auto const expanded = py::reinterpret_borrow<py::dict>(definition);

  return {
    .mass_fraction = expanded["mass_fraction"].cast<long double>(),
    .isotopic_composition = MakeIsotopicComposition(atomic_number, expanded),
  };
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(materials::GGEMSMaterial const &material,
                     materials::GGEMSMaterialInspection const &inspection)
  -> void {
  auto description = materials::DescribeMaterial(inspection);
  description += '\n';
  description += processes::DescribeProductionCutsForMaterial(material, 1_mm);

  GGEMS_INFO("Material", "\n{}\n", description);
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(materials::GGEMSMaterialManager const &manager,
                     std::string_view name) -> void {
  if (auto const index = manager.FindIndex(name); index.has_value()) {
    VerboseMaterial(manager.Require(*index),
                    materials::InspectMaterial(manager, *index));
    return;
  }

  if (auto const *material = manager.FindCustom(name); material != nullptr) {
    auto inspection = materials::InspectMaterial(*material);
    inspection.registration =
      materials::GGEMSMaterialRegistration::Unregistered;
    VerboseMaterial(*material, inspection);
    return;
  }

  auto material = builtins::BuildBuiltInMaterial(name);
  auto inspection = materials::InspectMaterial(material);
  inspection.registration = materials::GGEMSMaterialRegistration::Unregistered;
  VerboseMaterial(material, inspection);
}

} // namespace

// =============================================================================
// =============================================================================

/*!
 * \brief Registers the GGEMS material Python interface.
 *
 * \param module Python materials submodule.
 */
auto BindMaterials(py::module_ &module) -> void {
  // === === ===
  module.def(
    "available",
    [] -> void {
      auto const &manager = materials::GGEMSMaterialManager::GetInstance();

      for (auto const name : builtins::GetAvailableMaterialNames()) {
        VerboseMaterial(manager, name);
      }

      for (auto const &material : manager.GetCustomMaterials()) {
        VerboseMaterial(manager, material.GetName());
      }
    },
    R"doc(
Print all available GGEMS materials.

The output includes all built-in materials and all custom materials that have
been added by the user.

Each material report includes reference Production-Cut thresholds evaluated at
a 1 mm Cut length.

Available materials are not necessarily registered. A material becomes
registered when it is actually used by GGEMS.
)doc");

  // === === ===
  module.def(
    "registered",
    [] -> void {
      auto const &manager = materials::GGEMSMaterialManager::GetInstance();
      auto const registered = manager.GetMaterials();

      for (std::size_t index = 0U; index < registered.size(); ++index) {
        VerboseMaterial(registered[index],
                        materials::InspectMaterial(
                          manager, static_cast<std::uint32_t>(index)));
      }
    },
    R"doc(
Print all materials currently registered by GGEMS.

Each material report includes reference Production-Cut thresholds evaluated at
a 1 mm Cut length.

A registered material is a material that is actually referenced by the current
GGEMS configuration. Materials that are merely available are not listed.
)doc");

  // === === ===
  module.def(
    "add",
    [](std::string name, double density,
       std::map<std::string, py::object> const &elements,
       std::string const &density_unit) -> void {
      auto const material_density =
        ggems::python::detail::MakeQuantityOrThrow<ggems::units::Density>(
          density, density_unit);

      std::vector<materials::GGEMSElementalShare> elemental_shares;
      elemental_shares.reserve(elements.size());

      for (auto const &[symbol, definition] : elements) {
        elemental_shares.push_back(MakeElementalShare(symbol, definition));
      }

      materials::GGEMSMaterialManager::GetInstance().AddCustomMaterial(
        materials::GGEMSMaterial::FromIsotopicComposition(
          std::move(name), material_density, std::move(elemental_shares)));
    },
    R"doc(
Add a custom material to the set of available GGEMS materials.

An element can be specified directly by its mass fraction. In that form, GGEMS
uses the element's default isotopic composition.

An element can also be expanded with an explicit mass fraction and an optional
isotope list. Isotope fractions are atom fractions. Each isotope is identified
numerically by its mass number and, when needed, its isomer state. No isotope
name parsing is performed.

Adding a material makes it available but does not register it. Registration
occurs when the material is actually used by GGEMS.

Args:
    name: Material name.
    density: Material density.
    elements: Elemental composition and optional isotopic compositions.
    density_unit: Unit used for the density value. Defaults to "g/cm3".

Examples:
    Natural isotopic compositions::

        ggems.materials.add(
            "MyWater",
            1.0,
            {"H": 0.111898, "O": 0.888102},
            "g/cm3",
        )

    Pure deuterium::

        ggems.materials.add(
            "Deuterium",
            0.000180,
            {
                "H": {
                    "mass_fraction": 1.0,
                    "isotopes": [
                        {
                            "mass_number": 2,
                            "fraction": 1.0,
                        }
                    ],
                }
            },
            "g/cm3",
        )
)doc",
    py::arg("name"), py::arg("density"), py::arg("elements"),
    py::arg("density_unit") = "g/cm3");

  // === === ===
  module.def(
    "verbose",
    [](std::string const &name) -> void {
      VerboseMaterial(materials::GGEMSMaterialManager::GetInstance(), name);
    },
    R"doc(
Print the detailed scientific description of a material.

The report includes the material density, elemental composition, isotopic
composition, elemental number densities, electron densities, total atom density,
total electron density, and reference Production-Cut thresholds evaluated at a
1 mm Cut length.

The material may be built-in, custom, or already registered.

Args:
    name: Name of the material to inspect.

Example:
    ggems.materials.verbose("Water")
)doc",
    py::arg("name"));
}
