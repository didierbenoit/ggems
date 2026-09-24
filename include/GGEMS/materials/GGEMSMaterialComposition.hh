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
 * \brief Declares isotope-resolved material composition and derived elemental
 * EM data.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <span>
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

/*!
 * \brief Authors one element's material mass share and its isotope mixture.
 */
struct GGEMSElementalShare {
  long double mass_fraction; /*!< Dimensionless share of total material mass. */
  GGEMSIsotopicComposition
    isotopic_composition; /*!< Owned isotope fractions for this element. */
};

/*!
 * \brief Stores a resolved isotope number density and its within-element atom
 * share.
 */
struct GGEMSIsotopeConstituent {
  GGEMSIsotope isotope;                 /*!< Exact (Z, A, M) identity. */
  long double atom_fraction_in_element; /*!< Atom share within this Z, not
                                           within all matter. */
  long double number_density_per_cubic_centimeter; /*!< Isotope atoms per cubic
                                                      centimeter. */

  /*!
   * \brief Compares all isotope-constituent fields exactly.
   *
   * \return True when identity, within-element atom fraction, and number
   * density match.
   */
  [[nodiscard]] auto operator==(GGEMSIsotopeConstituent const &) const
    -> bool = default;
};

/*!
 * \brief Aggregates resolved isotope data for elemental electromagnetic
 * preparation.
 */
struct GGEMSDerivedElementalConstituent {
  std::uint32_t atomic_number; /*!< Chemical identity Z. */
  long double
    mass_fraction; /*!< Mass share derived from isotope densities and masses. */
  long double number_density_per_cubic_centimeter;   /*!< Sum of isotope atom
                                                        densities, in 1/cm3. */
  long double electron_density_per_cubic_centimeter; /*!< Z times atom density,
                                                        in 1/cm3. */
};

/*!
 * \brief Owns authored shares, resolved isotope densities, and elemental EM
 * aggregates.
 *
 * Elements are ordered by Z, and isotope rows by (Z, A, M). With elemental mass
 * share w and density rho in g/cm3, atom-basis shares a_i give n_i =
 * rho*w*N_A*a_i / sum(a_j*M_j). Mass-basis shares f_i give n_i =
 * rho*w*N_A*f_i/M_i, where M_i is the resolved molar mass in g/mol. Element
 * mass shares are rederived from sum(n_i*M_i); electron densities assume Z
 * electrons per atom. No temperature or thermal-scattering state is stored.
 */
class GGEMSMaterialComposition {
public:
  /*!
   * \brief Resolves positive-density matter into isotope and elemental number
   * densities.
   *
   * Elemental shares are sorted by Z, checked for duplicate Z before zero
   * shares are removed, admitted within 1.0e-5 of unity, and normalized once.
   * The isotope mass table is read during construction and is not retained.
   *
   * \pre Provide finite nonnegative fractions and finite positive resolved
   * masses.
   *
   * \param[in] density Bulk mass density; canonical storage is pg/pm3.
   * \param[in] elemental_shares Owned elemental mass shares with their authored
   * isotope mixtures.
   * \param[in] resolved_isotopes Exact-key molar-mass authority used for every
   * retained isotope.
   * \throws GGEMSRecoverable If density in g/cm3 is not positive and normal, an
   * element or mass is absent, or elemental key/sum checks fail.
   */
  GGEMSMaterialComposition(units::Density density,
                           std::vector<GGEMSElementalShare> elemental_shares,
                           GGEMSResolvedIsotopeTable const &resolved_isotopes);

  /*!
   * \brief Returns the normalized authored elemental mass shares.
   *
   * The view borrows this object's storage; destruction, assignment, or moving
   * from the owner can invalidate it.
   *
   * \return A borrowed span sorted by Z, retaining each isotope authoring
   * basis.
   */
  [[nodiscard]] auto GetElementalShares() const noexcept
    -> std::span<GGEMSElementalShare const> {
    return elemental_shares_;
  }

  /*!
   * \brief Returns the resolved isotope composition.
   *
   * The view borrows this object's storage; destruction, assignment, or moving
   * from the owner can invalidate it.
   *
   * \return A borrowed span sorted by (Z, A, M), with number densities in
   * 1/cm3.
   */
  [[nodiscard]] auto GetIsotopeConstituents() const noexcept
    -> std::span<GGEMSIsotopeConstituent const> {
    return isotope_constituents_;
  }

  /*!
   * \brief Returns the isotope-derived elemental EM view.
   *
   * The view borrows this object's storage; destruction, assignment, or moving
   * from the owner can invalidate it.
   *
   * \return A borrowed span sorted by Z, with number and electron densities in
   * 1/cm3.
   */
  [[nodiscard]] auto GetElementalConstituents() const noexcept
    -> std::span<GGEMSDerivedElementalConstituent const> {
    return elemental_constituents_;
  }

  /*!
   * \brief Returns the summed isotope atom density.
   *
   * \return Total atoms per cubic centimeter.
   */
  [[nodiscard]] auto GetTotalAtomDensityPerCubicCentimeter() const noexcept
    -> long double {
    return total_atom_density_per_cubic_centimeter_;
  }

  /*!
   * \brief Returns the summed elemental electron density.
   *
   * \return Total electrons per cubic centimeter.
   */
  [[nodiscard]] auto GetElectronDensityPerCubicCentimeter() const noexcept
    -> long double {
    return electron_density_per_cubic_centimeter_;
  }

private:
  std::vector<GGEMSElementalShare>
    elemental_shares_; /*!< Owned normalized authoring data by Z. */
  std::vector<GGEMSIsotopeConstituent>
    isotope_constituents_; /*!< Densities by isotope key. */
  std::vector<GGEMSDerivedElementalConstituent>
    elemental_constituents_; /*!< EM rows by Z. */
  long double total_atom_density_per_cubic_centimeter_{
    0.0L}; /*!< Total atoms in 1/cm3. */
  long double electron_density_per_cubic_centimeter_{
    0.0L}; /*!< Total electrons in 1/cm3. */
};

} // namespace ggems::core::materials
