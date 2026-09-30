#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"

namespace {

namespace render = ggems::render;
using ggems::ui::detail::ToVulkanClearColor;

constexpr std::array<float, 4U> k_expected_blue_abyss{
  0.011764707F,
  0.019607844F,
  0.027450982F,
  1.0F,
};

constexpr auto k_constexpr_blue_abyss = ToVulkanClearColor(render::BLUE_Abyss);

static_assert(k_constexpr_blue_abyss == k_expected_blue_abyss);
static_assert(noexcept(ToVulkanClearColor(render::BLUE_Abyss)));

// =============================================================================
// =============================================================================

auto ExpectVulkanColor(std::array<float, 4U> const &actual,
                       std::array<float, 4U> const &expected) -> void {
  EXPECT_FLOAT_EQ(actual[0U], expected[0U]);
  EXPECT_FLOAT_EQ(actual[1U], expected[1U]);
  EXPECT_FLOAT_EQ(actual[2U], expected[2U]);
  EXPECT_FLOAT_EQ(actual[3U], expected[3U]);
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, ZeroByteChannelsMapToZeroFloatChannels) {
  ExpectVulkanColor(ToVulkanClearColor(render::GREEN_DeepGreen),
                    {0.0F, 0.18823531F, 0.0F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, AllMaximumByteChannelsMapToOne) {
  ExpectVulkanColor(ToVulkanClearColor(render::WHITE_White),
                    {1.0F, 1.0F, 1.0F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, NamedColorMapsToExplicitComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato),
                    {1.0F, 0.38823533F, 0.27843139F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, BlueAbyssMatchesCurrentClearColor) {
  ExpectVulkanColor(ToVulkanClearColor(render::BLUE_Abyss),
                    k_expected_blue_abyss);
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, BrightVariantMapsToExplicitComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato_B),
                    {1.0F, 0.59215689F, 0.51764709F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, FaintVariantMapsToExplicitComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato_F),
                    {0.66666669F, 0.25882354F, 0.18431373F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, IgnoresForegroundAndBackgroundLayer) {
  auto const foreground = ToVulkanClearColor(render::BLUE_Abyss);
  auto const background = ToVulkanClearColor(render::BLUE_Abyss_BG);

  ExpectVulkanColor(foreground, k_expected_blue_abyss);
  ExpectVulkanColor(background, k_expected_blue_abyss);
  EXPECT_EQ(foreground, background);
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, ClampsShadeAbovePaletteRange) {
  constexpr render::ColorKey out_of_range_color{
    .family = render::ColorFamily::Blue,
    .shade = std::numeric_limits<std::uint8_t>::max(),
    .variant = render::ColorVariant::Normal,
    .layer = render::ColorLayer::Foreground,
  };

  EXPECT_EQ(ToVulkanClearColor(out_of_range_color),
            ToVulkanClearColor(render::BLUE_Tone90));
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, AlwaysReturnsOpaqueAlpha) {
  auto const color = ToVulkanClearColor(render::RED_Tomato_F);
  EXPECT_FLOAT_EQ(color[3U], 1.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, SupportsConstantEvaluation) {
  ExpectVulkanColor(k_constexpr_blue_abyss, k_expected_blue_abyss);
}

} // namespace
