#pragma once

#include <cstddef>

#include "GGEMS/core/GGEMSOutputState.hh"

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
  void Render(core::GGEMSOutputState &output_state);

private:
  std::size_t max_visible_lines_{2000U};
  bool auto_scroll_{true};
  bool show_prefix_{true};
};
} // namespace ggems::ui
