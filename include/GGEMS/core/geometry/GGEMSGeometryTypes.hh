#pragma once

#include <cmath>
#include <compare>
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
  float x{1.0f};
  float y{0.0f};
  float z{0.0f};
};

constexpr Position3PM MakePositionPM(CoordinatePM x, CoordinatePM y,
                                     CoordinatePM z) noexcept {
  return Position3PM{.x = x, .y = y, .z = z};
}

constexpr Displacement3PM MakeDisplacementPM(CoordinatePM x, CoordinatePM y,
                                             CoordinatePM z) noexcept {
  return Displacement3PM{.x = x, .y = y, .z = z};
}

constexpr Position3PM operator+(Position3PM position,
                                Displacement3PM displacement) noexcept {
  return Position3PM{.x = position.x + displacement.x,
                     .y = position.y + displacement.y,
                     .z = position.z + displacement.z};
}

constexpr Position3PM operator-(Position3PM position,
                                Displacement3PM displacement) noexcept {
  return Position3PM{.x = position.x - displacement.x,
                     .y = position.y - displacement.y,
                     .z = position.z - displacement.z};
}

constexpr Displacement3PM operator-(Position3PM lhs, Position3PM rhs) noexcept {
  return Displacement3PM{
      .x = lhs.x - rhs.x, .y = lhs.y - rhs.y, .z = lhs.z - rhs.z};
}

constexpr Displacement3PM operator+(Displacement3PM lhs,
                                    Displacement3PM rhs) noexcept {
  return Displacement3PM{
      .x = lhs.x + rhs.x, .y = lhs.y + rhs.y, .z = lhs.z + rhs.z};
}

constexpr Displacement3PM operator-(Displacement3PM lhs,
                                    Displacement3PM rhs) noexcept {
  return Displacement3PM{
      .x = lhs.x - rhs.x, .y = lhs.y - rhs.y, .z = lhs.z - rhs.z};
}

constexpr Displacement3PM operator-(Displacement3PM displacement) noexcept {
  return Displacement3PM{
      .x = -displacement.x, .y = -displacement.y, .z = -displacement.z};
}

inline float SquaredNorm(Direction3 direction) noexcept {
  return direction.x * direction.x + direction.y * direction.y +
         direction.z * direction.z;
}

inline float Norm(Direction3 direction) noexcept {
  return std::sqrt(SquaredNorm(direction));
}

inline std::optional<Direction3> TryMakeDirection3(float x, float y,
                                                   float z) noexcept {
  if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
    return std::nullopt;
  }

  float const squared_norm = x * x + y * y + z * z;

  if (squared_norm <= 1.0e-20F) {
    return std::nullopt;
  }

  float const inverse_norm = 1.0F / std::sqrt(squared_norm);

  return Direction3{
      .x = x * inverse_norm, .y = y * inverse_norm, .z = z * inverse_norm};
}

inline float Dot(Direction3 lhs, Direction3 rhs) noexcept {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

} // namespace ggems::geometry
