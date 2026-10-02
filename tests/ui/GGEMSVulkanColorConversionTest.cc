#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"

namespace {

namespace render = ggems::render;
using ggems::ui::detail::ToVulkanClearColor;

constexpr float k_color_tolerance{1.0e-6F};

constexpr std::array<float, 4U> k_expected_blue_abyss{
  0.000910581F,
  0.001517635F,
  0.002124689F,
  1.0F,
};

static_assert(noexcept(ToVulkanClearColor(render::BLUE_Abyss)));

// =============================================================================
// =============================================================================

auto ExpectVulkanColor(std::array<float, 4U> const &actual,
                       std::array<float, 4U> const &expected) -> void {
  EXPECT_NEAR(actual[0U], expected[0U], k_color_tolerance);
  EXPECT_NEAR(actual[1U], expected[1U], k_color_tolerance);
  EXPECT_NEAR(actual[2U], expected[2U], k_color_tolerance);
  EXPECT_NEAR(actual[3U], expected[3U], k_color_tolerance);
}
} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, ZeroByteChannelsMapToZeroLinearChannels) {
  ExpectVulkanColor(ToVulkanClearColor(render::GREEN_DeepGreen),
                    {0.0F, 0.029556835F, 0.0F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, AllMaximumByteChannelsMapToOne) {
  ExpectVulkanColor(ToVulkanClearColor(render::WHITE_White),
                    {1.0F, 1.0F, 1.0F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, NamedColorMapsToLinearComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato),
                    {1.0F, 0.124771819F, 0.063010015F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, BlueAbyssMatchesLinearClearColor) {
  ExpectVulkanColor(ToVulkanClearColor(render::BLUE_Abyss),
                    k_expected_blue_abyss);
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, BrightVariantMapsToLinearComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato_B),
                    {1.0F, 0.309468925F, 0.230740055F, 1.0F});
}

// =============================================================================
// =============================================================================

TEST(GGEMSVulkanColorConversion, FaintVariantMapsToLinearComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato_F),
                    {0.401977777F, 0.054480277F, 0.028426040F, 1.0F});
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
