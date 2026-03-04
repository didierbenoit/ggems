#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"

namespace ggems::render {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */
GGEMSBanner::GGEMSBanner() : width_{52}, height_{17} { ; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSBanner::Draw(GGEMSTerminalFramebuffer &framebuffer) {
  auto const &g = utf::Glyphs();

  framebuffer.DrawRectBorder(1, 1, width_, height_, render::GREEN_Mint);

  framebuffer.DrawHLine(2, 10, static_cast<std::int16_t>(width_ - 2),
                        g.separator, render::GREEN_Mint);

  // Text inferior part
  std::u32string line1 = U"GPU Geant4-based Monte Carlo Simulations";

  std::u32string line2 = U"Version 2.0 ";
  line2 += std::u32string(1, g.pulse2) + U" https://ggems.fr";

  framebuffer.DrawString(6, 12, line1, render::GREEN_Mint);
  framebuffer.DrawString(5, 13, line2, render::GREEN_Mint);
  framebuffer.DrawString(2, 14, U"", render::GREEN_Mint);
  framebuffer.DrawString(2, 15, U"", render::GREEN_Mint);

  // ║    GPU Geant4-based Monte Carlo Simulations      ║
  // ║   Version 2.0 • GGEMS Team • https://ggems.fr    ║
  // ║     Authors: Julien Bert  &  Didier Benoit       ║
  // ║  Copyright © 2025  Licensed under GNU GPL v3.0   ║
}
} // namespace ggems::render
