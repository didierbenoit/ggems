#pragma once

#include <cstddef>
#include <array>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::render {
struct WrappedLine;
} // namespace ggems::render

namespace ggems::ui {
class GGEMSImGuiOutputPanel {
public:
  GGEMSImGuiOutputPanel() = default;
  ~GGEMSImGuiOutputPanel() = default;

  GGEMSImGuiOutputPanel(GGEMSImGuiOutputPanel const &) = delete;
  GGEMSImGuiOutputPanel(GGEMSImGuiOutputPanel &&) = delete;
  auto operator=(GGEMSImGuiOutputPanel const &)
      -> GGEMSImGuiOutputPanel & = delete;
  auto operator=(GGEMSImGuiOutputPanel &&) -> GGEMSImGuiOutputPanel & = delete;

  auto Render(core::GGEMSOutputState &output_state) -> void;

private:
  static auto RenderWrappedLine(render::WrappedLine const &line) -> void;

  [[nodiscard]] auto
  ShouldDisplay(core::RenderedLogLine const &line) const noexcept -> bool;

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
