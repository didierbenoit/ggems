#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <format>
#include <limits>
#include <span>
#include <utility>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"

namespace ggems::core::random {
namespace {

std::string NormaliseEngineName(std::string_view engine_name) {
  std::string normalised;
  normalised.reserve(engine_name.size());

  for (char character : engine_name) {
    if (character == '_' || character == '-' || character == ' ') {
      continue;
    }

    normalised.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
  }

  return normalised;
}

// =============================================================================
// =============================================================================

std::uint64_t SplitMix64(std::uint64_t value) noexcept {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

// =============================================================================
// =============================================================================

GGEMSJKissState MakeJKissState(std::uint32_t seed,
                               std::uint32_t stream_id) noexcept {
  return GGEMSJKissState{.x = seed + 123456789U + 1013904223U * stream_id,
                         .y = seed ^ (362436069U + 1664525U * stream_id),
                         .z = seed + 521288629U + 69069U * stream_id,
                         .w = seed ^ (88675123U + 22695477U * stream_id),
                         .c = stream_id & 1U};
}

// =============================================================================
// =============================================================================

GGEMSPCG32State MakePCG32State(std::uint64_t seed,
                               std::uint64_t stream_id) noexcept {
  std::uint64_t state =
      SplitMix64(seed + 0xD1B54A32D192ED03ULL * (stream_id + 1ULL));

  std::uint64_t stream = SplitMix64(seed ^ (0xABC98388FB8FAC03ULL + stream_id));

  return GGEMSPCG32State{.state = state, .increment = stream | 1ULL};
}

// =============================================================================
// =============================================================================

GGEMSPhiloxState MakePhiloxState(std::uint64_t seed,
                                 std::uint64_t stream_id) noexcept {
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

std::size_t CheckedStateCount(std::size_t state_size,
                              std::span<std::byte> state_storage) {
  GGEMS_CHECK_INTERNAL(state_size > 0U,
                       "Unsupported GGEMS Random engine state size.");

  GGEMS_CHECK_RECOVERABLE(state_storage.size() % state_size == 0U,
                          "Random state storage size must be a multiple of the "
                          "selected engine state size.");

  return state_storage.size() / state_size;
}

// =============================================================================
// =============================================================================

std::uint64_t CheckLastStreamId(std::uint64_t first_stream_id,
                                std::size_t state_count) {
  if (state_count == 0U) {
    return first_stream_id;
  }

  GGEMS_CHECK_RECOVERABLE(
      std::cmp_less_equal(state_count - 1U,
                          std::numeric_limits<std::uint64_t>::max() -
                              first_stream_id),
      "Random stream identifier range overflow uint64_t.");

  return first_stream_id + static_cast<std::uint64_t>(state_count - 1U);
}

// =============================================================================
// =============================================================================

template <typename State, typename Factory>
void InitialiseStateStorage(std::uint64_t first_stream_id,
                            std::size_t state_count,
                            std::span<std::byte> state_storage,
                            Factory make_state) noexcept {
  for (std::size_t state_index = 0U; state_index < state_count; ++state_index) {
    auto state =
        make_state(first_stream_id + static_cast<std::uint64_t>(state_index));

    std::memcpy(state_storage.data() + state_index * sizeof(State), &state,
                sizeof(State));
  }
}

} // namespace

// =============================================================================
// =============================================================================

std::string ToString(GGEMSRandomEngine engine) {
  switch (engine) {
  case GGEMSRandomEngine::JKISS:
    return "JKISS";
  case GGEMSRandomEngine::PCG32:
    return "PCG32";
  case GGEMSRandomEngine::Philox:
    return "Philox";
  }

  GGEMS_INTERNAL("Unsupported GGEMS random engine.");
  return "Unknown";
}

// =============================================================================
// =============================================================================

GGEMSRandomEngine ParseRandomEngine(std::string_view engine_name) {
  std::string normalised = NormaliseEngineName(engine_name);

  if (normalised == "jkiss" || normalised == "kiss") {
    return GGEMSRandomEngine::JKISS;
  }

  if (normalised == "pcg32" || normalised == "pcg") {
    return GGEMSRandomEngine::PCG32;
  }

  if (normalised == "philox") {
    return GGEMSRandomEngine::Philox;
  }

  GGEMS_RECOVERABLE(
      std::format("Unsupported GGEMS random engine '{}'.", engine_name));

  return GGEMSRandomEngine::JKISS;
}

// =============================================================================
// =============================================================================

std::uint32_t ToKernelEngineId(GGEMSRandomEngine engine) noexcept {
  return static_cast<std::uint32_t>(engine);
}

// =============================================================================
// =============================================================================

GGEMSRandom::GGEMSRandom() {
  GGEMS_INFOEX("Random", 3, "GGEMSRandom instance created.");
}

// -----------------------------------------------------------------------------

GGEMSRandom &GGEMSRandom::SetEngine(GGEMSRandomEngine engine) noexcept {
  engine_ = engine;
  return *this;
}

// -----------------------------------------------------------------------------

GGEMSRandom &GGEMSRandom::SetEngine(std::string_view engine_name) {
  engine_ = ParseRandomEngine(engine_name);
  return *this;
}

// -----------------------------------------------------------------------------

GGEMSRandomEngine GGEMSRandom::GetEngine() const noexcept { return engine_; }

// -----------------------------------------------------------------------------

std::string GGEMSRandom::GetEngineName() const { return ToString(engine_); }

// -----------------------------------------------------------------------------

GGEMSRandom &GGEMSRandom::SetSeed(std::uint64_t seed) noexcept {
  seed_ = seed;
  return *this;
}

// -----------------------------------------------------------------------------

std::uint64_t GGEMSRandom::GetSeed() const noexcept { return seed_; }

// -----------------------------------------------------------------------------

std::uint32_t GGEMSRandom::GetKernelEngineId() const noexcept {
  return ToKernelEngineId(engine_);
}

// -----------------------------------------------------------------------------

std::string GGEMSRandom::GetKernelBuildDefinition() const {
  return std::format("-DGGEMS_RANDOM_ENGINE={}", GetKernelEngineId());
}

// -----------------------------------------------------------------------------

std::size_t GGEMSRandom::GetStateSize() const noexcept {
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

void GGEMSRandom::ValidateStateRange(std::uint64_t first_stream_id,
                                     std::size_t state_count) const {
  GGEMS_CHECK_INTERNAL(GetStateSize() > 0U,
                       "Unsupported GGEMS random engine state size.");

  std::uint64_t last_stream_id =
      CheckLastStreamId(first_stream_id, state_count);

  if (engine_ == GGEMSRandomEngine::JKISS) {
    GGEMS_CHECK_RECOVERABLE(state_count == 0U ||
                                last_stream_id <=
                                    std::numeric_limits<std::uint32_t>::max(),
                            "JKISS stream identifier must fit uint32_t.");
  }
}

// -----------------------------------------------------------------------------

void GGEMSRandom::InitialiseStates(std::uint64_t first_stream_id,
                                   std::span<std::byte> state_storage) const {
  std::size_t state_count = CheckedStateCount(GetStateSize(), state_storage);

  ValidateStateRange(first_stream_id, state_count);

  switch (engine_) {
  case GGEMSRandomEngine::JKISS: {
    std::uint32_t seed = static_cast<std::uint32_t>(seed_);

    InitialiseStateStorage<GGEMSJKissState>(
        first_stream_id, state_count, state_storage,
        [seed](std::uint64_t stream_id) noexcept {
          return MakeJKissState(seed, static_cast<std::uint32_t>(stream_id));
        });
    return;
  }

  case GGEMSRandomEngine::PCG32:
    InitialiseStateStorage<GGEMSPCG32State>(
        first_stream_id, state_count, state_storage,
        [seed = seed_](std::uint64_t stream_id) noexcept {
          return MakePCG32State(seed, stream_id);
        });
    return;

  case GGEMSRandomEngine::Philox:
    InitialiseStateStorage<GGEMSPhiloxState>(
        first_stream_id, state_count, state_storage,
        [seed = seed_](std::uint64_t stream_id) noexcept {
          return MakePhiloxState(seed, stream_id);
        });
    return;
  }

  GGEMS_INTERNAL("Unsupported GGEMS random engine state initialisation.");
}

// -----------------------------------------------------------------------------

std::vector<std::string> GGEMSRandom::BuildSummaryLines() const {
  return {
      std::format("Random engine           : {}", GetEngineName()),
      std::format("Seed                    : {}", seed_),
      std::format("State size              : {} bytes", GetStateSize()),
      std::format("OpenCL engine id        : {}", GetKernelEngineId()),
      std::format("OpenCL build definition : {}", GetKernelBuildDefinition()),
      "kernel scalar API      : GGEMS_RndmUniform",
      "kernel vector API      : GGEMS_RndmUniform4"};
}

// -----------------------------------------------------------------------------

void GGEMSRandom::Verbose() const {
  for (std::string &line : BuildSummaryLines()) {
    GGEMS_INFO("Random", "{}", line);
  }
}

} // namespace ggems::core::random
