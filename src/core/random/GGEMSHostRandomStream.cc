#include <array>
#include <cstdint>
#include <span>

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"

namespace ggems::core::random {
namespace {

// =============================================================================
// =============================================================================

constexpr std::uint32_t k_philox_m4x32_0{0xD2511F53U};
constexpr std::uint32_t k_philox_m4x32_1{0xCD9E8D57U};
constexpr std::uint32_t k_philox_w32_0{0x9E3779B9U};
constexpr std::uint32_t k_philox_w32_1{0xBB67AE85U};

// =============================================================================
// =============================================================================

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

auto NextPCG32(GGEMSPCG32State &state) noexcept -> std::uint32_t {
  std::uint64_t const old_state = state.state;

  state.state = old_state * 6'364'136'223'846'793'005ULL + state.increment;

  auto const xorshifted =
      static_cast<std::uint32_t>(((old_state >> 18U) ^ old_state) >> 27U);
  auto const rotation = static_cast<std::uint32_t>(old_state >> 59U);

  return (xorshifted >> rotation) | (xorshifted << ((0U - rotation) & 31U));
}

// =============================================================================
// =============================================================================

auto MultiplyHigh32(std::uint32_t lhs, std::uint32_t rhs) noexcept
    -> std::uint32_t {
  return static_cast<std::uint32_t>(
      (static_cast<std::uint64_t>(lhs) * static_cast<std::uint64_t>(rhs)) >>
      32U);
}

// =============================================================================
// =============================================================================

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

  throw ggems::core::GGEMSInternal("Unsupported GGEMS random engine for host stream.");
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
  auto const a = static_cast<std::uint64_t>(NextUInt32() >> 5U);
  auto const b = static_cast<std::uint64_t>(NextUInt32() >> 6U);
  std::uint64_t const bits = (a << 26U) + b;

  double const uniform = (static_cast<double>(bits) + 0.5) * 0x1.0p-53;

  // The top midpoint rounds to 1.0 in binary64; keep the contract open.
  constexpr double k_largest_open_unit{0x1.fffffffffffffp-1};
  return uniform < 1.0 ? uniform : k_largest_open_unit;
}

} // namespace ggems::core::random
