#pragma once

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/render/GGEMSTerminalRenderer.hh"

namespace ggems::render {
class GGEMSBanner {
public:
  GGEMSBanner();
  ~GGEMSBanner() = default;

  GGEMSBanner(GGEMSBanner const &) = delete;
  GGEMSBanner(GGEMSBanner &&) = delete;
  GGEMSBanner &operator=(GGEMSBanner const &) = delete;
  GGEMSBanner &operator=(GGEMSBanner &&) = delete;

public:
  void Draw(GGEMSTerminalFramebuffer &framebuffer,
            GGEMSTerminalRenderer::Rect const &rect);
  void EmitToLogger(core::GGEMSLogger &) { ; }

private:
};
} // namespace ggems::render
