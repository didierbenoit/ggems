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
 * \brief Builds the compiled P-32 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildP32Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled P-33 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildP33Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Co-57 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCo57Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ga-67 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildGa67Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Cu-67 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCu67Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Sc-44 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildSc44Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Sc-47 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildSc47Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Mn-52 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildMn52Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Br-76 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildBr76Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Na-24 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildNa24Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled S-35 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildS35Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ca-45 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCa45Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Hg-203 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildHg203Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Tl-201 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildTl201Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Xe-133 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildXe133Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled N-13 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildN13Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Rb-82 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildRb82Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled In-111 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildIn111Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Cu-64 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCu64Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Zr-89 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildZr89Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ra-223 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildRa223Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Sr-89 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildSr89Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Sm-153 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildSm153Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Re-186 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildRe186Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Re-188 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildRe188Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ho-166 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildHo166Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ac-225 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildAc225Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled At-211 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildAt211Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Pb-212 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildPb212Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Bi-212 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildBi212Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Bi-213 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildBi213Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Pd-103 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildPd103Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Cs-131 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCs131Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ir-192 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildIr192Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Cs-137 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCs137Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ru-106 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildRu106Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Mo-99 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildMo99Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Sr-82 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildSr82Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ge-68 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildGe68Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Na-22 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildNa22Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ba-133 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildBa133Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Eu-152 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildEu152Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Mn-54 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildMn54Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Zn-65 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildZn65Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Cd-109 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCd109Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Ce-139 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCe139Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Sn-113 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildSn113Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Sr-85 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildSr85Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Y-88 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildY88Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Y-90 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildY90Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Cr-51 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCr51Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Cs-134 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildCs134Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Gd-153 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildGd153Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Kr-81m source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildKr81mRadionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Rb-81 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildRb81Radionuclide() -> GGEMSRadionuclideDefinition;

/*!
 * \brief Builds the compiled Tb-161 source-emission definition.
 *
 * \return Owned parent lifetime and independent marginal emission laws.
 */
[[nodiscard]] auto BuildTb161Radionuclide() -> GGEMSRadionuclideDefinition;

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
