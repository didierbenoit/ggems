#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSColour.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMSVulkanColorConversion.hh"

namespace {

namespace render = ggems::render;
using ggems::ui::detail::ToVulkanClearColor;

constexpr std::array<float, 4U> k_expected_blue_abyss{
    0.011764707F, 0.019607844F, 0.027450982F, 1.0F};
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
  ExpectVulkanColor(ToVulkanClearColor(render::GREEN_Deep),
                    {0.0F, 0.18823531F, 0.0F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, AllMaximumByteChannelsMapToOne) {
  ExpectVulkanColor(ToVulkanClearColor(render::WHITE_Pure),
                    {1.0F, 1.0F, 1.0F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, NamedColorMapsToExplicitComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato),
                    {0.86274517F, 0.078431375F, 0.23529413F, 1.0F});
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
                    {0.9058824F, 0.38431376F, 0.4901961F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, FaintVariantMapsToExplicitComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato_F),
                    {0.57254905F, 0.050980397F, 0.15686275F, 1.0F});
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
  constexpr render::ColourKey out_of_range_color{
      .family = render::ColourFamily::Blue,
      .shade = std::numeric_limits<std::uint8_t>::max(),
      .variant = render::ColourVariant::Normal,
      .layer = render::ColourLayer::Foreground};

  ExpectVulkanColor(ToVulkanClearColor(out_of_range_color),
                    k_expected_blue_abyss);
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
