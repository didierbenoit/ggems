#pragma once

#include <cmath>
#include <cstdint>
#include <optional>

namespace ggems::geometry {
using CoordinatePM = std::int64_t;
using DistancePM = std::uint64_t;

struct Position3PM {
  CoordinatePM x{0};
  CoordinatePM y{0};
  CoordinatePM z{0};

  constexpr auto operator<=>(Position3PM const &) const = default;
};

struct Displacement3PM {
  CoordinatePM x{0};
  CoordinatePM y{0};
  CoordinatePM z{0};

  constexpr auto operator<=>(Displacement3PM const &) const = default;
};

struct Direction3 {
  float x{0.0F};
  float y{0.0F};
  float z{1.0F};
};

constexpr auto MakePositionPM(CoordinatePM pos_x, CoordinatePM pos_y,
                              CoordinatePM pos_z) noexcept -> Position3PM {
  return Position3PM{.x = pos_x, .y = pos_y, .z = pos_z};
}

constexpr auto MakeDisplacementPM(CoordinatePM dis_x, CoordinatePM dis_y,
                                  CoordinatePM dis_z) noexcept
    -> Displacement3PM {
  return Displacement3PM{.x = dis_x, .y = dis_y, .z = dis_z};
}

constexpr auto operator+(Position3PM position,
                         Displacement3PM displacement) noexcept -> Position3PM {
  return Position3PM{.x = position.x + displacement.x,
                     .y = position.y + displacement.y,
                     .z = position.z + displacement.z};
}

constexpr auto operator-(Position3PM position,
                         Displacement3PM displacement) noexcept -> Position3PM {
  return Position3PM{.x = position.x - displacement.x,
                     .y = position.y - displacement.y,
                     .z = position.z - displacement.z};
}

constexpr auto operator-(Position3PM lhs, Position3PM rhs) noexcept
    -> Displacement3PM {
  return Displacement3PM{
      .x = lhs.x - rhs.x, .y = lhs.y - rhs.y, .z = lhs.z - rhs.z};
}

constexpr auto operator+(Displacement3PM lhs, Displacement3PM rhs) noexcept
    -> Displacement3PM {
  return Displacement3PM{
      .x = lhs.x + rhs.x, .y = lhs.y + rhs.y, .z = lhs.z + rhs.z};
}

constexpr auto operator-(Displacement3PM lhs, Displacement3PM rhs) noexcept
    -> Displacement3PM {
  return Displacement3PM{
      .x = lhs.x - rhs.x, .y = lhs.y - rhs.y, .z = lhs.z - rhs.z};
}

constexpr auto operator-(Displacement3PM displacement) noexcept
    -> Displacement3PM {
  return Displacement3PM{
      .x = -displacement.x, .y = -displacement.y, .z = -displacement.z};
}

[[nodiscard]] inline auto SquaredNorm(Direction3 direction) noexcept -> float {
  auto const dir_x = static_cast<double>(direction.x);
  auto const dir_y = static_cast<double>(direction.y);
  auto const dir_z = static_cast<double>(direction.z);
  return static_cast<float>((dir_x * dir_x) + (dir_y * dir_y) +
                            (dir_z * dir_z));
}

[[nodiscard]] inline auto Norm(Direction3 direction) noexcept -> float {
  return static_cast<float>(std::hypot(static_cast<double>(direction.x),
                                       static_cast<double>(direction.y),
                                       static_cast<double>(direction.z)));
}

namespace detail {
struct NormalizedVector3D {
  double x;
  double y;
  double z;
};

[[nodiscard]] inline auto TryNormalizeVector3D(double x_val, double y_val,
                                               double z_val) noexcept
    -> std::optional<NormalizedVector3D> {
  if (!std::isfinite(x_val) || !std::isfinite(y_val) || !std::isfinite(z_val)) {
    return std::nullopt;
  }

  double const norm = std::hypot(x_val, y_val, z_val);
  if (!std::isfinite(norm) || !(norm > 0.0)) {
    return std::nullopt;
  }

  NormalizedVector3D const result{
      .x = x_val / norm, .y = y_val / norm, .z = z_val / norm};

  if (!std::isfinite(result.x) || !std::isfinite(result.y) ||
      !std::isfinite(result.z)) {
    return std::nullopt;
  }

  return result;
}
} // namespace detail

[[nodiscard]] inline auto TryMakeDirection3(double dir_x, double dir_y,
                                            double dir_z) noexcept
    -> std::optional<Direction3> {
  auto const precise = detail::TryNormalizeVector3D(dir_x, dir_y, dir_z);
  if (!precise.has_value()) {
    return std::nullopt;
  }

  Direction3 const result{.x = static_cast<float>(precise->x),
                          .y = static_cast<float>(precise->y),
                          .z = static_cast<float>(precise->z)};

  if (!std::isfinite(result.x) || !std::isfinite(result.y) ||
      !std::isfinite(result.z) || !(Norm(result) > 0.0F)) {
    return std::nullopt;
  }

  return result;
}

[[nodiscard]] inline auto Dot(Direction3 lhs, Direction3 rhs) noexcept
    -> float {
  return static_cast<float>(
      (static_cast<double>(lhs.x) * static_cast<double>(rhs.x)) +
      (static_cast<double>(lhs.y) * static_cast<double>(rhs.y)) +
      (static_cast<double>(lhs.z) * static_cast<double>(rhs.z)));
}
} // namespace ggems::geometry
