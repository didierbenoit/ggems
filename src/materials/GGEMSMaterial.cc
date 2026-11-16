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
 * \brief Implements owned material preparation, exact vacuum, and scientific
 * identity comparison.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSElement.hh"
#include "GGEMS/materials/GGEMSIsotopeMassAuthority.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

GGEMSMaterial::GGEMSMaterial(std::string name, units::Density density)
    : name_{std::move(name)}, density_{density} {}

// =============================================================================
// =============================================================================

GGEMSMaterial::GGEMSMaterial(
  std::string name, units::Density density,
  std::vector<GGEMSMaterialComponent> const &composition)
    : GGEMSMaterial{std::move(name), density} {
  if (density_.value == 0.0L) {
    return;
  }

  std::vector<GGEMSElementalShare> elemental_shares;
  elemental_shares.reserve(composition.size());

  for (auto const &component : composition) {
    elemental_shares.push_back({
      .mass_fraction = component.mass_fraction,
      .isotopic_composition =
        BuildDefaultIsotopicComposition(component.atomic_number),
    });
  }

  Compile(std::move(elemental_shares));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSMaterial::FromIsotopicComposition(
  std::string name, units::Density density,
  std::vector<GGEMSElementalShare> elemental_shares) -> GGEMSMaterial {
  GGEMSMaterial material{std::move(name), density};
  material.Compile(std::move(elemental_shares));
  return material;
}

// =============================================================================
// =============================================================================

auto GGEMSMaterial::Compile(std::vector<GGEMSElementalShare> elemental_shares)
  -> void {
  composition_.emplace(density_, std::move(elemental_shares),
                       GetIsotopeMassAuthority());
}

// =============================================================================
// =============================================================================

auto GGEMSMaterial::SetElementColor(std::uint32_t atomic_number,
                                    render::ColorKey color) -> GGEMSMaterial & {
  static_cast<void>(RequireElementByAtomicNumber(atomic_number));

  auto const found = std::ranges::find(element_colors_, atomic_number,
                                       &ElementColor::atomic_number);
  if (found == element_colors_.end()) {
    element_colors_.push_back({.atomic_number = atomic_number, .color = color});
  } else {
    found->color = color;
  }

  return *this;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSMaterial::GetElementColor(std::uint32_t atomic_number) const
  -> render::ColorKey {
  auto const found = std::ranges::find(element_colors_, atomic_number,
                                       &ElementColor::atomic_number);

  return found != element_colors_.end()
           ? found->color
           : RequireElementByAtomicNumber(atomic_number).GetColor();
}

// =============================================================================
// =============================================================================

auto GGEMSMaterial::SetIsotopeColor(GGEMSIsotope const &isotope,
                                    render::ColorKey color) -> GGEMSMaterial & {
  auto const found =
    std::ranges::find(isotope_colors_, isotope, &IsotopeColor::isotope);

  if (found == isotope_colors_.end()) {
    isotope_colors_.push_back({.isotope = isotope, .color = color});
  } else {
    found->color = color;
  }

  return *this;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSMaterial::GetIsotopeColor(GGEMSIsotope const &isotope) const
  -> render::ColorKey {
  auto const found =
    std::ranges::find(isotope_colors_, isotope, &IsotopeColor::isotope);

  return found != isotope_colors_.end()
           ? found->color
           : GetElementColor(isotope.GetAtomicNumber());
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSMaterial::GetIsotopeConstituents() const noexcept
  -> std::span<GGEMSIsotopeConstituent const> {
  if (!composition_.has_value()) {
    return {};
  }

  return composition_->GetIsotopeConstituents();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSMaterial::GetElementalConstituents() const noexcept
  -> std::span<GGEMSDerivedElementalConstituent const> {
  if (!composition_.has_value()) {
    return {};
  }

  return composition_->GetElementalConstituents();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSMaterial::GetTotalAtomDensityPerCubicCentimeter() const noexcept
  -> long double {
  if (!composition_.has_value()) {
    return 0.0L;
  }

  return composition_->GetTotalAtomDensityPerCubicCentimeter();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSMaterial::GetElectronDensityPerCubicCentimeter() const noexcept
  -> long double {
  if (!composition_.has_value()) {
    return 0.0L;
  }

  return composition_->GetElectronDensityPerCubicCentimeter();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
HasSameScientificIdentity(GGEMSMaterial const &first,
                          GGEMSMaterial const &second) noexcept -> bool {
  return first.GetDensity().value == second.GetDensity().value &&
         std::ranges::equal(first.GetIsotopeConstituents(),
                            second.GetIsotopeConstituents());
}

} // namespace ggems::core::materials
