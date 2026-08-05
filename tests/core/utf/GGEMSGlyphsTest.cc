#include <array>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"

#include "support/GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::utf::GlyphSet;

struct GlyphCase {
  std::string_view name;
  char32_t GlyphSet::*member;
  char32_t ascii;
  char32_t unicode;
};

constexpr std::array<GlyphCase, 19U> k_glyph_cases{{
    {.name = "gamma",
     .member = &GlyphSet::gamma,
     .ascii = U'g',
     .unicode = U'γ'},
    {.name = "electron",
     .member = &GlyphSet::electron,
     .ascii = U'e',
     .unicode = U'β'},
    {.name = "proton",
     .member = &GlyphSet::proton,
     .ascii = U'p',
     .unicode = U'p'},
    {.name = "aionino",
     .member = &GlyphSet::aionino,
     .ascii = U'l',
     .unicode = U'λ'},
    {.name = "alpha",
     .member = &GlyphSet::alpha,
     .ascii = U'a',
     .unicode = U'α'},
    {.name = "neutron",
     .member = &GlyphSet::neutron,
     .ascii = U'n',
     .unicode = U'ν'},
    {.name = "minus",
     .member = &GlyphSet::minus,
     .ascii = U'-',
     .unicode = U'-'},
    {.name = "plus", .member = &GlyphSet::plus, .ascii = U'+', .unicode = U'+'},
    {.name = "pulse2",
     .member = &GlyphSet::pulse2,
     .ascii = U'.',
     .unicode = U'•'},
    {.name = "separator",
     .member = &GlyphSet::separator,
     .ascii = U'-',
     .unicode = U'─'},
    {.name = "block_filled",
     .member = &GlyphSet::block_filled,
     .ascii = U'#',
     .unicode = U'█'},
    {.name = "border_top_left",
     .member = &GlyphSet::border_top_left,
     .ascii = U'+',
     .unicode = U'╔'},
    {.name = "border_top_right",
     .member = &GlyphSet::border_top_right,
     .ascii = U'+',
     .unicode = U'╗'},
    {.name = "border_bottom_left",
     .member = &GlyphSet::border_bottom_left,
     .ascii = U'+',
     .unicode = U'╚'},
    {.name = "border_bottom_right",
     .member = &GlyphSet::border_bottom_right,
     .ascii = U'+',
     .unicode = U'╝'},
    {.name = "horizontal_line",
     .member = &GlyphSet::horizontal_line,
     .ascii = U'*',
     .unicode = U'═'},
    {.name = "vertical_line",
     .member = &GlyphSet::vertical_line,
     .ascii = U'*',
     .unicode = U'║'},
    {.name = "border_right",
     .member = &GlyphSet::border_right,
     .ascii = U'+',
     .unicode = U'╟'},
    {.name = "border_left",
     .member = &GlyphSet::border_left,
     .ascii = U'+',
     .unicode = U'╢'},
}};

TEST(GGEMSGlyphsTest, DefinesEveryAsciiAndUnicodeValue) {
  for (auto const &glyph_case : k_glyph_cases) {
    SCOPED_TRACE(std::string{glyph_case.name});
    EXPECT_EQ(ggems::utf::Ascii.*glyph_case.member, glyph_case.ascii);
    EXPECT_EQ(ggems::utf::Unicode.*glyph_case.member, glyph_case.unicode);
  }
}

TEST(GGEMSGlyphsTest, ReturnsAsciiTableForAsciiEncoding) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
  EXPECT_EQ(&ggems::utf::Glyphs(), &ggems::utf::Ascii);
}

TEST(GGEMSGlyphsTest, ReturnsUnicodeTableForUnicodeEncoding) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
  EXPECT_EQ(&ggems::utf::Glyphs(), &ggems::utf::Unicode);
}

} // namespace
