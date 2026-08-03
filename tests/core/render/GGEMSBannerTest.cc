#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColourNames.hh"

#include "../support/GGEMSScopedLoggerEncoding.hh"

namespace {

using BannerSnapshot = std::array<std::u32string_view, 17U>;
using ggems::test::ScopedLoggerEncoding;

constexpr BannerSnapshot k_ascii_banner{{
    U"+****************************************************+",
    U"*                                                    *",
    U"*    ######+  ######+ #######+###+   ###+#######+    *",
    U"*   ##+****+ ##+****+ ##+****+####+ ####*##+****+    *",
    U"*   ##*  ###+##*  ###+#####+  ##+####+##*#######+    *",
    U"*   ##*   ##*##*   ##*##+**+  ##*+##++##*+****##*    *",
    U"*   +######+++######++#######+##* +*+ ##*#######*    *",
    U"*    +*****+  +*****+ +******++*+     +*++******+    *",
    U"*                                                    *",
    U"+----------------------------------------------------+",
    U"*                                                    *",
    U"*      GPU Geant4-based Monte Carlo Simulations      *",
    U"*    Version 2.0 . GGEMS Team . https://ggems.fr     *",
    U"*       Authors: Julien Bert & Didier Benoit         *",
    U"*   Copyright (C) 2026 Licensed under GNU GPL v3.0   *",
    U"*                                                    *",
    U"+****************************************************+",
}};

constexpr BannerSnapshot k_unicode_banner{{
    U"╔════════════════════════════════════════════════════╗",
    U"║                                                    ║",
    U"║    ██████╗  ██████╗ ███████╗███╗   ███╗███████╗    ║",
    U"║   ██╔════╝ ██╔════╝ ██╔════╝████╗ ████║██╔════╝    ║",
    U"║   ██║  ███╗██║  ███╗█████╗  ██╔████╔██║███████╗    ║",
    U"║   ██║   ██║██║   ██║██╔══╝  ██║╚██╔╝██║╚════██║    ║",
    U"║   ╚██████╔╝╚██████╔╝███████╗██║ ╚═╝ ██║███████║    ║",
    U"║    ╚═════╝  ╚═════╝ ╚══════╝╚═╝     ╚═╝╚══════╝    ║",
    U"║                                                    ║",
    U"╟────────────────────────────────────────────────────╢",
    U"║                                                    ║",
    U"║      GPU Geant4-based Monte Carlo Simulations      ║",
    U"║    Version 2.0 • GGEMS Team • https://ggems.fr     ║",
    U"║       Authors: Julien Bert & Didier Benoit         ║",
    U"║   Copyright (C) 2026 Licensed under GNU GPL v3.0   ║",
    U"║                                                    ║",
    U"╚════════════════════════════════════════════════════╝",
}};

auto ExpectNominalBanner(ggems::core::Encoding encoding,
                         BannerSnapshot const &expected_lines) -> void {
  ScopedLoggerEncoding const scoped_encoding{encoding};
  ggems::render::GGEMSBanner banner;

  auto const lines = banner.BuildLines(banner.GetWidth());
  auto const untruncated_lines =
      banner.BuildLines(std::numeric_limits<std::int16_t>::max());

  ASSERT_EQ(lines.size(), expected_lines.size());
  ASSERT_EQ(untruncated_lines.size(), lines.size());
  EXPECT_EQ(lines.size(), static_cast<std::size_t>(banner.GetHeight()));

  for (std::size_t line_index = 0U; line_index < lines.size(); ++line_index) {
    SCOPED_TRACE(line_index);
    ASSERT_EQ(lines[line_index].segments.size(), 1U);

    auto const &segment = lines[line_index].segments.front();
    ASSERT_EQ(untruncated_lines[line_index].segments.size(), 1U);
    EXPECT_EQ(segment.colour, ggems::render::GREEN_Acid);
    EXPECT_EQ(segment.text, expected_lines[line_index]);
    EXPECT_EQ(segment.text,
              untruncated_lines[line_index].segments.front().text);
    EXPECT_EQ(segment.text.size(),
              static_cast<std::size_t>(banner.GetWidth()));
  }
}

TEST(GGEMSBannerTest, BuildsCompleteAsciiBanner) {
  ExpectNominalBanner(ggems::core::Encoding::Ascii, k_ascii_banner);
}

TEST(GGEMSBannerTest, BuildsCompleteUnicodeBanner) {
  ExpectNominalBanner(ggems::core::Encoding::Unicode, k_unicode_banner);
}

TEST(GGEMSBannerTest, HandlesEmptyAndReducedWidths) {
  ggems::render::GGEMSBanner banner;

  EXPECT_TRUE(banner.BuildLines(0).empty());
  EXPECT_TRUE(banner.BuildLines(-1).empty());

  ScopedLoggerEncoding const scoped_encoding{ggems::core::Encoding::Ascii};
  constexpr std::int16_t k_reduced_width{10};
  auto const lines = banner.BuildLines(k_reduced_width);

  ASSERT_EQ(lines.size(), k_ascii_banner.size());
  for (std::size_t line_index = 0U; line_index < lines.size(); ++line_index) {
    SCOPED_TRACE(line_index);
    ASSERT_EQ(lines[line_index].segments.size(), 1U);

    auto const &segment = lines[line_index].segments.front();
    EXPECT_EQ(segment.colour, ggems::render::GREEN_Acid);
    EXPECT_LE(segment.text.size(),
              static_cast<std::size_t>(k_reduced_width));
    EXPECT_EQ(segment.text,
              k_ascii_banner[line_index].substr(
                  0U, static_cast<std::size_t>(k_reduced_width)));
  }
}

} // namespace
