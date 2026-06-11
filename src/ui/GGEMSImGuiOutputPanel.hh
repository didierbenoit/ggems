#pragma once

#include <cstddef>
#include <array>

#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::render {
class GGEMSBanner;
struct WrappedLine;
} // namespace ggems::render

namespace ggems::ui {
class GGEMSImGuiOutputPanel {
public:
  GGEMSImGuiOutputPanel() = default;
  ~GGEMSImGuiOutputPanel() = default;

  GGEMSImGuiOutputPanel(GGEMSImGuiOutputPanel const &) = delete;
  GGEMSImGuiOutputPanel(GGEMSImGuiOutputPanel &&) = delete;
  GGEMSImGuiOutputPanel &operator=(GGEMSImGuiOutputPanel const &) = delete;
  GGEMSImGuiOutputPanel &operator=(GGEMSImGuiOutputPanel &&) = delete;

public:
  void Render(render::GGEMSBanner const &banner,
              core::GGEMSOutputState &output_state);

private:
  void RenderWrappedLine(render::WrappedLine const &line) const;
  [[nodiscard]] bool ShouldDisplay(core::RenderedLogLine const &line) const;

private:
  std::size_t max_visible_lines_{2000U};
  bool auto_scroll_{true};
  bool show_prefix_{true};
  bool show_banner_{true};
  std::array<bool, 5> show_info_depth_{{true, true, true, true, true}};
  bool show_debug_{true};
  bool show_warn_{true};
  bool show_error_{true};
};
} // namespace ggems::ui
