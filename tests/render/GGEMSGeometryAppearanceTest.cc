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
 * \brief Checks independent display choices and hierarchical initial modes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <initializer_list>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"

namespace {
using namespace ggems::render;
using Mode = GGEMSGeometryDisplayMode;
} // namespace

TEST(GGEMSGeometryAppearance,
     DefaultsFollowPhysicalHierarchyAndWorldNeverSolid) {
  GGEMSGeometryOccurrence world{};
  GGEMSGeometryOccurrence mother{};
  mother.volume_id = 1U;
  mother.children = {2U};
  GGEMSGeometryOccurrence leaf{};
  leaf.volume_id = 2U;
  GGEMSGeometryAppearance appearance{};
  EXPECT_EQ(ResolveGeometryDisplayMode(world, appearance), Mode::Wireframe);
  EXPECT_EQ(ResolveGeometryDisplayMode(mother, appearance), Mode::Wireframe);
  EXPECT_EQ(ResolveGeometryDisplayMode(leaf, appearance), Mode::Solid);
  EXPECT_FALSE(appearance.mode_override);
  for (auto const mode : {Mode::None, Mode::Wireframe, Mode::Solid}) {
    appearance.mode_override = mode;
    EXPECT_EQ(ResolveGeometryDisplayMode(mother, appearance), mode);
    EXPECT_EQ(ResolveGeometryDisplayMode(leaf, appearance), mode);
    EXPECT_EQ(ResolveGeometryDisplayMode(world, appearance),
              mode == Mode::None ? Mode::None : Mode::Wireframe);
  }
}

TEST(GGEMSGeometryAppearance,
     HidingMotherDoesNotSuppressChildOrChangeOverrides) {
  GGEMSGeometryOccurrence mother{};
  mother.volume_id = 1U;
  mother.children = {2U};
  GGEMSGeometryOccurrence child{};
  child.volume_id = 2U;
  child.parent_volume_id = 1U;
  GGEMSGeometryAppearance mother_appearance{
    .mode_override = Mode::None,
  };
  GGEMSGeometryAppearance child_appearance{};
  EXPECT_EQ(ResolveGeometryDisplayMode(mother, mother_appearance), Mode::None);
  EXPECT_EQ(ResolveGeometryDisplayMode(child, child_appearance), Mode::Solid);
  EXPECT_FALSE(child_appearance.mode_override);
  child_appearance.mode_override = Mode::Wireframe;
  EXPECT_EQ(ResolveGeometryDisplayMode(child, child_appearance),
            Mode::Wireframe);
  EXPECT_EQ(mother_appearance.mode_override, Mode::None);
}
