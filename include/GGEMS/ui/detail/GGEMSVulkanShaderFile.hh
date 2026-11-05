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
 * \brief Declares binary loading for Vulkan shader modules.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace ggems::ui::detail {

/*!
 * \brief Reads a SPIR-V shader file into 32-bit words.
 *
 * Checks byte count and read status without validating SPIR-V contents or
 *   converting byte order.
 *
 * \param[in] path Shader binary to read.
 * \return Owned words containing the file bytes in their original order.
 * \throws ggems::core::GGEMSInternal If opening or reading fails, the reported
 *   byte count is nonpositive, or it is not divisible by sizeof(std::uint32_t).
 */
[[nodiscard]] auto ReadSPIRVFile(std::filesystem::path const &path)
  -> std::vector<std::uint32_t>;

} // namespace ggems::ui::detail
