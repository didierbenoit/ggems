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
 * \brief Creates small external MHD/RAW inputs for voxel host and device tests.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <random>
#include <span>
#include <string_view>
#include <system_error>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"

namespace ggems::test {

/*! \brief Owns tiny import files, removed when the fixture leaves scope. */
class GGEMSVoxelizedTestFixture {
public:
  /*! \brief Anatomical labels in a deliberately asymmetric X-fastest grid. */
  static constexpr std::array<std::uint16_t, 8> k_labels{
    10U, 20U, 30U, 10U, 30U, 20U, 20U, 10U,
  };

  /*!
   * \brief Writes an external little-endian ushort grid and its material map.
   * \param[in] dimensions Local X, Y, Z voxel counts.
   * \param[in] spacing MHD millimeter spacing triplet.
   * \param[in] labels Anatomical labels, with X varying fastest.
   * \param[in] mapping Singleton or half-open material mapping rows.
   */
  explicit GGEMSVoxelizedTestFixture(
    std::array<std::size_t, 3> dimensions = {2U, 2U, 2U},
    std::string_view spacing = "1 2 3",
    std::span<std::uint16_t const> labels = k_labels,
    std::string_view mapping = "10 10 Water\n20 20 Air\n30 30 Water\n")
      : root_(std::filesystem::path{::testing::TempDir()} /
              std::format(
                "ggems_voxel_{}_{}",
                std::chrono::steady_clock::now().time_since_epoch().count(),
                std::random_device{}())) {
    std::filesystem::create_directory(root_);
    std::ofstream metadata;
    metadata.exceptions(std::ios::failbit | std::ios::badbit);
    metadata.open(root_ / "grid.mhd", std::ios::binary);
    metadata
      << "ObjectType = Image\nNDims = 3\nBinaryData = True\n"
         "BinaryDataByteOrderMSB = False\nCompressedData = False\n"
      << std::format("DimSize = {} {} {}\n", dimensions[0], dimensions[1],
                     dimensions[2])
      << "ElementSpacing = " << spacing
      << "\nElementType = MET_USHORT\nElementDataFile = voxel labels.raw\n";
    metadata.close();
    std::ofstream raw;
    raw.exceptions(std::ios::failbit | std::ios::badbit);
    raw.open(GetRawPath(), std::ios::binary);
    for (auto const label : labels) {
      raw.put(static_cast<char>(label & 0xFFU));
      raw.put(static_cast<char>(label >> 8U));
    }
    raw.close();
    std::ofstream materials;
    materials.exceptions(std::ios::failbit | std::ios::badbit);
    materials.open(root_ / "materials.txt", std::ios::binary);
    materials << mapping;
    materials.close();
  }

  /*! \brief Removes only this fixture's privately created directory. */
  ~GGEMSVoxelizedTestFixture() {
    std::error_code ignored;
    std::filesystem::remove_all(root_, ignored);
  }

  GGEMSVoxelizedTestFixture(GGEMSVoxelizedTestFixture const &) = delete;
  auto operator=(GGEMSVoxelizedTestFixture const &)
    -> GGEMSVoxelizedTestFixture & = delete;

  /*! \brief Imports the generated files. \return Independently posed grid. */
  [[nodiscard]] auto Import() const -> geometry::GGEMSVoxelizedVolume {
    return {root_ / "grid.mhd", root_ / "materials.txt"};
  }

  /*! \brief Returns the external RAW path. \return This fixture's RAW file. */
  [[nodiscard]] auto GetRawPath() const -> std::filesystem::path {
    return root_ / "voxel labels.raw";
  }

private:
  /*! \brief Private temporary directory below the configured test temp root. */
  std::filesystem::path root_;
};

} // namespace ggems::test
