#include <utility>
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"

namespace ggems::render {

render::ColorKey constexpr banner_color = render::GREEN_Acid;

// =============================================================================
// =============================================================================

auto GGEMSBanner::BuildLines(std::int16_t max_width) const
    -> std::vector<WrappedLine> {
  std::vector<WrappedLine> lines{};
  if (max_width <= 0) {
    return lines;
  }

  auto const &glyphs = utf::Glyphs();

  auto clip = [max_width](std::u32string text) -> std::u32string {
    if (static_cast<std::int16_t>(text.size()) > max_width) {
      text.resize(static_cast<std::size_t>(max_width));
    }
    return text;
  };

  auto push_line = [&](std::u32string text, render::ColorKey color) -> void {
    WrappedLine line{};
    line.segments.push_back({.text = clip(std::move(text)), .color = color});
    lines.push_back(std::move(line));
  };

  // Logo superior part
  std::u32string block = std::u32string(1, glyphs.block_filled);
  std::u32string border_1 = std::u32string(1, glyphs.border_top_right);
  std::u32string border_2 = std::u32string(1, glyphs.border_bottom_right);
  std::u32string border_3 = std::u32string(1, glyphs.border_top_left);
  std::u32string border_4 = std::u32string(1, glyphs.border_bottom_left);
  std::u32string h_line = std::u32string(1, glyphs.horizontal_line);
  std::u32string v_line = std::u32string(1, glyphs.vertical_line);

  std::u32string hline_border_top =
      border_3 +
      std::u32string(static_cast<std::size_t>(width_ - 2),
                     glyphs.horizontal_line) +
      border_1;

  std::u32string hline_border_bottom =
      border_4 +
      std::u32string(static_cast<std::size_t>(width_ - 2),
                     glyphs.horizontal_line) +
      border_2;

  std::u32string empty_line =
      v_line + std::u32string(static_cast<std::size_t>(width_ - 2), U' ') +
      v_line;

  std::u32string separator_line =
      std::u32string(1, glyphs.border_right) +
      std::u32string(static_cast<std::size_t>(width_ - 2), glyphs.separator) +
      std::u32string(1, glyphs.border_left);

  std::u32string logo_line1 =
      block + block + block + block + block + block + border_1;
  logo_line1 +=
      U"  " + block + block + block + block + block + block + border_1;
  logo_line1 +=
      U" " + block + block + block + block + block + block + block + border_1;
  logo_line1 += block + block + block + border_1 + U"   " + block + block +
                block + border_1;
  logo_line1 +=
      block + block + block + block + block + block + block + border_1;

  std::u32string logo_line2 =
      block + block + border_3 + h_line + h_line + h_line + h_line + border_2;
  logo_line2 += U" " + block + block + border_3 + h_line + h_line + h_line +
                h_line + border_2;
  logo_line2 += U" " + block + block + border_3 + h_line + h_line + h_line +
                h_line + border_2;
  logo_line2 += block + block + block + block + border_1 + U" " + block +
                block + block + block + v_line;
  logo_line2 +=
      block + block + border_3 + h_line + h_line + h_line + h_line + border_2;

  std::u32string logo_line3 =
      block + block + v_line + U"  " + block + block + block + border_1;
  logo_line3 +=
      block + block + v_line + U"  " + block + block + block + border_1;
  logo_line3 += block + block + block + block + block + border_1 + U"  ";
  logo_line3 += block + block + border_3 + block + block + block + block +
                border_3 + block + block + v_line;
  logo_line3 +=
      block + block + block + block + block + block + block + border_1;

  std::u32string logo_line4 =
      block + block + v_line + U"   " + block + block + v_line;
  logo_line4 += block + block + v_line + U"   " + block + block + v_line;
  logo_line4 += block + block + border_3 + h_line + h_line + border_2 + U"  ";
  logo_line4 += block + block + v_line + border_4 + block + block + border_3 +
                border_2 + block + block + v_line;
  logo_line4 +=
      border_4 + h_line + h_line + h_line + h_line + block + block + v_line;

  std::u32string logo_line5 = border_4 + block + block + block + block + block +
                              block + border_3 + border_2;
  logo_line5 += border_4 + block + block + block + block + block + block +
                border_3 + border_2;
  logo_line5 +=
      block + block + block + block + block + block + block + border_1;
  logo_line5 += block + block + v_line + U" " + border_4 + h_line + border_2 +
                U" " + block + block + v_line;
  logo_line5 += block + block + block + block + block + block + block + v_line;

  std::u32string logo_line6 =
      border_4 + h_line + h_line + h_line + h_line + h_line + border_2;
  logo_line6 +=
      U"  " + border_4 + h_line + h_line + h_line + h_line + h_line + border_2;
  logo_line6 += U" " + border_4 + h_line + h_line + h_line + h_line + h_line +
                h_line + border_2;
  logo_line6 +=
      border_4 + h_line + border_2 + U"     " + border_4 + h_line + border_2;
  logo_line6 +=
      border_4 + h_line + h_line + h_line + h_line + h_line + h_line + border_2;

  // Text inferior part
  std::u32string text_line1 = U"GPU Geant4-based Monte Carlo Simulations";

  std::u32string text_line2 = U"Version 2.0 ";
  text_line2 += std::u32string(1, glyphs.pulse2) + U" GGEMS Team ";
  text_line2 += std::u32string(1, glyphs.pulse2) + U" https://ggems.fr";

  std::u32string text_line3 = U"Authors: Julien Bert & Didier Benoit";
  std::u32string text_line4 = U"Copyright (C) 2026 Licensed under GNU GPL v3.0";

  push_line(hline_border_top, banner_color);
  push_line(empty_line, banner_color);

  push_line(v_line + U"    " + logo_line1 + U"    " + v_line, banner_color);
  push_line(v_line + U"   " + logo_line2 + U"    " + v_line, banner_color);
  push_line(v_line + U"   " + logo_line3 + U"    " + v_line, banner_color);
  push_line(v_line + U"   " + logo_line4 + U"    " + v_line, banner_color);
  push_line(v_line + U"   " + logo_line5 + U"    " + v_line, banner_color);
  push_line(v_line + U"    " + logo_line6 + U"    " + v_line, banner_color);

  push_line(empty_line, banner_color);
  push_line(separator_line, banner_color);
  push_line(empty_line, banner_color);

  push_line(v_line + U"      " + text_line1 + U"      " + v_line,
            banner_color);
  push_line(v_line + U"    " + text_line2 + U"     " + v_line, banner_color);
  push_line(v_line + U"       " + text_line3 + U"         " + v_line,
            banner_color);
  push_line(v_line + U"   " + text_line4 + U"   " + v_line, banner_color);

  push_line(empty_line, banner_color);
  push_line(hline_border_bottom, banner_color);

  return lines;
}
} // namespace ggems::render
