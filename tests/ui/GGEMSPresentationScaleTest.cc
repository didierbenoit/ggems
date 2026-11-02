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
 * \brief Checks display-density conversion to render-target dimensions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <gtest/gtest.h>

#include "GGEMS/ui/detail/GGEMSPresentationScale.hh"

namespace {

using ggems::ui::detail::ComputeSceneTargetExtent;
using ggems::ui::detail::GGEMSPixelExtent;
using ggems::ui::detail::NormalizeContentScale;

// =============================================================================
// =============================================================================

// Display scale must preserve admitted fractional values and bound values
// outside the supported range.
TEST(GGEMSPresentationScale, ContentScaleIsClampedToTheAdmittedRange) {
  EXPECT_FLOAT_EQ(NormalizeContentScale(0.0F), 1.0F);
  EXPECT_FLOAT_EQ(NormalizeContentScale(0.75F), 1.0F);
  EXPECT_FLOAT_EQ(NormalizeContentScale(1.0F), 1.0F);
  EXPECT_FLOAT_EQ(NormalizeContentScale(1.5F), 1.5F);
  EXPECT_FLOAT_EQ(NormalizeContentScale(2.5F), 2.5F);
  EXPECT_FLOAT_EQ(NormalizeContentScale(3.0F), 2.5F);
}

// =============================================================================
// =============================================================================

// A unit-density display must not rescale logical viewport dimensions.
TEST(GGEMSPresentationScale, UnitDensityKeepsTheLogicalExtent) {
  for (GGEMSPixelExtent const logical : {
         GGEMSPixelExtent{.width = 1U, .height = 1U},
         GGEMSPixelExtent{.width = 1407U, .height = 905U},
         GGEMSPixelExtent{.width = 1920U, .height = 1080U},
       }) {
    EXPECT_EQ(ComputeSceneTargetExtent(logical, 1.0F, 1.0F), logical);
  }
}

// =============================================================================
// =============================================================================

// Asymmetric display densities must scale the matching axis with nearest-pixel
// rounding.
TEST(GGEMSPresentationScale, TargetExtentScalesEachAxisAndRoundsToNearest) {
  EXPECT_EQ(ComputeSceneTargetExtent(
              GGEMSPixelExtent{.width = 800U, .height = 600U}, 2.0F, 2.0F),
            (GGEMSPixelExtent{.width = 1600U, .height = 1200U}));

  EXPECT_EQ(ComputeSceneTargetExtent(
              GGEMSPixelExtent{.width = 101U, .height = 100U}, 1.5F, 1.5F),
            (GGEMSPixelExtent{.width = 152U, .height = 150U}));

  EXPECT_EQ(ComputeSceneTargetExtent(
              GGEMSPixelExtent{.width = 400U, .height = 300U}, 2.0F, 1.25F),
            (GGEMSPixelExtent{.width = 800U, .height = 375U}));
}

// =============================================================================
// =============================================================================

// A small viewport must still allocate a renderable target when scale rounds
// toward zero.
TEST(GGEMSPresentationScale, TargetExtentNeverDropsBelowOnePixel) {
  EXPECT_EQ(ComputeSceneTargetExtent(
              GGEMSPixelExtent{.width = 1U, .height = 1U}, 0.1F, 0.1F),
            (GGEMSPixelExtent{.width = 1U, .height = 1U}));
}

} // namespace
