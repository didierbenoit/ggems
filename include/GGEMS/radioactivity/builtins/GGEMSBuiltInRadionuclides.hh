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
 * \brief Exposes the compiled radioactive source catalog.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <optional>
#include <string_view>
#include <span>
#include <string>

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

/*!
 * \namespace ggems::core::radioactivity::builtins
 * \brief Supplies the compiled source-radionuclide catalog and descriptions.
 *
 * Built-ins contain independent marginal spectra and yields. Positron groups do
 * not add annihilation photons; such photons require transport physics.
 */
namespace ggems::core::radioactivity::builtins {

/*!
 * \brief Builds the compiled H-3 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildH3Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled C-14 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildC14Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled F-18 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildF18Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled C-11 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildC11Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled O-15 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildO15Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ga-68 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildGa68Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Co-60 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCo60Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Lu-177 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildLu177Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled I-123 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildI123Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled I-124 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildI124Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled I-125 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildI125Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled I-131 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildI131Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Am-241 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildAm241Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Tc-99m source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildTc99mRadionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Borrows the catalog names in registration order.
 *
 * \return Span backed by static storage for the process lifetime.
 */
[[nodiscard]] auto GetAvailableRadionuclideNames() noexcept
  -> std::span<std::string_view const>;

/*!
 * \brief Builds a catalog entry by its exact canonical name.
 *
 * \param[in] canonical_name Case-sensitive catalog label, such as F-18.
 * \return Owned definition, or no value if the name is unknown.
 */
[[nodiscard]] auto BuildBuiltInRadionuclide(std::string_view canonical_name)
  -> std::optional<GGEMSRadionuclideDefinition>;

/*!
 * \brief Formats parent lifetime, emissions, spectra, and total yield.
 *
 * \param[in] definition Definition to describe; it need not be a catalog entry.
 * \return Owned multiline description.
 */
[[nodiscard]] auto
DescribeBuiltInRadionuclide(GGEMSRadionuclideDefinition const &definition)
  -> std::string;

/*!
 * \brief Formats a catalog definition selected by exact name.
 *
 * \param[in] canonical_name Case-sensitive catalog label.
 * \return Owned multiline description.
 * \throws GGEMSRecoverable If the name is unknown.
 */
[[nodiscard]] auto DescribeBuiltInRadionuclide(std::string_view canonical_name)
  -> std::string;

/*!
 * \brief Logs parent lifetime, emissions, spectra, and total yield.
 *
 * \param[in] definition Definition to describe; it need not be a catalog entry.
 */
auto VerboseBuiltInRadionuclide(GGEMSRadionuclideDefinition const &definition)
  -> void;

/*!
 * \brief Logs a catalog definition selected by exact name.
 *
 * \param[in] canonical_name Case-sensitive catalog label.
 * \throws GGEMSRecoverable If the name is unknown.
 */
auto VerboseBuiltInRadionuclide(std::string_view canonical_name) -> void;

} // namespace ggems::core::radioactivity::builtins
