#include "GGEMS/render/GGEMSBanner.hh"

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
                             render::DEFAULT_FG);
}
} // namespace ggems::render
