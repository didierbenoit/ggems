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
 * \brief Declares authored isotope fractions for one chemical element.
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

namespace ggems::core::materials {

/*! \brief Selects the meaning of fractions within one element. */
enum class GGEMSFractionBasis : std::uint8_t {
  /*! \brief Number share. */
  AtomFraction,

  /*! \brief Mass share. */
  MassFraction,
};

/*!
 * \brief Associates an isotope identity with an authored dimensionless
 * fraction.
 */
struct GGEMSIsotopeFraction {
  /*! \brief Exact isotope identity. */
  GGEMSIsotope isotope;

  /*! \brief Share on the composition's declared basis. */
  long double fraction;

  /*!
   * \brief Compares isotope identity and fraction by exact field equality.
   *
   * \return True when both the isotope key and stored fraction compare equal.
   */
  [[nodiscard]] auto operator==(GGEMSIsotopeFraction const &) const
    -> bool = default;
};

/*!
 * \brief Owns canonical authored isotope fractions for one chemical element.
 *
 * The declared atom or mass basis is retained; this is not yet a resolved
 * number-density table. Different bases are not converted by equality.
 */
class GGEMSIsotopicComposition {
public:
  /*!
   * \brief Canonicalizes isotope fractions while retaining their authoring
   * basis.
   *
   * Entries are sorted by key. Duplicate keys are rejected before exact-zero
   * entries are removed. The remaining sum must differ from one by no more than
   * 1.0e-5, then every retained fraction is divided by that sum. Individual
   * signs and finiteness are not checked here.
   *
   * \pre Supply finite nonnegative fractions and a declared basis enumerator.
   *
   * \param[in] basis AtomFraction or MassFraction.
   * \param[in] fractions One element's dimensionless isotope shares; ownership
   * is taken.
   * \throws GGEMSRecoverable If entries span multiple Z values, contain
   * duplicate keys, or fail the sum check.
   */
  GGEMSIsotopicComposition(GGEMSFractionBasis basis,
                           std::vector<GGEMSIsotopeFraction> fractions);

  /*!
   * \brief Returns the common proton number of the retained isotopes.
   *
   * \return Z from the first canonical isotope entry.
   */
  [[nodiscard]] auto GetAtomicNumber() const noexcept -> std::uint32_t {
    return fractions_.front().isotope.GetAtomicNumber();
  }

  /*!
   * \brief Returns the retained fraction basis.
   *
   * \return The basis supplied at construction.
   */
  [[nodiscard]] auto GetBasis() const noexcept -> GGEMSFractionBasis {
    return basis_;
  }

  /*!
   * \brief Returns retained isotope fractions in increasing (Z, A, M) order.
   *
   * The view borrows this object's storage; destruction, assignment, or moving
   * from the owner can invalidate it.
   *
   * \return A read-only span of the normalized authored fractions.
   */
  [[nodiscard]] auto GetFractions() const noexcept
    -> std::span<GGEMSIsotopeFraction const> {
    return fractions_;
  }

  /*!
   * \brief Compares the basis and canonical fraction sequence exactly.
   *
   * \return True when the basis and every isotope/fraction pair compare equal.
   */
  [[nodiscard]] auto operator==(GGEMSIsotopicComposition const &) const
    -> bool = default;

private:
  /*! \brief Retained atom or mass authoring basis. */
  GGEMSFractionBasis basis_;

  /*! \brief Fractions by isotope key. */
  std::vector<GGEMSIsotopeFraction> fractions_;
};

/*!
 * \brief Builds the compiled default atom-fraction composition for an element.
 *
 * Uses frozen NIST 4.1 representative natural fractions where listed, including
 * Ta-180m. Remaining supported elements use explicit single reference isotopes;
 * a reference isotope is not a claim of natural abundance. The current defaults
 * cover Z=1 through Z=99.
 *
 * \param[in] atomic_number Proton number of the requested element.
 * \return An independently owned composition on the atom-fraction basis.
 * \throws GGEMSRecoverable If neither a natural profile nor a reference isotope
 * is available.
 */
[[nodiscard]] auto BuildDefaultIsotopicComposition(std::uint32_t atomic_number)
  -> GGEMSIsotopicComposition;

} // namespace ggems::core::materials
