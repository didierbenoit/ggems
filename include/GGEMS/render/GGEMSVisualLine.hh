#pragma once

#include <string>
#include <vector>

#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::render {
struct VisualSegment {
  std::u32string text{};
  ColourKey colour{DEFAULT_FG};
};

struct WrappedLine {
  std::vector<VisualSegment> segments{};
};
} // namespace ggems::render
