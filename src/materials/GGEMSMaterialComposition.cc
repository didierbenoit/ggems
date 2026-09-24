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
 * \brief Resolves authored material shares into isotope number densities and
 * elemental EM views.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>
/// \endcond

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"
#include "GGEMS/materials/detail/GGEMSAvogadroConstant.hh"
#include "GGEMS/materials/detail/GGEMSFractionCanonicalization.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::core::materials {

namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Computes isotope densities from an element's mass share and fraction
 * basis.
 *
 * For atom fractions a_i, divide the scaled elemental mass density by mean
 * molar mass sum(a_i*M_i), then multiply by each a_i. For mass fractions f_i,
 * divide each scaled contribution by its own molar mass M_i.
 *
 * \pre molar_masses must contain one entry per fraction; the composition basis
 * must be valid.
 *
 * \param[in] density_grams_per_cubic_centimeter Bulk material density in g/cm3.
 * \param[in] element_mass_fraction Dimensionless fraction of material mass
 * assigned to this Z.
 * \param[in] composition Canonical isotope shares for one element.
 * \param[in] molar_masses Finite positive g/mol values in the same order as the
 * fractions.
 * \return Isotope atom densities in 1/cm3, in fraction order.
 */
[[nodiscard]] auto
ComputeIsotopeNumberDensities(long double density_grams_per_cubic_centimeter,
                              long double element_mass_fraction,
                              GGEMSIsotopicComposition const &composition,
                              std::span<long double const> molar_masses)
  -> std::vector<long double> {
  auto const fractions = composition.GetFractions();
  std::vector<long double> number_densities(fractions.size(), 0.0L);

  long double const scaled_element_density =
    detail::k_avogadro_constant_per_mole * density_grams_per_cubic_centimeter *
    element_mass_fraction;

  switch (composition.GetBasis()) {
  case GGEMSFractionBasis::AtomFraction: {
    long double mean_molar_mass{0.0L};
    for (std::size_t index = 0U; index < fractions.size(); ++index) {
      mean_molar_mass += fractions[index].fraction * molar_masses[index];
    }

    long double const element_number_density =
      scaled_element_density / mean_molar_mass;

    for (std::size_t index = 0U; index < fractions.size(); ++index) {
      number_densities[index] =
        fractions[index].fraction * element_number_density;
    }
    break;
  }
  case GGEMSFractionBasis::MassFraction:
    for (std::size_t index = 0U; index < fractions.size(); ++index) {
      number_densities[index] = scaled_element_density *
                                fractions[index].fraction / molar_masses[index];
    }
    break;
  }

  return number_densities;
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSMaterialComposition::GGEMSMaterialComposition(
  units::Density density, std::vector<GGEMSElementalShare> elemental_shares,
  GGEMSResolvedIsotopeTable const &resolved_isotopes) {
  auto const density_grams_per_cubic_centimeter =
    units::ConvertTo(density, "g/cm3");

  if (!density_grams_per_cubic_centimeter.has_value() ||
      !std::isnormal(*density_grams_per_cubic_centimeter) ||
      !(*density_grams_per_cubic_centimeter > 0.0L)) {
    throw GGEMSRecoverable{"Material composition density must be a finite, "
                           "strictly positive normal value."};
  }

  for (auto const &share : elemental_shares) {
    static_cast<void>(RequireElementByAtomicNumber(
      share.isotopic_composition.GetAtomicNumber()));
  }

  detail::CanonicalizeFractions(
    elemental_shares,
    [](GGEMSElementalShare const &share) noexcept -> std::uint32_t {
      return share.isotopic_composition.GetAtomicNumber();
    },
    &GGEMSElementalShare::mass_fraction, "Elemental mass");

  std::vector<long double> element_molar_mass_densities;
  element_molar_mass_densities.reserve(elemental_shares.size());
  elemental_constituents_.reserve(elemental_shares.size());

  long double total_molar_mass_density{0.0L};

  for (auto const &share : elemental_shares) {
    auto const &composition = share.isotopic_composition;
    auto const fractions = composition.GetFractions();

    std::vector<long double> molar_masses;
    molar_masses.reserve(fractions.size());
    for (auto const &entry : fractions) {
      molar_masses.push_back(
        resolved_isotopes.Require(entry.isotope).molar_mass_grams_per_mole);
    }

    auto const number_densities = ComputeIsotopeNumberDensities(
      *density_grams_per_cubic_centimeter, share.mass_fraction, composition,
      molar_masses);

    long double element_number_density{0.0L};
    long double element_molar_mass_density{0.0L};
    for (std::size_t index = 0U; index < fractions.size(); ++index) {
      element_number_density += number_densities[index];
      element_molar_mass_density +=
        number_densities[index] * molar_masses[index];
    }

    for (std::size_t index = 0U; index < fractions.size(); ++index) {
      long double const atom_fraction =
        composition.GetBasis() == GGEMSFractionBasis::AtomFraction
          ? fractions[index].fraction
          : number_densities[index] / element_number_density;

      isotope_constituents_.push_back({
        .isotope = fractions[index].isotope,
        .atom_fraction_in_element = atom_fraction,
        .number_density_per_cubic_centimeter = number_densities[index],
      });
    }

    auto const atomic_number = composition.GetAtomicNumber();

    elemental_constituents_.push_back({
      .atomic_number = atomic_number,
      .mass_fraction = 0.0L,
      .number_density_per_cubic_centimeter = element_number_density,
      .electron_density_per_cubic_centimeter =
        element_number_density * static_cast<long double>(atomic_number),
    });
    element_molar_mass_densities.push_back(element_molar_mass_density);

    total_atom_density_per_cubic_centimeter_ += element_number_density;
    electron_density_per_cubic_centimeter_ +=
      elemental_constituents_.back().electron_density_per_cubic_centimeter;
    total_molar_mass_density += element_molar_mass_density;
  }

  for (std::size_t index = 0U; index < elemental_constituents_.size();
       ++index) {
    elemental_constituents_[index].mass_fraction =
      element_molar_mass_densities[index] / total_molar_mass_density;
  }

  elemental_shares_ = std::move(elemental_shares);
}

} // namespace ggems::core::materials
