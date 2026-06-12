#include <imgui.h>

#include "GGEMSImGuiOutputPanel.hh"
#include "GGEMS/render/GGEMSColour.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSProgressBar.hh"
#include "GGEMS/utf/GGEMSUTF.hh"

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

void GGEMSImGuiOutputPanel::RenderWrappedLine(
    render::WrappedLine const &line) const {
  if (line.segments.empty()) {
    ImGui::Spacing();
    return;
  }

  bool first_segment{true};

  for (render::VisualSegment const &segment : line.segments) {
    if (!first_segment) {
      ImGui::SameLine(0.0f, 0.0f);
    }

    std::string text = utf::UTF32ToUTF8(segment.text);

    ImGui::PushStyleColor(ImGuiCol_Text, ToImGuiColour(segment.colour));
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();

    first_segment = false;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSImGuiOutputPanel::ShouldDisplay(
    core::RenderedLogLine const &line) const {
  switch (line.level) {
  case core::LogLevel::Debug:
    return show_debug_;

  case core::LogLevel::Warn:
    return show_warn_;

  case core::LogLevel::Error:
    return show_error_;

  case core::LogLevel::Info: {
    if (line.depth < 0) {
      return show_info_depth_[0];
    }

    if (line.depth >= static_cast<std::int32_t>(show_info_depth_.size())) {
      return show_info_depth_.back();
    }

    return show_info_depth_[static_cast<std::size_t>(line.depth)];
  }
  }

  return true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSImGuiOutputPanel::Render(
    render::GGEMSBanner const &banner, core::GGEMSOutputState &output_state,
    render::GGEMSProgressBar const &progress_bar) {
  ImGui::Begin("GGEMS Output");

  std::size_t log_count = output_state.GetLogCount();

  ImGui::Text("Logs: %zu", log_count);

  ImGui::Separator();

  if (ImGui::Button("Clear logs")) {
    output_state.ClearLogs();
  }

  ImGui::SameLine();
  if (ImGui::Button("Filters")) {
    ImGui::OpenPopup("GGEMSOutputFilters");
  }

  if (ImGui::BeginPopup("GGEMSOutputFilters")) {
    ImGui::TextUnformatted("Log levels");
    ImGui::Separator();

    ImGui::Checkbox("DEBUG", &show_debug_);
    ImGui::Checkbox("WARN", &show_warn_);
    ImGui::Checkbox("ERROR", &show_error_);

    ImGui::Spacing();
    ImGui::TextUnformatted("Info depth");
    ImGui::Separator();

    ImGui::Checkbox("INFO", &show_info_depth_[0]);
    ImGui::Checkbox("INFOEX 1", &show_info_depth_[1]);
    ImGui::Checkbox("INFOEX 2", &show_info_depth_[2]);
    ImGui::Checkbox("INFOEX 3", &show_info_depth_[3]);
    ImGui::Checkbox("INFOEX 4+", &show_info_depth_[4]);

    ImGui::EndPopup();
  }

  ImGui::SameLine();
  ImGui::Checkbox("Auto_scroll", &auto_scroll_);

  ImGui::SameLine();
  ImGui::Checkbox("Prefix", &show_prefix_);

  ImGui::SameLine();
  ImGui::Checkbox("Banner", &show_banner_);

  ImGui::SameLine();
  ImGui::Checkbox("Progress", &show_progress_);

  ImGui::Separator();

  ImGui::PushStyleColor(ImGuiCol_ChildBg,
                        ToImGuiColour(ggems::render::DEFAULT_BG));

  ImGui::BeginChild("GGEMSOutputLogRegion", ImVec2{0.0f, 0.0f}, true,
                    ImGuiWindowFlags_HorizontalScrollbar);

  if (show_banner_) {
    std::vector<render::WrappedLine> banner_lines =
        banner.BuildLines(banner.GetWidth());

    for (render::WrappedLine const &line : banner_lines) {
      RenderWrappedLine(line);
    }

    ImGui::Separator();
  }

  std::vector<core::RenderedLogLine> lines =
      output_state.GetLastLogLinesSnapshot(max_visible_lines_);

  for (core::RenderedLogLine const &line : lines) {
    if (!ShouldDisplay(line)) {
      continue;
    }

    if (show_prefix_) {
      ImGui::PushStyleColor(ImGuiCol_Text, ToImGuiColour(line.color));
      ImGui::TextUnformatted(line.prefix.c_str());
      ImGui::PopStyleColor();

      ImGui::SameLine();
    }

    ImGui::PushStyleColor(ImGuiCol_Text,
                          ToImGuiColour(ggems::render::DEFAULT_FG));
    ImGui::TextUnformatted(line.msg.c_str());
    ImGui::PopStyleColor();
  }

  if (show_progress_) {
    std::vector<render::WrappedLine> progress_lines = progress_bar.BuildLines();

    if (!progress_lines.empty()) {
      ImGui::Separator();

      for (render::WrappedLine const &line : progress_lines) {
        RenderWrappedLine(line);
      }
    }
  }

  if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();

  ImGui::PopStyleColor();

  ImGui::End();
}

} // namespace ggems::ui
