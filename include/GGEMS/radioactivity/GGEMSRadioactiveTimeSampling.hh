// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Defines finite-precision radioactive birth-time sampling.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

namespace ggems::core::radioactivity {

/*!
 * \brief Maps a uniform variate to a relative truncated exponential time.
 *
 * The ideal inverse is \f$r=-\log(1-u(1-e^{-d}))/d\f$, where
 * \f$u\in[0,1)\f$, \f$d=\lambda\Delta t\f$ is dimensionless scaled decay,
 * and \f$r\f$ is the fraction of the emission window. The implementation uses
 * the uniform limit for d <= 2^-14, a second-order expansion through d = 0.01,
 * and expm1/log1p otherwise, all in binary32.
 *
 * \param[in] uniform Uniform variate in [0, 1).
 * \param[in] scaled_decay Finite nonnegative decay constant times window width.
 * \return Approximate relative emission time before integer quantization.
 */
[[nodiscard]] inline auto
ComputeRadioactiveTimeRelative(float uniform, float scaled_decay) noexcept
  -> float {
  if (scaled_decay <= 0x1.0p-14F) {
    return uniform;
  }

  if (scaled_decay <= 0.01F) {
    float const uu = uniform * (uniform - 1.0F);

    return uniform + (0.5F * scaled_decay * uu) +
           (1.0F / 6.0F) * scaled_decay * (scaled_decay * uu) *
             ((2.0F * uniform) - 1.0F);
  }

  float const decay_mass = -std::expm1(-scaled_decay);
  return -std::log1p(-uniform * decay_mass) / scaled_decay;
}

/*!
 * \brief Converts a relative time to a bounded 32-bit fixed-point ticket.
 * \param[in] relative Relative time, normally in [0, 1].
 * \return Truncated relative * 2^32, bounded to [0, 2^32 - 1]; values that fail
 * the positive comparison map to zero.
 */
[[nodiscard]] inline auto
QuantizeRadioactiveTimeRelative(float relative) noexcept -> std::uint32_t {
  if (!(relative > 0.0F)) {
    return 0U;
  }

  double const scaled_ticket = static_cast<double>(relative) * 4'294'967'296.0;

  if (scaled_ticket >=
      static_cast<double>(std::numeric_limits<std::uint32_t>::max())) {
    return std::numeric_limits<std::uint32_t>::max();
  }

  return static_cast<std::uint32_t>(scaled_ticket);
}

/*!
 * \brief Scales a fixed-point ticket to an integer picosecond offset.
 * \param[in] window_width_ps Window width in ps.
 * \param[in] ticket Unsigned fraction with denominator 2^32.
 * \return Exact floor of window_width_ps * ticket / 2^32, computed by splitting
 * the width into two words to avoid a 128-bit product.
 */
[[nodiscard]] constexpr auto
ScaleRadioactiveTimeTicket(std::uint64_t window_width_ps,
                           std::uint32_t ticket) noexcept -> std::uint64_t {
  std::uint64_t const width_upper = window_width_ps >> 32U;
  std::uint64_t const width_lower =
    window_width_ps & std::uint64_t{0xFFFF'FFFFULL};
  std::uint64_t const ticket_u64 = ticket;

  return (width_upper * ticket_u64) + ((width_lower * ticket_u64) >> 32U);
}

/*!
 * \brief Samples an integer birth time within a half-open ps interval.
 *
 * Uses the high 24 bits of raw_word for the binary32 uniform variate, applies
 * the truncated exponential inverse approximation, then quantizes through a
 * 32-bit ticket. This finite sampling grid is distinct from a continuous
 * exponential distribution.
 *
 * \param[in] time_start_ps Inclusive window start in ps.
 * \param[in] time_stop_ps Exclusive window stop in ps.
 * \param[in] scaled_decay Finite nonnegative decay constant times window width.
 * \param[in] raw_word Uniform 32-bit random word.
 * \return Quantized time in [start, stop), or start when stop <= start.
 */
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
  std::uint64_t const offset_ps =
    ScaleRadioactiveTimeTicket(window_width_ps, ticket);

  return time_start_ps + offset_ps;
}
} // namespace ggems::core::radioactivity
