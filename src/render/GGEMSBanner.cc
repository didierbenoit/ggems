#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"

namespace ggems::render {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */
GGEMSBanner::GGEMSBanner() : width_{53}, height_{17} { ; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSBanner::Draw(GGEMSTerminalFramebuffer &framebuffer) {
  auto const &g = utf::Glyphs();

  framebuffer.DrawRectBorder(1, 1, width_, height_, render::GREEN_Mint);

  framebuffer.DrawHLine(2, 10, static_cast<std::int16_t>(width_ - 2),
                        g.separator, render::GREEN_Mint);

  // Logo superior part
  std::u32string block = std::u32string(1, g.block_filled);
  std::u32string border_1 = std::u32string(1, g.border_top_right);
  std::u32string border_2 = std::u32string(1, g.border_bottom_right);
  std::u32string border_3 = std::u32string(1, g.border_top_left);
  std::u32string border_4 = std::u32string(1, g.border_bottom_left);
  std::u32string h_line = std::u32string(1, g.horizontal_line);
  std::u32string v_line = std::u32string(1, g.vertical_line);

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

  framebuffer.DrawString(6, 3, logo_line1, render::GREEN_Mint);
  framebuffer.DrawString(5, 4, logo_line2, render::GREEN_Mint);
  framebuffer.DrawString(5, 5, logo_line3, render::GREEN_Mint);
  framebuffer.DrawString(5, 6, logo_line4, render::GREEN_Mint);
  framebuffer.DrawString(5, 7, logo_line5, render::GREEN_Mint);
  framebuffer.DrawString(6, 8, logo_line6, render::GREEN_Mint);

  // Text inferior part
  std::u32string text_line1 = U"GPU Geant4-based Monte Carlo Simulations";

  std::u32string text_line2 = U"Version 2.0 ";
  text_line2 += std::u32string(1, g.pulse2) + U" GGEMS Team ";
  text_line2 += std::u32string(1, g.pulse2) + U" https://ggems.fr";

  std::u32string text_line3 = U"Authors: Julien Bert & Didier Benoit";
  std::u32string text_line4 = U"Copyright (C) 2026 Licensed under GNU GPL v3.0";

  framebuffer.DrawString(7, 12, text_line1, render::GREEN_Mint);
  framebuffer.DrawString(6, 13, text_line2, render::GREEN_Mint);
  framebuffer.DrawString(8, 14, text_line3, render::GREEN_Mint);
  framebuffer.DrawString(5, 15, text_line4, render::GREEN_Mint);
}
} // namespace ggems::render
