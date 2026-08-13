#pragma once

#include <vector>

#include "GGEMS/render/GGEMSVisualLine.hh"

namespace ggems::render {
[[nodiscard]] auto BuildBannerLines() -> std::vector<WrappedLine>;
} // namespace ggems::render
