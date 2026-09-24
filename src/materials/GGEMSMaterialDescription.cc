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
 * \brief Copies material inspection records and formats isotope-aware
 * composition reports.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <algorithm>
/// \endcond

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialDescription.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

/*!
 * \brief Selects the number-density label for the current logger encoding.
 *
 * \return A static ASCII 1/cm3 or Unicode cubic-centimeter unit label.
 */
[[nodiscard]] auto NumberDensityUnit() noexcept -> std::string_view {
  return ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
             ggems::core::Encoding::Ascii
           ? "1/cm3"
           : "1/cm³";
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats an isotope with its element symbol and explicit isomer suffix.
 *
 * Ground states use Symbol-A, M=1 uses Symbol-Am, and larger keys use
 * Symbol-AmM.
 *
 * \param[in] isotope Isotope identity to format.
 * \return An owned label for display, not a parsed identity or source
 * radionuclide.
 */
[[nodiscard]] auto IsotopeLabel(materials::GGEMSIsotope const &isotope)
  -> std::string {
  auto const symbol =
    materials::RequireElementByAtomicNumber(isotope.GetAtomicNumber())
      .GetSymbol();

  switch (isotope.GetIsomerState()) {
  case 0U:
    return std::format("{}-{}", symbol, isotope.GetMassNumber());
  case 1U:
    return std::format("{}-{}m", symbol, isotope.GetMassNumber());
  default:
    return std::format("{}-{}m{}", symbol, isotope.GetMassNumber(),
                       isotope.GetIsomerState());
  }
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats known registration context for a material inspection.
 *
 * \param[in] inspection Inspection record; Registered requires a present
 * manager_index.
 * \return Registered/index text, available/unregistered text, or an empty
 * string for Unknown.
 */
[[nodiscard]] auto
RegistrationLabel(materials::GGEMSMaterialInspection const &inspection)
  -> std::string {
  switch (inspection.registration) {
  case materials::GGEMSMaterialRegistration::Registered:
    return std::format("registered, manager material index {}",
                       *inspection.manager_index);
  case materials::GGEMSMaterialRegistration::Unregistered:
    return "available, not registered";
  case materials::GGEMSMaterialRegistration::Unknown:
    break;
  }
  return {};
}

} // namespace

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

[[nodiscard]] auto InspectMaterial(GGEMSMaterial const &material)
  -> GGEMSMaterialInspection {
  GGEMSMaterialInspection inspection{
    .name = std::string{material.GetName()},
    .density = material.GetDensity(),
    .registration = GGEMSMaterialRegistration::Unknown,
    .manager_index = std::nullopt,
    .elements = {},
    .isotopes = {},
    .total_atom_density_per_cubic_centimeter =
      material.GetTotalAtomDensityPerCubicCentimeter(),
    .electron_density_per_cubic_centimeter =
      material.GetElectronDensityPerCubicCentimeter(),
  };

  for (auto const &values : material.GetElementalConstituents()) {
    auto const &element = RequireElementByAtomicNumber(values.atomic_number);

    inspection.elements.push_back({
      .values = values,
      .symbol = std::string{element.GetSymbol()},
      .name = std::string{element.GetName()},
    });
  }

  auto const isotopes = material.GetIsotopeConstituents();
  inspection.isotopes.assign(isotopes.begin(), isotopes.end());

  return inspection;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::uint32_t manager_index)
  -> GGEMSMaterialInspection {
  auto inspection = InspectMaterial(manager.Require(manager_index));
  inspection.registration = GGEMSMaterialRegistration::Registered;
  inspection.manager_index = manager_index;
  return inspection;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::string_view name)
  -> GGEMSMaterialInspection {
  if (auto const manager_index = manager.FindIndex(name);
      manager_index.has_value()) {
    return InspectMaterial(manager, *manager_index);
  }

  auto const builtin_names = builtins::GetAvailableMaterialNames();

  if (std::ranges::find(builtin_names, name) != builtin_names.end()) {
    auto inspection = InspectMaterial(builtins::BuildBuiltInMaterial(name));
    inspection.registration = GGEMSMaterialRegistration::Unregistered;
    return inspection;
  }

  if (auto const *material = manager.FindCustom(name); material != nullptr) {
    auto inspection = InspectMaterial(*material);
    inspection.registration = GGEMSMaterialRegistration::Unregistered;
    return inspection;
  }

  return InspectMaterial(builtins::BuildBuiltInMaterial(name));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeMaterial(GGEMSMaterial const &material)
  -> std::string {
  return DescribeMaterial(InspectMaterial(material));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeMaterial(GGEMSMaterialInspection const &inspection)
  -> std::string {
  std::string description = std::format("Material: {}", inspection.name);

  if (auto const registration = RegistrationLabel(inspection);
      !registration.empty()) {
    description += std::format("\n  Registration     : {}", registration);
  }

  description += std::format("\n  Density          : {}",
                             units::HumanReadable(inspection.density));

  if (inspection.elements.empty()) {
    description += "\n  Elements         : none";
  } else {
    description +=
      std::format("\n  Elements         : {}", inspection.elements.size());
  }

  std::size_t isotope_index{0U};
  for (auto const &element : inspection.elements) {
    auto const &values = element.values;

    description += std::format(
      "\n    {} {} Z={}"
      "\n      derived mass fraction : {:.8g}"
      "\n      number density        : {:.8g} {}"
      "\n      electron density      : {:.8g} {}",
      element.symbol, element.name, values.atomic_number, values.mass_fraction,
      values.number_density_per_cubic_centimeter, NumberDensityUnit(),
      values.electron_density_per_cubic_centimeter, NumberDensityUnit());

    for (; isotope_index < inspection.isotopes.size() &&
           inspection.isotopes[isotope_index].isotope.GetAtomicNumber() ==
             values.atomic_number;
         ++isotope_index) {
      auto const &isotope = inspection.isotopes[isotope_index];

      description += std::format(
        "\n      {:<8} Z={} A={} M={} atom fraction in element {:.8g}, "
        "number density {:.8g} {}",
        IsotopeLabel(isotope.isotope), isotope.isotope.GetAtomicNumber(),
        isotope.isotope.GetMassNumber(), isotope.isotope.GetIsomerState(),
        isotope.atom_fraction_in_element,
        isotope.number_density_per_cubic_centimeter, NumberDensityUnit());
    }
  }

  description += std::format(
    "\n  Atom density     : {:.8g} {}"
    "\n  Electron density : {:.8g} {}",
    inspection.total_atom_density_per_cubic_centimeter, NumberDensityUnit(),
    inspection.electron_density_per_cubic_centimeter, NumberDensityUnit());

  return description;
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(GGEMSMaterial const &material) -> void {
  GGEMS_INFO("Material", "\n{}\n", DescribeMaterial(material));
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(GGEMSMaterialInspection const &inspection) -> void {
  GGEMS_INFO("Material", "\n{}\n", DescribeMaterial(inspection));
}

} // namespace ggems::core::materials
