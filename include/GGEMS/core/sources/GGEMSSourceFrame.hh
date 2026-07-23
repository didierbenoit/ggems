#pragma once

#include <array>

#include "GGEMS/core/geometry/GGEMSGeometryTypes.hh"

namespace ggems::core::sources {
inline constexpr double k_source_frame_parallel_tolerance{1.0e-6};
inline constexpr double k_source_frame_float_tolerance{1.0e-5};

struct GGEMSSourceFrame {
  geometry::Direction3 axis_x{.x = 1.0F, .y = 0.0F, .z = 0.0F};
  geometry::Direction3 axis_y{.x = 0.0F, .y = 1.0F, .z = 0.0F};
  geometry::Direction3 axis_z{.x = 0.0F, .y = 0.0F, .z = 1.0F};
};

[[nodiscard]] auto BuildSourceFrame(std::array<double, 3U> const &direction,
                                    std::array<double, 3U> const &up_reference)
    -> GGEMSSourceFrame;

[[nodiscard]] auto
BuildSourceFrameWithAutomaticUp(std::array<double, 3> const &direction)
    -> GGEMSSourceFrame;
} // namespace ggems::core::sources
