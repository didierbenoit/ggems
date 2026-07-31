#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

namespace ggems::core::radioactivity {
inline constexpr float k_radioactive_time_uniform_limit_scaled_decay{
    0x1.0p-14F};

[[nodiscard]] inline auto
ComputeRadioactiveTimeRelative(float uniform, float scaled_decay) noexcept
    -> float {
  if (scaled_decay <= k_radioactive_time_uniform_limit_scaled_decay) {
    return uniform;
  }

  float const decay_mass = -std::expm1(-scaled_decay);
  return -std::log1p(-uniform * decay_mass) / scaled_decay;
}

[[nodiscard]] inline auto
QuantizeRadioactiveTimeRelative(float relative) noexcept -> std::uint32_t {
  constexpr double k_ticket_scale{4'294'967'296.0};

  if (!(relative > 0.0F)) {
    return 0U;
  }

  double const scaled_ticket = static_cast<double>(relative) * k_ticket_scale;

  if (scaled_ticket >=
      static_cast<double>(std::numeric_limits<std::uint32_t>::max())) {
    return std::numeric_limits<std::uint32_t>::max();
  }

  return static_cast<std::uint32_t>(scaled_ticket);
}

[[nodiscard]] constexpr auto
ScaleRadioactiveTimeTicket(std::uint64_t window_width_ps,
                           std::uint32_t ticket) noexcept -> std::uint64_t {
  std::uint64_t const width_upper = window_width_ps >> 32U;
  std::uint64_t const width_lower =
      window_width_ps & std::uint64_t{0xFFFF'FFFFULL};
  std::uint64_t const ticket_u64 = ticket;

  return (width_upper * ticket_u64) + ((width_lower * ticket_u64) >> 32U);
}

[[nodiscard]] inline auto
SampleRadioactiveTimeFromRaw(std::uint64_t time_start_ps,
                             std::uint64_t time_stop_ps, float scaled_decay,
                             std::uint32_t raw_word) noexcept -> std::uint64_t {
  if (time_stop_ps <= time_start_ps) {
    return time_start_ps;
  }

  float const uniform = static_cast<float>(raw_word >> 8U) * 0x1.0p-24F;
  float const relative = ComputeRadioactiveTimeRelative(uniform, scaled_decay);
  std::uint32_t const ticket = QuantizeRadioactiveTimeRelative(relative);
  std::uint64_t const window_width_ps = time_stop_ps - time_start_ps;
  std::uint64_t offset_ps = ScaleRadioactiveTimeTicket(window_width_ps, ticket);

  if (offset_ps >= window_width_ps) {
    offset_ps = window_width_ps - 1ULL;
  }

  return time_start_ps + offset_ps;
}
} // namespace ggems::core::radioactivity
