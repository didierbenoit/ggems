#include <gtest/gtest.h>

#include "GGEMS/ui/detail/GGEMSPresentationScale.hh"

namespace {

using ggems::ui::detail::ComputeSceneTargetExtent;
using ggems::ui::detail::GGEMSPixelExtent;
using ggems::ui::detail::NormalizeContentScale;

// =============================================================================
// =============================================================================

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

TEST(GGEMSPresentationScale, TargetExtentNeverDropsBelowOnePixel) {
  EXPECT_EQ(ComputeSceneTargetExtent(
              GGEMSPixelExtent{.width = 1U, .height = 1U}, 0.1F, 0.1F),
            (GGEMSPixelExtent{.width = 1U, .height = 1U}));
}

} // namespace
