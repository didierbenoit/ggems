#pragma once

#include <cstdint>

#include <imgui.h>

#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::ui {

enum class GGEMSThemeRole : std::uint8_t {
  WindowBackground,
  RecessedBackground,
  Selection,
  TabBackground,
  TabAccent,
  PrimaryText,
  MutedText,
  Accent,
  AccentStrong,
  Attention,
  AttentionStrong,
  SceneBackground,
  OutputBackground,
  Separator,
  Border,
  ScrollbarAccent,
  ApplicationBackground,
  PopupBackground,
};

inline constexpr float k_ggems_base_font_size{16.5F};

[[nodiscard]] auto GetThemeColorKey(GGEMSThemeRole role) noexcept
  -> render::ColorKey;

[[nodiscard]] auto GetThemeColor(GGEMSThemeRole role) noexcept -> ImVec4;

[[nodiscard]] auto ToImGuiColor(render::ColorKey const &color) noexcept
  -> ImVec4;

[[nodiscard]] auto BuildGGEMSStyle(float user_scale, float content_scale)
  -> ImGuiStyle;

} // namespace ggems::ui
