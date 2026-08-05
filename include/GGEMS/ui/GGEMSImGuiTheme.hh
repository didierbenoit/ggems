#pragma once

#include <imgui.h>

#include "GGEMS/render/GGEMSColour.hh"

namespace ggems::ui {
[[nodiscard]] auto ToImGuiColour(render::ColourKey const &colour) -> ImVec4;

void ApplyGGEMSImGuiTheme();
} // namespace ggems::ui
