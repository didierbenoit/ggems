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
 * \brief Defines exact material-isotope keys using Z, A, and isomer state.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <compare>
#include <cstdint>
/// \endcond

#include "GGEMS/GGEMSException.hh"

namespace ggems::core::materials {

/*!
 * \brief Identifies one material isotope by the exact tuple (Z, A, M).
 *
 * Z is the proton number, A the nucleon number, and M the isomer-state key. M=0
 * denotes the ground state. An identity does not establish mass-data
 * availability, radioactive source behavior, or a nuclear transport model.
 */
class GGEMSIsotope {
public:
  /*!
   * \brief Stores an isotope key after checking its atomic number.
   *
   * \param[in] atomic_number Proton number Z in [1, 99].
   * \param[in] mass_number Nucleon number A; stored without validation.
   * \param[in] isomer_state State key M; zero for ground state, positive for an
   * isomer.
   * \throws GGEMSRecoverable If atomic_number is outside [1, 99].
   */
  constexpr GGEMSIsotope(std::uint32_t atomic_number, std::uint32_t mass_number,
                         std::uint32_t isomer_state)
      : atomic_number_{atomic_number}, mass_number_{mass_number},
        isomer_state_{isomer_state} {
    if (atomic_number < 1U || atomic_number > 99U) {
      throw GGEMSRecoverable{"Isotope atomic number must be in [1, 99]."};
    }
  }

  /*!
   * \brief Returns the isotope proton number.
   *
   * \return Z in [1, 99].
   */
  [[nodiscard]] constexpr auto GetAtomicNumber() const noexcept
    -> std::uint32_t {
    return atomic_number_;
  }

  /*!
   * \brief Returns the stored nucleon number.
   *
   * \return The mass number A.
   */
  [[nodiscard]] constexpr auto GetMassNumber() const noexcept -> std::uint32_t {
    return mass_number_;
  }

  /*!
   * \brief Returns the stored isomer-state key.
   *
   * \return M, with zero denoting the ground state.
   */
  [[nodiscard]] constexpr auto GetIsomerState() const noexcept
    -> std::uint32_t {
    return isomer_state_;
  }

  /*!
   * \brief Orders isotope keys lexicographically by Z, then A, then M.
   *
   * \return Less, equal, or greater according to the three integer fields.
   */
  [[nodiscard]] constexpr auto operator<=>(GGEMSIsotope const &) const noexcept
    -> std::strong_ordering = default;

private:
  /*! \brief Proton number Z. */
  std::uint32_t atomic_number_;

  /*! \brief Nucleon number A. */
  std::uint32_t mass_number_;

  /*! \brief State key M; zero denotes the ground state. */
  std::uint32_t isomer_state_;
};

} // namespace ggems::core::materials
