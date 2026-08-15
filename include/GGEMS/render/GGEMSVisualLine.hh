#pragma once

#include <string>
#include <vector>

#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"

namespace ggems::render {
struct VisualSegment {
  std::u32string text;
  ColorKey color{DEFAULT_FG};
};

struct WrappedLine {
  std::vector<VisualSegment> segments;
};
} // namespace ggems::render
