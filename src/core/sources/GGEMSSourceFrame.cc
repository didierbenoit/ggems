#include <array>
#include <cmath>
#include <format>
#include <string_view>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceFrame.hh"

namespace ggems::core::sources {
namespace {

using PreciseAxis = geometry::detail::NormalisedVector3D;
using Vector3D = std::array<double, 3U>;

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireNormalised(Vector3D const &vector,
                                     std::string_view name) -> PreciseAxis {
  GGEMS_CHECK_RECOVERABLE(
      std::isfinite(vector[0U]) && std::isfinite(vector[1U]) &&
          std::isfinite(vector[2U]),
      std::format("Source  {} must contain finite values.", name));

  auto const normalised =
      geometry::detail::TryNormaliseVector3D(vector[0], vector[1], vector[2]);

  GGEMS_CHECK_RECOVERABLE(
      normalised.has_value(),
      std::format("Source {} must have a finite, strictly positive norm.",
                  name));

  return *normalised;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto Dot(PreciseAxis lhs, PreciseAxis rhs) noexcept -> double {
  return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto Cross(PreciseAxis lhs, PreciseAxis rhs) noexcept
    -> Vector3D {
  return Vector3D{(lhs.y * rhs.z) - (lhs.z * rhs.y),
                  (lhs.z * rhs.x) - (lhs.x * rhs.z),
                  (lhs.x * rhs.y) - (lhs.y * rhs.x)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto IsTooParallel(PreciseAxis direction,
                                 PreciseAxis up_reference) noexcept -> bool {
  return 1.0 - std::abs(Dot(direction, up_reference)) <=
         k_source_frame_parallel_tolerance;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ToFloatDirection(PreciseAxis direction)
    -> geometry::Direction3 {
  auto const converted =
      geometry::TryMakeDirection3(direction.x, direction.y, direction.z);

  GGEMS_CHECK_INTERNAL(converted.has_value(),
                       "Source orientation cannot be represented by a finite, "
                       "non-zero float direction.");

  return *converted;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FloatHandedness(GGEMSSourceFrame const &frame) noexcept
    -> double {
  double const cross_x =
      (static_cast<double>(frame.axis_x.y) * frame.axis_y.z) -
      (static_cast<double>(frame.axis_x.z) * frame.axis_y.y);
  double const cross_y =
      (static_cast<double>(frame.axis_x.z) * frame.axis_y.x) -
      (static_cast<double>(frame.axis_x.x) * frame.axis_y.z);
  double const cross_z =
      (static_cast<double>(frame.axis_x.x) * frame.axis_y.y) -
      (static_cast<double>(frame.axis_x.y) * frame.axis_y.x);

  return (cross_x * frame.axis_z.x) + (cross_y * frame.axis_z.y) +
         (cross_z * frame.axis_z.z);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto IsValidFloatFrame(GGEMSSourceFrame const &frame) noexcept
    -> bool {
  auto const finite = [](geometry::Direction3 axis) noexcept -> bool {
    return std::isfinite(axis.x) && std::isfinite(axis.y) &&
           std::isfinite(axis.z);
  };

  if (!finite(frame.axis_x) || !finite(frame.axis_y) || !finite(frame.axis_z)) {
    return false;
  }

  double const tolerance = k_source_frame_float_tolerance;

  return std::abs(static_cast<double>(geometry::Norm(frame.axis_x)) - 1.0) <=
             tolerance &&
         std::abs(static_cast<double>(geometry::Norm(frame.axis_y)) - 1.0) <=
             tolerance &&
         std::abs(static_cast<double>(geometry::Norm(frame.axis_z)) - 1.0) <=
             tolerance &&
         std::abs(static_cast<double>(
             geometry::Dot(frame.axis_x, frame.axis_y))) <= tolerance &&
         std::abs(static_cast<double>(
             geometry::Dot(frame.axis_y, frame.axis_z))) <= tolerance &&
         std::abs(static_cast<double>(
             geometry::Dot(frame.axis_z, frame.axis_x))) <= tolerance &&
         std::abs(FloatHandedness(frame) - 1.0) <= tolerance;
}

// =============================================================================
// =============================================================================

auto BuildSourceFrameFromNormalised(PreciseAxis direction,
                                    PreciseAxis up_reference)
    -> GGEMSSourceFrame {
  GGEMS_CHECK_RECOVERABLE(!IsTooParallel(direction, up_reference),
                          std::format("Source direction and up are too close "
                                      "to parallel (1 - abs(dot) <= {}).",
                                      k_source_frame_parallel_tolerance));

  PreciseAxis const axis_x = RequireNormalised(Cross(up_reference, direction),
                                               "frame horizontal axis");
  PreciseAxis const axis_y =
      RequireNormalised(Cross(direction, axis_x), "frame vertical axis");

  GGEMSSourceFrame const frame = {.axis_x = ToFloatDirection(axis_x),
                                  .axis_y = ToFloatDirection(axis_y),
                                  .axis_z = ToFloatDirection(direction)};

  GGEMS_CHECK_INTERNAL(IsValidFloatFrame(frame),
                       "Source orientation cannot be represented by a "
                       "sufficiently orthonormal right-handed float frame.");

  return frame;
}

} // namespace

// =============================================================================
// =============================================================================

auto BuildSourceFrame(std::array<double, 3U> const &direction,
                      std::array<double, 3U> const &up_reference)
    -> GGEMSSourceFrame {
  return BuildSourceFrameFromNormalised(
      RequireNormalised(direction, "direction"),
      RequireNormalised(up_reference, "up vector"));
}

// =============================================================================
// =============================================================================

auto IsValidSourceFrame(GGEMSSourceFrame const &frame) noexcept -> bool {
  return IsValidFloatFrame(frame);
}

// =============================================================================
// =============================================================================

auto BuildSourceFrameWithAutomaticUp(std::array<double, 3U> const &direction)
    -> GGEMSSourceFrame {
  PreciseAxis const axis_z = RequireNormalised(direction, "direction");
  PreciseAxis constexpr preferred_up{.x = 0.0, .y = 0.0, .z = 1.0};
  PreciseAxis constexpr fallback_up{.x = 0.0, .y = 1.0, .z = 0.0};

  PreciseAxis const selected_up =
      IsTooParallel(axis_z, preferred_up) ? fallback_up : preferred_up;

  return BuildSourceFrameFromNormalised(axis_z, selected_up);
}

} // namespace ggems::core::sources
