#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::render {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */
GGEMSBanner::GGEMSBanner() { ; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSBanner::Draw(GGEMSTerminalFramebuffer &framebuffer,
                       GGEMSTerminalRenderer::Rect const &rect) {
  framebuffer.DrawRectBorder(rect.x, rect.y, rect.w, rect.h,
                             render::GREEN_Mint);
}
} // namespace ggems::render
