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
 * \brief Defines the parent lifetime and emission groups of a radioactive
 * source.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"

/*!
 * \namespace ggems::core::radioactivity
 * \brief Defines radioactive source populations and emission-time laws.
 */
namespace ggems::core::radioactivity {

/*!
 * \brief Owns a parent half-life and its independent emission populations.
 *
 * This is a source definition, distinct from Material isotope composition. Each
 * emission carries its own particle kind, energy law, and particles-per-decay
 * yield. Yields need not sum to one. The current model samples marginal
 * emission populations; it does not preserve correlated products from a single
 * decay.
 */
class GGEMSRadionuclideDefinition {
public:
  /*!
   * \brief Takes ownership of a named parent and its emission definitions.
   * \param[in] canonical_name Nonempty canonical source label.
   * \param[in] half_life_seconds Positive finite parent half-life in seconds.
   * \param[in] emissions Nonempty list of marginal emissions.
   * \throws GGEMSRecoverable If the name or emission list is empty, or
   * half-life fails the positive comparison. Finiteness is a caller
   * precondition.
   */
  GGEMSRadionuclideDefinition(std::string canonical_name,
                              long double half_life_seconds,
                              std::vector<GGEMSRadionuclideEmission> emissions);

  /*!
   * \brief Borrows the canonical source-radionuclide name.
   *
   * \return View valid until this definition is assigned or destroyed.
   */
  [[nodiscard]] auto GetCanonicalName() const noexcept -> std::string_view {
    return canonical_name_;
  }

  /*!
   * \brief Reads the parent half-life.
   *
   * \return Half-life in seconds.
   */
  [[nodiscard]] auto GetHalfLifeSeconds() const noexcept -> long double {
    return half_life_seconds_;
  }

  /*!
   * \brief Borrows the ordered marginal emission definitions.
   *
   * \return Span valid until this definition is assigned or destroyed.
   */
  [[nodiscard]] auto GetEmissions() const noexcept
    -> std::span<GGEMSRadionuclideEmission const> {
    return emissions_;
  }

  /*!
   * \brief Sums the yields of all admitted emission groups.
   *
   * \return Expected emitted particles per parent decay, which may exceed one.
   */
  [[nodiscard]] auto GetTotalYieldPerDecay() const noexcept -> long double;

private:
  /*! \brief Owned canonical source-radionuclide label. */
  std::string canonical_name_;

  /*! \brief Parent half-life in seconds. */
  long double half_life_seconds_;

  /*! \brief Owned independent marginal emission definitions. */
  std::vector<GGEMSRadionuclideEmission> emissions_;
};
} // namespace ggems::core::radioactivity
