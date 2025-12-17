#include "GGEMS/render/GGEMSTerminalRenderer.hh"
#include "GGEMS/render/GGEMSBanner.hh"

namespace ggems::render {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSTerminalRenderer::~GGEMSTerminalRenderer() noexcept { Stop(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::Start() noexcept { presenter_.Begin(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::Stop() noexcept { presenter_.End(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::RenderOnce() {
  framebuffer_.UpdateSizeIfNeeded();
  framebuffer_.Clear(U' ', DEFAULT_FG);

  auto w = framebuffer_.Width();
  auto h = framebuffer_.Height();

  std::int16_t header_h = 8;
  if (header_h > h)
    header_h = h;

  Rect header{0, 0, w, header_h};
  banner_.Draw(framebuffer_, header);

  std::string const out = framebuffer_.Render();
  presenter_.Present(out);
}
} // namespace ggems::render
