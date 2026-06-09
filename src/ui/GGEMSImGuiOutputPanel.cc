#include <imgui.h>

#include "GGEMSImGuiOutputPanel.hh"
#include "GGEMS/render/GGEMSColour.hh"

namespace {
[[nodiscard]] ImVec4 ToImGuiColour(ggems::render::ColourKey const &colour) {
  ggems::render::RGB rgb =
      ggems::render::GetColourRGB(colour.family, colour.shade, colour.variant);

  constexpr float k_inverse_255{1.0f / 255.0f};

  return ImVec4{static_cast<float>(rgb.r) * k_inverse_255,
                static_cast<float>(rgb.g) * k_inverse_255,
                static_cast<float>(rgb.b) * k_inverse_255, 1.0f};
}
} // namespace

namespace ggems::ui {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSImGuiOutputPanel::Render(core::GGEMSOutputState &output_state) {
  ImGui::Begin("GGEMS Output");

  std::size_t log_count = output_state.GetLogCount();

  ImGui::Text("Logs: %zu", log_count);

  ImGui::Separator();

  if (ImGui::Button("Clear")) {
    output_state.ClearLogs();
  }

  ImGui::SameLine();
  ImGui::Checkbox("Auto_scroll", &auto_scroll_);

  ImGui::SameLine();
  ImGui::Checkbox("Prefix", &show_prefix_);

  ImGui::Separator();

  ImGui::PushStyleColor(ImGuiCol_ChildBg,
                        ToImGuiColour(ggems::render::DEFAULT_BG));

  ImGui::BeginChild("GGEMSOutputLogRegion", ImVec2{0.0f, 0.0f}, true,
                    ImGuiWindowFlags_HorizontalScrollbar);

  std::vector<core::RenderedLogLine> lines =
      output_state.GetLastLogLinesSnapshot(max_visible_lines_);

  for (core::RenderedLogLine const &line : lines) {
    ImGui::PushStyleColor(ImGuiCol_Text, ToImGuiColour(line.color));

    if (show_prefix_) {
      ImGui::TextUnformatted(line.prefix.c_str());
      ImGui::SameLine();
    }

    ImGui::TextUnformatted(line.msg.c_str());

    ImGui::PopStyleColor();
  }

  if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();

  ImGui::PopStyleColor();

  ImGui::End();
}

} // namespace ggems::ui
