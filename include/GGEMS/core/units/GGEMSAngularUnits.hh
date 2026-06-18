#pragma once

#include <compare>
#include <format>
#include <string>

namespace ggems::units {

namespace detail {
inline constexpr long double k_pi{3.141592653589793238462643383279502884L};
}

struct Angle {
  long double radians{0.0L};

  constexpr auto operator<=>(Angle const &) const = default;
};

constexpr Angle MakeRadians(long double radians) noexcept {
  return Angle{radians};
}

constexpr Angle MakeDegrees(long double degrees) noexcept {
  return Angle{degrees * detail::k_pi / 180.0L};
}

constexpr long double ToRadians(Angle angle) noexcept { return angle.radians; }

constexpr long double ToDegrees(Angle angle) noexcept {
  return angle.radians * 180.0L / detail::k_pi;
}

constexpr Angle operator+(Angle lhs, Angle rhs) noexcept {
  return Angle{lhs.radians + rhs.radians};
}

constexpr Angle operator-(Angle lhs, Angle rhs) noexcept {
  return Angle{lhs.radians - rhs.radians};
}

constexpr Angle operator-(Angle angle) noexcept {
  return Angle{-angle.radians};
}

constexpr Angle operator*(Angle angle, long double scale) noexcept {
  return Angle{angle.radians * scale};
}

constexpr Angle operator*(long double scale, Angle angle) noexcept {
  return Angle{angle.radians * scale};
}

constexpr Angle operator/(Angle angle, long double scale) noexcept {
  return Angle{angle.radians / scale};
}

inline std::string HumanReadable(Angle angle, std::int8_t precision = 3,
                                 std::int8_t width = -1) {
  std::string fmt;

  if (width < 0) {
    fmt = std::format("{{:.{}f}} deg", precision);
  } else {
    fmt = std::format("{{:{}.{}f}} deg", width, precision);
  }

  long double degrees = ToDegrees(angle);
  return std::vformat(fmt, std::make_format_args(degrees));
}

consteval Angle operator""_rad(long double value) noexcept {
  return MakeRadians(value);
}

consteval Angle operator""_rad(unsigned long long value) noexcept {
  return MakeRadians(static_cast<long double>(value));
}

consteval Angle operator""_deg(long double value) noexcept {
  return MakeDegrees(value);
}

consteval Angle operator""_deg(unsigned long long value) noexcept {
  return MakeDegrees(static_cast<long double>(value));
}

} // namespace ggems::units

template <> struct std::formatter<ggems::units::Angle> {
  std::int8_t precision{3};

  constexpr auto parse(std::format_parse_context &ctx) {
    auto it = ctx.begin();

    if (it != ctx.end() && *it >= '0' && *it < '9') {
      precision = static_cast<std::int8_t>(*it - '0');
      ++it;
    }

    if (it != ctx.end() && *it != '}') {
      throw std::format_error("Invalid GGEMS angle format specifier.");
    }

    return it;
  }

  auto format(ggems::units::Angle const &angle,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}",
                          ggems::units::HumanReadable(angle, precision));
  }
};
