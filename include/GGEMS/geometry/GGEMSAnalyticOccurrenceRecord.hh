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
 * \brief Defines flat references for physically placed analytical
 * occurrences.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::geometry {

/*!
 * \brief Stores 20-byte geometry references and physical hierarchy links.
 *
 * Multi-occurrence auxiliary buffers begin with occurrence_count descriptors.
 * The offset is measured in bytes from that buffer's beginning. Sphere has no
 * separate pose and uses offset zero. Record indices preserve insertion order;
 * physical VolumeID is that index plus one, after World identity zero.
 * Parent and child/sibling links use physical VolumeIDs. Zero denotes World
 * for the parent and no occurrence for child/sibling links; the first World
 * child is always VolumeID one. Parents precede children in insertion order.
 */
struct GGEMSAnalyticOccurrenceRecord {
  /*! \brief Index in the contiguous evaluated analytical record array. */
  std::uint32_t record_index{0};

  /*! \brief Byte offset of the occurrence pose within the auxiliary buffer. */
  std::uint32_t auxiliary_offset{0};

  /*! \brief Parent physical VolumeID; zero selects the World. */
  std::uint32_t parent_volume_id{0};

  /*! \brief First direct child VolumeID; zero means no child. */
  std::uint32_t first_child_id{0};

  /*! \brief Next direct sibling VolumeID; zero ends the sibling list. */
  std::uint32_t next_sibling_id{0};
};

} // namespace ggems::geometry
