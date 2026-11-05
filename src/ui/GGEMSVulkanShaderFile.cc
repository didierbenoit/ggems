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
 * \brief Loads shader binaries for Vulkan shader module creation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <cstdint>

#include <format>
#include <fstream>
#include <ios>
#include <vector>
#include <filesystem>

#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"
#include "GGEMS/GGEMSException.hh"

namespace ggems::ui::detail {

auto ReadSPIRVFile(std::filesystem::path const &path)
  -> std::vector<std::uint32_t> {
  std::ifstream file{path, std::ios::binary | std::ios::ate};

  if (!file.is_open()) {
    throw ggems::core::GGEMSInternal(
      std::format("Unable to open SPIR-V shader file '{}'.", path.string()));
  }

  std::streamsize file_size = file.tellg();

  if (!(file_size > 0)) {
    throw ggems::core::GGEMSInternal(
      std::format("SPIR-V shader file '{}' is empty.", path.string()));
  }

  if (!(file_size % static_cast<std::streamsize>(sizeof(std::uint32_t)) == 0)) {
    throw ggems::core::GGEMSInternal(std::format(
      "SPIR-V shader file '{}' has an invalid byte size.", path.string()));
  }

  file.seekg(0, std::ios::beg);

  std::size_t word_count =
    static_cast<std::size_t>(file_size) / sizeof(std::uint32_t);

  std::vector<std::uint32_t> code(word_count);

  file.read(reinterpret_cast<char *>(code.data()), file_size);

  if (!file.good()) {
    throw ggems::core::GGEMSInternal(
      std::format("Unable to read SPIR-V shader file '{}'.", path.string()));
  }

  return code;
}

} // namespace ggems::ui::detail
