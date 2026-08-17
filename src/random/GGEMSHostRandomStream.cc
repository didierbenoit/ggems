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
 * \brief Implements the host-side GGEMS random stream.
 *
 * Implements JKISS, PCG32, and Philox state progression and scalar uniform sampling on the host.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstdint>
#include <span>

/// \endcond
#include "GGEMS/GGEMSException.hh"

#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomState.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"

namespace ggems::core::random {
namespace {

// =============================================================================
// =============================================================================

/*! \brief First Philox 4x32 multiplication constant. */
constexpr std::uint32_t k_philox_m4x32_0{0xD2511F53U};
/*! \brief Second Philox 4x32 multiplication constant. */
constexpr std::uint32_t k_philox_m4x32_1{0xCD9E8D57U};
/*! \brief First Philox key-bump constant. */
constexpr std::uint32_t k_philox_w32_0{0x9E3779B9U};
/*! \brief Second Philox key-bump constant. */
constexpr std::uint32_t k_philox_w32_1{0xBB67AE85U};

// =============================================================================
// =============================================================================

/*!
 * \brief Initializes one concrete host stream state through GGEMSRandom.
 * \tparam State Random-engine state type.
 * \param[in] random Random configuration.
 * \param[in] stream_id Logical stream identifier.
 * \return Initialized state.
 */
template <typename State>
auto InitializeState(GGEMSRandom const &random, std::uint64_t stream_id)
    -> State {
  State state{};
  random.InitializeStates(stream_id,
                          std::as_writable_bytes(std::span<State>{&state, 1U}));
  return state;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Advances one JKISS state and returns the next raw word.
 * \param[in,out] state JKISS stream state.
 * \return Next 32-bit JKISS output.
 */
auto NextJKiss(GGEMSJKissState &state) noexcept -> std::uint32_t {
  std::uint32_t y = state.y;

  y ^= y << 5U;
  y ^= y >> 7U;
  y ^= y << 22U;

  state.y = y;

  std::uint32_t const t = state.z + state.w + state.c;

  state.z = state.w;
  state.c = t >> 31U;
  state.w = t & 2'147'483'647U;
  state.x += 1'411'392'427U;

  return state.x + state.y + state.w;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Advances one PCG32 state and returns the next raw word.
 * \param[in,out] state PCG32 stream state.
 * \return Next 32-bit PCG32 output.
 */
auto NextPCG32(GGEMSPCG32State &state) noexcept -> std::uint32_t {
  std::uint64_t const old_state = state.state;

  state.state = (old_state * 6'364'136'223'846'793'005ULL) + state.increment;

  auto const xorshifted =
      static_cast<std::uint32_t>(((old_state >> 18U) ^ old_state) >> 27U);
  auto const rotation = static_cast<std::uint32_t>(old_state >> 59U);

  return (xorshifted >> rotation) | (xorshifted << ((0U - rotation) & 31U));
}

// =============================================================================
// =============================================================================

/*!
 * \brief Returns the high 32 bits of a 32-by-32-bit unsigned product.
 * \param[in] lhs Left factor.
 * \param[in] rhs Right factor.
 * \return High 32 bits of the 64-bit product.
 */
auto MultiplyHigh32(std::uint32_t lhs, std::uint32_t rhs) noexcept
    -> std::uint32_t {
  return static_cast<std::uint32_t>(
      (static_cast<std::uint64_t>(lhs) * static_cast<std::uint64_t>(rhs)) >>
      32U);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Applies one Philox 4x32 round to a counter and key.
 * \param[in] counter Four-word Philox counter.
 * \param[in] key Two-word Philox key.
 * \return Counter transformed by one Philox round.
 */
auto PhiloxRound(std::array<std::uint32_t, 4> const &counter,
                 std::array<std::uint32_t, 2> const &key) noexcept
    -> std::array<std::uint32_t, 4> {
  std::uint32_t const lo_0 = k_philox_m4x32_0 * counter[0];
  std::uint32_t const hi_0 = MultiplyHigh32(k_philox_m4x32_0, counter[0]);
  std::uint32_t const lo_1 = k_philox_m4x32_1 * counter[2];
  std::uint32_t const hi_1 = MultiplyHigh32(k_philox_m4x32_1, counter[2]);

  return {hi_1 ^ counter[1] ^ key[0], lo_1, hi_0 ^ counter[3] ^ key[1], lo_0};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Generates the next scalar Philox output and advances the host counter.
 * \param[in,out] state Philox stream state.
 * \return First 32-bit word of the generated Philox block.
 */
auto NextPhilox(GGEMSPhiloxState &state) noexcept -> std::uint32_t {
  std::array<std::uint32_t, 4> counter{state.counter_0, state.counter_1,
                                       state.counter_2, state.counter_3};
  std::array<std::uint32_t, 2> key{state.key_0, state.key_1};

  for (std::uint32_t round = 0U; round < 10U; ++round) {
    counter = PhiloxRound(counter, key);
    key[0] += k_philox_w32_0;
    key[1] += k_philox_w32_1;
  }

  ++state.counter_0;
  if (state.counter_0 == 0U) {
    ++state.counter_1;
  }

  return counter[0];
}
} // namespace

// =============================================================================
// =============================================================================

GGEMSHostRandomStream::GGEMSHostRandomStream(GGEMSRandom const &random,
                                             std::uint64_t stream_id)
    : engine_{random.GetEngine()}, stream_id_{stream_id},
      state_{GGEMSJKissState{}} {
  switch (engine_) {
  case GGEMSRandomEngine::JKISS:
    state_.emplace<GGEMSJKissState>(
        InitializeState<GGEMSJKissState>(random, stream_id_));
    return;
  case GGEMSRandomEngine::PCG32:
    state_.emplace<GGEMSPCG32State>(
        InitializeState<GGEMSPCG32State>(random, stream_id_));
    return;
  case GGEMSRandomEngine::Philox:
    state_.emplace<GGEMSPhiloxState>(
        InitializeState<GGEMSPhiloxState>(random, stream_id_));
    return;
  }

  throw ggems::core::GGEMSInternal(
      "Unsupported GGEMS random engine for host stream.");
}

// -----------------------------------------------------------------------------

auto GGEMSHostRandomStream::GetEngine() const noexcept -> GGEMSRandomEngine {
  return engine_;
}

// -----------------------------------------------------------------------------

auto GGEMSHostRandomStream::GetStreamId() const noexcept -> std::uint64_t {
  return stream_id_;
}

// -----------------------------------------------------------------------------

auto GGEMSHostRandomStream::NextUInt32() noexcept -> std::uint32_t {
  switch (engine_) {
  case GGEMSRandomEngine::JKISS:
    return NextJKiss(std::get<GGEMSJKissState>(state_));
  case GGEMSRandomEngine::PCG32:
    return NextPCG32(std::get<GGEMSPCG32State>(state_));
  case GGEMSRandomEngine::Philox:
    return NextPhilox(std::get<GGEMSPhiloxState>(state_));
  }

  return 0U;
}

// -----------------------------------------------------------------------------

auto GGEMSHostRandomStream::UniformFloat01() noexcept -> float {
  constexpr float k_uint32_to_float{5.9604644775390625e-8F};
  return static_cast<float>(NextUInt32() >> 8U) * k_uint32_to_float;
}

// -----------------------------------------------------------------------------

auto GGEMSHostRandomStream::UniformDoubleOpen01() noexcept -> double {
  auto const high_bits = static_cast<std::uint64_t>(NextUInt32() >> 5U);
  auto const low_bits = static_cast<std::uint64_t>(NextUInt32() >> 6U);
  std::uint64_t const bits = (high_bits << 26U) + low_bits;

  double const uniform = (static_cast<double>(bits) + 0.5) * 0x1.0p-53;

  // The top midpoint rounds to 1.0 in binary64; keep the contract open.
  constexpr double k_largest_open_unit{0x1.fffffffffffffp-1};
  return uniform < 1.0 ? uniform : k_largest_open_unit;
}

} // namespace ggems::core::random
