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
 * \brief Defines flat references for physically placed analytic and voxelized
 * occurrences.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::geometry {

/*! \brief Selects one of the implemented physical geometry families. */
enum class GGEMSOccurrenceFamily : std::uint32_t {
  Analytic = 0U,  /*!< \brief Evaluated analytical solid. */
  Voxelized = 1U, /*!< \brief Regular material grid. */
};

/*!
 * \brief Stores 24-byte family references and physical hierarchy links.
 *
 * Multi-occurrence and voxelized scene auxiliary buffers begin with
 * occurrence_count descriptors. Physical VolumeID is the descriptor index
 * plus one; record_index is dense within the selected geometry family.
 * Analytical pose offsets are bytes from the auxiliary buffer's beginning.
 * Sphere and voxelized entries use offset zero; the voxel record owns its pose.
 * Links use global physical VolumeIDs: zero denotes World for the parent and
 * no occurrence for child/sibling links. The first World child is VolumeID one.
 * Parents precede children in insertion order.
 */
struct GGEMSAnalyticOccurrenceRecord {
  /*! \brief Index in the contiguous record array selected by family. */
  std::uint32_t record_index{0};

  /*! \brief Byte offset of the occurrence pose within the auxiliary buffer. */
  std::uint32_t auxiliary_offset{0};

  /*! \brief Parent physical VolumeID; zero selects the World. */
  std::uint32_t parent_volume_id{0};

  /*! \brief First direct child VolumeID; zero means no child. */
  std::uint32_t first_child_id{0};

  /*! \brief Next direct sibling VolumeID; zero ends the sibling list. */
  std::uint32_t next_sibling_id{0};

  /*! \brief Physical geometry family selected by record_index. */
  GGEMSOccurrenceFamily family{GGEMSOccurrenceFamily::Analytic};
};

} // namespace ggems::geometry
