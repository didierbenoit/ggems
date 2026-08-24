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
 * \brief Implements GGEMS random-engine configuration and state initialization.
 *
 * Implements engine-name parsing, deterministic stream-state construction,
 * range validation, and OpenCL-facing engine metadata.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cctype>
#include <cstddef>
#include <cstring>
#include <format>
#include <limits>
#include <span>
#include <vector>
#include <utility>
#include <string>
#include <cstdint>
#include <string_view>

/// \endcond
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomState.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"

namespace ggems::core::random {
namespace {

/*!
 * \brief Normalizes a random-engine name for case-insensitive alias matching.
 * \param[in] engine_name Name supplied by the caller.
 * \return Lowercase name with separators removed.
 */
auto NormalizeEngineName(std::string_view engine_name) -> std::string {
  std::string normalized;
  normalized.reserve(engine_name.size());

  for (char character : engine_name) {
    if (character == '_' || character == '-' || character == ' ') {
      continue;
    }

    normalized.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
  }

  return normalized;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Applies one SplitMix64 mixing step used during state construction.
 * \param[in] value Input word.
 * \return Mixed 64-bit word.
 */
auto SplitMix64(std::uint64_t value) noexcept -> std::uint64_t {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Constructs the deterministic initial state of one JKISS stream.
 * \param[in] seed 32-bit configured seed.
 * \param[in] stream_id 32-bit logical stream identifier.
 * \return Initialized JKISS state.
 */
auto MakeJKissState(std::uint32_t seed,
                    std::uint32_t stream_id) noexcept -> GGEMSJKissState {
  return GGEMSJKissState{.x = seed + 123456789U + (1013904223U * stream_id),
                         .y = seed ^ (362436069U + (1664525U * stream_id)),
                         .z = seed + 521288629U + (69069U * stream_id),
                         .w = seed ^ (88675123U + (22695477U * stream_id)),
                         .c = stream_id & 1U};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Constructs the deterministic initial state of one PCG32 stream.
 * \param[in] seed Configured seed.
 * \param[in] stream_id Logical stream identifier.
 * \return Initialized PCG32 state with an odd stream increment.
 */
auto MakePCG32State(std::uint64_t seed,
                    std::uint64_t stream_id) noexcept -> GGEMSPCG32State {
  std::uint64_t state =
      SplitMix64(seed + (0xD1B54A32D192ED03ULL * (stream_id + 1ULL)));

  std::uint64_t stream = SplitMix64(seed ^ (0xABC98388FB8FAC03ULL + stream_id));

  return GGEMSPCG32State{.state = state, .increment = stream | 1ULL};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Constructs the deterministic initial state of one Philox stream.
 * \param[in] seed Configured seed used to derive the Philox key.
 * \param[in] stream_id Logical stream identifier encoded in the high counter
 * words. \return Initialized Philox state.
 */
auto MakePhiloxState(std::uint64_t seed,
                     std::uint64_t stream_id) noexcept -> GGEMSPhiloxState {
  std::uint64_t key = SplitMix64(seed);

  return GGEMSPhiloxState{.counter_0 = 0U,
                          .counter_1 = 0U,
                          .counter_2 = static_cast<std::uint32_t>(stream_id),
                          .counter_3 =
                              static_cast<std::uint32_t>(stream_id >> 32U),
                          .key_0 = static_cast<std::uint32_t>(key),
                          .key_1 = static_cast<std::uint32_t>(key >> 32U)};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Validates state-storage sizing and returns the represented state
 * count. \param[in] state_size Selected engine state size in bytes. \param[in]
 * state_storage Raw state storage. \return Number of complete states
 * represented by the storage. \throws ggems::core::GGEMSRecoverable If the
 * storage size is not an exact multiple of the state size. \throws
 * ggems::core::GGEMSInternal If the state size is zero.
 */
auto CheckedStateCount(std::size_t state_size,
                       std::span<std::byte> state_storage) -> std::size_t {
  if (!(state_size > 0U)) {
    throw ggems::core::GGEMSInternal(
        "Unsupported GGEMS Random engine state size.");
  }

  if (!(state_storage.size() % state_size == 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Random state storage size must be a multiple of the "
        "selected engine state size.");
  }

  return state_storage.size() / state_size;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Computes the last logical stream identifier of a contiguous state
 * range. \param[in] first_stream_id First stream identifier. \param[in]
 * state_count Number of states in the range. \return Last stream identifier, or
 * \p first_stream_id for an empty range. \throws ggems::core::GGEMSRecoverable
 * If the identifier range overflows uint64_t.
 */
auto CheckLastStreamId(std::uint64_t first_stream_id,
                       std::size_t state_count) -> std::uint64_t {
  if (state_count == 0U) {
    return first_stream_id;
  }

  if (!(std::cmp_less_equal(state_count - 1U,
                            std::numeric_limits<std::uint64_t>::max() -
                                first_stream_id))) {
    throw ggems::core::GGEMSRecoverable(
        "Random stream identifier range overflow uint64_t.");
  }

  return first_stream_id + static_cast<std::uint64_t>(state_count - 1U);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Initializes a sequence of engine states into raw byte storage.
 * \tparam State Concrete random-engine state type.
 * \tparam Factory Callable that creates a state for a logical stream
 * identifier. \param[in] first_stream_id First stream identifier. \param[in]
 * state_count Number of states to initialize. \param[out] state_storage
 * Destination byte storage. \param[in] make_state State-construction callable.
 */
template <typename State, typename Factory>
auto InitializeStateStorage(
    std::uint64_t first_stream_id, std::size_t state_count,
    std::span<std::byte> state_storage,
    Factory make_state) noexcept(noexcept(make_state(first_stream_id)))
    -> void {
  for (std::size_t state_index = 0U; state_index < state_count; ++state_index) {
    auto state =
        make_state(first_stream_id + static_cast<std::uint64_t>(state_index));

    std::memcpy(state_storage.data() + (state_index * sizeof(State)), &state,
                sizeof(State));
  }
}

} // namespace

// =============================================================================
// =============================================================================

auto ToString(GGEMSRandomEngine engine) -> std::string {
  switch (engine) {
  case GGEMSRandomEngine::JKISS:
    return "JKISS";
  case GGEMSRandomEngine::PCG32:
    return "PCG32";
  case GGEMSRandomEngine::Philox:
    return "Philox";
  }

  throw ggems::core::GGEMSInternal("Unsupported GGEMS random engine.");
}

// =============================================================================
// =============================================================================

auto ParseRandomEngine(std::string_view engine_name) -> GGEMSRandomEngine {
  std::string normalized = NormalizeEngineName(engine_name);

  if (normalized == "jkiss" || normalized == "kiss") {
    return GGEMSRandomEngine::JKISS;
  }

  if (normalized == "pcg32" || normalized == "pcg") {
    return GGEMSRandomEngine::PCG32;
  }

  if (normalized == "philox") {
    return GGEMSRandomEngine::Philox;
  }

  throw ggems::core::GGEMSRecoverable(
      std::format("Unsupported GGEMS random engine '{}'.", engine_name));
}

// =============================================================================
// =============================================================================

auto ToKernelEngineId(GGEMSRandomEngine engine) noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(engine);
}

// =============================================================================
// =============================================================================

GGEMSRandom::GGEMSRandom() {
  GGEMS_INFOEX("Random", 3, "GGEMSRandom instance created.");
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::SetEngine(GGEMSRandomEngine engine) noexcept
    -> GGEMSRandom & {
  engine_ = engine;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::SetEngine(std::string_view engine_name) -> GGEMSRandom & {
  engine_ = ParseRandomEngine(engine_name);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::GetEngine() const noexcept -> GGEMSRandomEngine {
  return engine_;
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::GetEngineName() const -> std::string {
  return ToString(engine_);
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::SetSeed(std::uint64_t seed) noexcept -> GGEMSRandom & {
  seed_ = seed;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::GetSeed() const noexcept -> std::uint64_t { return seed_; }

// -----------------------------------------------------------------------------

auto GGEMSRandom::GetKernelEngineId() const noexcept -> std::uint32_t {
  return ToKernelEngineId(engine_);
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::GetKernelBuildDefinition() const -> std::string {
  return std::format("-DGGEMS_RANDOM_ENGINE={}", GetKernelEngineId());
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::GetStateSize() const noexcept -> std::size_t {
  switch (engine_) {
  case GGEMSRandomEngine::JKISS:
    return sizeof(GGEMSJKissState);
  case GGEMSRandomEngine::PCG32:
    return sizeof(GGEMSPCG32State);
  case GGEMSRandomEngine::Philox:
    return sizeof(GGEMSPhiloxState);
  }

  return 0U;
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::ValidateStateRange(std::uint64_t first_stream_id,
                                     std::size_t state_count) const -> void {
  if (!(GetStateSize() > 0U)) {
    throw ggems::core::GGEMSInternal(
        "Unsupported GGEMS random engine state size.");
  }

  std::uint64_t last_stream_id =
      CheckLastStreamId(first_stream_id, state_count);

  if (engine_ == GGEMSRandomEngine::JKISS) {
    if (state_count != 0U &&
        last_stream_id > std::numeric_limits<std::uint32_t>::max()) {
      throw ggems::core::GGEMSRecoverable(
          "JKISS stream identifier must fit uint32_t.");
    }
  }
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::InitializeStates(std::uint64_t first_stream_id,
                                   std::span<std::byte> state_storage) const
    -> void {
  std::size_t state_count = CheckedStateCount(GetStateSize(), state_storage);

  ValidateStateRange(first_stream_id, state_count);

  switch (engine_) {
  case GGEMSRandomEngine::JKISS: {
    auto seed = static_cast<std::uint32_t>(seed_);

    InitializeStateStorage<GGEMSJKissState>(
        first_stream_id, state_count, state_storage,
        [seed](std::uint64_t stream_id) noexcept -> GGEMSJKissState {
          return MakeJKissState(seed, static_cast<std::uint32_t>(stream_id));
        });
    return;
  }

  case GGEMSRandomEngine::PCG32:
    InitializeStateStorage<GGEMSPCG32State>(
        first_stream_id, state_count, state_storage,
        [seed = seed_](std::uint64_t stream_id) noexcept -> GGEMSPCG32State {
          return MakePCG32State(seed, stream_id);
        });
    return;

  case GGEMSRandomEngine::Philox:
    InitializeStateStorage<GGEMSPhiloxState>(
        first_stream_id, state_count, state_storage,
        [seed = seed_](std::uint64_t stream_id) noexcept -> GGEMSPhiloxState {
          return MakePhiloxState(seed, stream_id);
        });
    return;
  }

  throw ggems::core::GGEMSInternal(
      "Unsupported GGEMS random engine state initialization.");
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::BuildSummaryLines() const -> std::vector<std::string> {
  return {
      std::format("Random engine           : {}", GetEngineName()),
      std::format("Seed                    : {}", seed_),
      std::format("State size              : {} bytes", GetStateSize()),
      std::format("OpenCL engine id        : {}", GetKernelEngineId()),
      std::format("OpenCL build definition : {}", GetKernelBuildDefinition()),
      "kernel raw API         : GGEMS_RndmUInt32",
      "kernel scalar API      : GGEMS_RndmUniform",
      "kernel vector API      : GGEMS_RndmUniform4"};
}

// -----------------------------------------------------------------------------

auto GGEMSRandom::Verbose() const -> void {
  for (std::string const &line : BuildSummaryLines()) {
    GGEMS_INFO("Random", "{}", line);
  }
}

} // namespace ggems::core::random
