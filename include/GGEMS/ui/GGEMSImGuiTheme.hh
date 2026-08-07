#pragma once

#include <imgui.h>

#include "GGEMS/render/GGEMSColor.hh"

namespace ggems::ui {
[[nodiscard]] auto ToImGuiColor(render::ColorKey const &color) -> ImVec4;

void ApplyGGEMSImGuiTheme();
} // namespace ggems::ui
