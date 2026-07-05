#pragma once

#include <imgui.h>

#include "GGEMS/render/GGEMSColour.hh"

namespace ggems::ui {
[[nodiscard]] ImVec4 ToImGuiColour(render::ColourKey const &colour);

void ApplyGGEMSImGuiTheme();
} // namespace ggems::ui
