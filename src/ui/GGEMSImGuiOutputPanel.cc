#include <string_view>
#include <cstddef>
#include <string>
#include <cstdint>
#include <vector>

#include <imgui.h>

#include "GGEMS/logging/GGEMSOutputState.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/ui/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/GGEMSImGuiOutputPanel.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/utf/GGEMSUTF.hh"

namespace {

auto RenderTextLine(std::string_view text, ggems::render::ColorKey const &color)
    -> void {
  ImGui::PushStyleColor(ImGuiCol_Text, ggems::ui::ToImGuiColor(color));

  if (text.empty()) {
    ImGui::Dummy(ImVec2{0.0F, ImGui::GetTextLineHeight()});
  } else {
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
  }

  ImGui::PopStyleColor();
}

// =============================================================================
// =============================================================================

auto RenderMultilineText(std::string_view text,
                         ggems::render::ColorKey const &color) -> void {
  if (text.empty()) {
    RenderTextLine(text, color);
    return;
  }

  std::size_t start{0U};

  while (start < text.size()) {
    std::size_t const end = text.find('\n', start);

    if (end == std::string_view::npos) {
      RenderTextLine(text.substr(start), color);
      break;
    }

    RenderTextLine(text.substr(start, end - start), color);
    start = end + 1U;
  }
}

// =============================================================================
// =============================================================================

auto RemoveLeadingBlockNewline(std::string_view text) noexcept
    -> std::string_view {
  if (!text.empty() && text.front() == '\n') {
    text.remove_prefix(1U);
  }

  return text;
}

} // namespace

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto GGEMSImGuiOutputPanel::RenderWrappedLine(render::WrappedLine const &line)
    -> void {
  if (line.segments.empty()) {
    ImGui::Spacing();
    return;
  }

  bool first_segment{true};

  for (render::VisualSegment const &segment : line.segments) {
    if (!first_segment) {
      ImGui::SameLine(0.0F, 0.0F);
    }

    std::string text = utf::UTF32ToUTF8(segment.text);

    ImGui::PushStyleColor(ImGuiCol_Text, ToImGuiColor(segment.color));
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();

    first_segment = false;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiOutputPanel::ShouldDisplay(
    core::RenderedLogLine const &line) const noexcept -> bool {
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

auto GGEMSImGuiOutputPanel::Render(core::GGEMSOutputState &output_state)
    -> void {
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

    ImGui::Checkbox("INFO", show_info_depth_.data());
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

  ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImGuiColor(render::GRAY_Void));

  ImGui::BeginChild("GGEMSOutputLogRegion", ImVec2{0.0F, 0.0F},
                    ImGuiChildFlags_Borders,
                    ImGuiWindowFlags_HorizontalScrollbar);

  if (show_banner_) {
    auto const banner_lines = render::BuildBannerLines();

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

    bool const is_multiline = line.msg.contains('\n');

    if (show_prefix_ && !line.prefix.empty()) {
      RenderTextLine(line.prefix, line.color);

      if (!is_multiline) {
        ImGui::SameLine();
      }
    }

    std::string_view message = line.msg;

    if (is_multiline) {
      message = RemoveLeadingBlockNewline(message);
      RenderMultilineText(message, render::WHITE_Bone);
    } else {
      RenderTextLine(message, render::WHITE_Bone);
    }
  }

  if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0F);
  }

  ImGui::EndChild();

  ImGui::PopStyleColor();

  ImGui::End();
}

} // namespace ggems::ui
