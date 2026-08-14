#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSColor.hh"

namespace {

namespace render = ggems::render;

constexpr auto k_constexpr_rgb =
    render::MakeRGB(std::uint8_t{12U}, std::uint8_t{34U}, std::uint8_t{56U});

// =============================================================================
// =============================================================================

static_assert(k_constexpr_rgb.red == 12U);
static_assert(k_constexpr_rgb.green == 34U);
static_assert(k_constexpr_rgb.blue == 56U);
static_assert(render::BrightenChannel(std::uint8_t{0U}) == 85U);
static_assert(render::FaintChannel(std::uint8_t{255U}) == 170U);

// =============================================================================
// =============================================================================

auto ExpectRGB(render::RGB const &color, std::uint8_t red, std::uint8_t green,
               std::uint8_t blue) -> void {
  EXPECT_EQ(color.red, red);
  EXPECT_EQ(color.green, green);
  EXPECT_EQ(color.blue, blue);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSColorTest, BuildsRgbAndRepresentativePaletteEntries) {
  ExpectRGB(render::MakeRGB(12U, 34U, 56U), 12U, 34U, 56U);
  ExpectRGB(render::base_palette[static_cast<std::size_t>(
                render::ColorFamily::Gray)][0U],
            16U, 16U, 16U);
  ExpectRGB(render::base_palette[static_cast<std::size_t>(
                render::ColorFamily::Green)][6U],
            0U, 255U, 0U);
  ExpectRGB(render::base_palette[static_cast<std::size_t>(
                render::ColorFamily::Orange)][12U],
            180U, 88U, 38U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorTest, TransformsChannelsAndAppliesVariants) {
  EXPECT_EQ(render::BrightenChannel(0U), 85U);
  EXPECT_EQ(render::BrightenChannel(90U), 145U);
  EXPECT_EQ(render::BrightenChannel(255U), 255U);
  EXPECT_EQ(render::FaintChannel(0U), 0U);
  EXPECT_EQ(render::FaintChannel(90U), 60U);
  EXPECT_EQ(render::FaintChannel(255U), 170U);

  auto const base = render::MakeRGB(90U, 120U, 150U);
  ExpectRGB(render::ApplyVariant(base, render::ColorVariant::Normal), 90U, 120U,
            150U);
  ExpectRGB(render::ApplyVariant(base, render::ColorVariant::Bright), 145U,
            165U, 185U);
  ExpectRGB(render::ApplyVariant(base, render::ColorVariant::Faint), 60U, 80U,
            100U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorTest, RetrievesPaletteColorsAndClampsShade) {
  ExpectRGB(render::GetColorRGB(render::ColorFamily::Green, 6U,
                                render::ColorVariant::Normal),
            0U, 255U, 0U);
  ExpectRGB(render::GetColorRGB(render::ColorFamily::Blue,
                                std::numeric_limits<std::uint8_t>::max(),
                                render::ColorVariant::Normal),
            3U, 5U, 7U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorTest, EncodesForegroundAndBackgroundAnsiColors) {
  auto const foreground = render::MakeColor(render::ColorFamily::Green, 6U,
                                            render::ColorVariant::Normal,
                                            render::ColorLayer::Foreground);
  auto const background = render::MakeColor(render::ColorFamily::Green, 6U,
                                            render::ColorVariant::Normal,
                                            render::ColorLayer::Background);

  EXPECT_EQ(render::AnsiColor(foreground), "\033[38;2;0;255;0m");
  EXPECT_EQ(render::AnsiColor(background), "\033[48;2;0;255;0m");
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorTest, AppendAnsiColorMatchesAnsiColor) {
  constexpr std::array keys{
      render::MakeColor(render::ColorFamily::Red, 3U,
                        render::ColorVariant::Bright,
                        render::ColorLayer::Foreground),
      render::MakeColor(render::ColorFamily::Cyan, 9U,
                        render::ColorVariant::Faint,
                        render::ColorLayer::Background),
  };

  for (auto const &key : keys) {
    std::string appended{"prefix"};
    render::AppendAnsiColor(appended, key);
    EXPECT_EQ(appended, std::string{"prefix"} + render::AnsiColor(key));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorTest, AnsiControlCodesMatchAppendOperation) {
  struct ControlCase {
    render::AnsiControl control;
    std::string_view expected;
  };

  constexpr std::array test_cases{
      ControlCase{.control = render::AnsiControl::ResetAll,
                  .expected = "\033[0m"},
      ControlCase{.control = render::AnsiControl::ResetColor,
                  .expected = "\033[39;49m"},
      ControlCase{.control = render::AnsiControl::Bold, .expected = "\033[1m"},
      ControlCase{.control = render::AnsiControl::Faint, .expected = "\033[2m"},
  };

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.expected);
    EXPECT_EQ(render::AnsiControlCode(test_case.control), test_case.expected);

    std::string appended{"prefix"};
    render::AppendAnsiControl(appended, test_case.control);
    std::string expected_output{"prefix"};
    expected_output.append(test_case.expected);
    EXPECT_EQ(appended, expected_output);
  }
}
