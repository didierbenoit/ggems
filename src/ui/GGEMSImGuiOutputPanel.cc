#include <imgui.h>
#include <string_view>

#include "GGEMSImGuiOutputPanel.hh"
#include "GGEMS/render/GGEMSColour.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMSImGuiTheme.hh"

namespace {

void RenderTextLine(std::string_view text,
                    ggems::render::ColourKey const &color) {
  ImGui::PushStyleColor(ImGuiCol_Text, ggems::ui::ToImGuiColour(color));

  if (text.empty()) {
    ImGui::Dummy(ImVec2{0.0F, ImGui::GetTextLineHeight()});
  } else {
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
  }

  ImGui::PopStyleColor();
}

// =============================================================================
// =============================================================================

void RenderMultilineText(std::string_view text,
                         ggems::render::ColourKey const &colour) {
  if (text.empty()) {
    RenderTextLine(text, colour);
    return;
  }

  std::size_t start{0U};

  while (start < text.size()) {
    std::size_t const end = text.find('\n', start);

    if (end == std::string_view::npos) {
      RenderTextLine(text.substr(start), colour);
      break;
    }

    RenderTextLine(text.substr(start, end - start), colour);
    start = end + 1U;
  }
}

// =============================================================================
// =============================================================================

std::string_view RemoveLeadingBlockNewline(std::string_view text) noexcept {
  if (!text.empty() && text.front() == '\n') {
    text.remove_prefix(1U);
  }

  return text;
}

} // namespace

namespace ggems::ui {

// =============================================================================
// =============================================================================

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void GGEMSImGuiOutputPanel::Render(render::GGEMSBanner const &banner,
                                   core::GGEMSOutputState &output_state) {
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

  ImGui::Separator();

  ImGui::PushStyleColor(ImGuiCol_ChildBg,
                        ToImGuiColour(render::GGEMS_THEME_OUTPUT_BACKGROUND));

  ImGui::PushStyleColor(
      ImGuiCol_Border, ToImGuiColour(ggems::render::GGEMS_THEME_OUTPUT_BORDER));

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

    bool const is_multiline = line.msg.find('\n') != std::string::npos;

    if (show_prefix_ && !line.prefix.empty()) {
      RenderTextLine(line.prefix, line.color);

      if (!is_multiline) {
        ImGui::SameLine();
      }
    }

    std::string_view message = line.msg;

    if (is_multiline) {
      message = RemoveLeadingBlockNewline(message);
      RenderMultilineText(message, ggems::render::GGEMS_THEME_OUTPUT_TEXT);
    } else {
      RenderTextLine(message, ggems::render::GGEMS_THEME_OUTPUT_TEXT);
    }
  }

  if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();

  ImGui::PopStyleColor(2);

  ImGui::End();
}

} // namespace ggems::ui
