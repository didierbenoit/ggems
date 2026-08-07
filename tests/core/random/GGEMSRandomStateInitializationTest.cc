#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"

namespace {

using ggems::core::random::GGEMSJKissState;
using ggems::core::random::GGEMSPCG32State;
using ggems::core::random::GGEMSPhiloxState;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;

// =============================================================================
// =============================================================================

std::vector<std::byte> InitializeStateBytes(GGEMSRandom const &random,
                                            std::uint64_t first_stream_id,
                                            std::size_t state_count) {
  std::vector<std::byte> storage(state_count * random.GetStateSize());

  random.InitializeStates(first_stream_id,
                          std::span<std::byte>{storage.data(), storage.size()});

  return storage;
}

// =============================================================================
// =============================================================================

template <typename State>
State ReadState(std::vector<std::byte> const &storage,
                std::size_t state_index = 0U) {
  State state{};

  std::memcpy(&state, storage.data() + state_index * sizeof(State),
              sizeof(State));

  return state;
}

// =============================================================================
// =============================================================================

class GGEMSRandomStateInitializationTest
    : public ::testing::TestWithParam<GGEMSRandomEngine> {};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSRandomStateInitializationContractTest,
     EngineIdsAndStateSizesMatchKernelContracts) {
  GGEMSRandom random{};

  random.SetEngine(GGEMSRandomEngine::JKISS);
  EXPECT_EQ(random.GetKernelEngineId(), 1U);
  EXPECT_EQ(random.GetStateSize(), sizeof(GGEMSJKissState));

  random.SetEngine(GGEMSRandomEngine::PCG32);
  EXPECT_EQ(random.GetKernelEngineId(), 2U);
  EXPECT_EQ(random.GetStateSize(), sizeof(GGEMSPCG32State));

  random.SetEngine(GGEMSRandomEngine::Philox);
  EXPECT_EQ(random.GetKernelEngineId(), 3U);
  EXPECT_EQ(random.GetStateSize(), sizeof(GGEMSPhiloxState));
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomStateInitializationContractTest,
     ExistingInitialStateContractsArePreserved) {
  GGEMSRandom random{};
  random.SetSeed(77'777ULL);

  random.SetEngine(GGEMSRandomEngine::JKISS);
  auto jkiss =
      ReadState<GGEMSJKissState>(InitializeStateBytes(random, 42ULL, 1U));

  EXPECT_EQ(jkiss.x, 4'052'806'268U);
  EXPECT_EQ(jkiss.y, 432'290'774U);
  EXPECT_EQ(jkiss.z, 524'267'304U);
  EXPECT_EQ(jkiss.w, 1'041'811'508U);
  EXPECT_EQ(jkiss.c, 0U);

  random.SetEngine(GGEMSRandomEngine::PCG32);
  auto const pcg32 =
      ReadState<GGEMSPCG32State>(InitializeStateBytes(random, 42ULL, 1U));

  EXPECT_EQ(pcg32.state, 0x2AFC81E9C4CF0395ULL);
  EXPECT_EQ(pcg32.increment, 0xE5E985F73D706249ULL);

  random.SetEngine(GGEMSRandomEngine::Philox);
  auto const philox =
      ReadState<GGEMSPhiloxState>(InitializeStateBytes(random, 42ULL, 1U));

  EXPECT_EQ(philox.counter_0, 0U);
  EXPECT_EQ(philox.counter_1, 0U);
  EXPECT_EQ(philox.counter_2, 42U);
  EXPECT_EQ(philox.counter_3, 0U);
  EXPECT_EQ(philox.key_0, 2'434'675'590U);
  EXPECT_EQ(philox.key_1, 3'228'916'122U);
}

// =============================================================================
// =============================================================================

TEST_P(GGEMSRandomStateInitializationTest,
       SameSeedAndStreamProduceIdenticalStateBytes) {
  GGEMSRandom random{};
  random.SetEngine(GetParam()).SetSeed(77'777ULL);

  auto first = InitializeStateBytes(random, 42ULL, 1U);
  auto second = InitializeStateBytes(random, 42ULL, 1U);

  EXPECT_EQ(first, second);
}

// =============================================================================
// =============================================================================

TEST_P(GGEMSRandomStateInitializationTest,
       DifferentStreamsProduceDifferentStateBytes) {
  GGEMSRandom random{};
  random.SetEngine(GetParam()).SetSeed(77'777ULL);

  auto first = InitializeStateBytes(random, 42ULL, 1U);
  auto second = InitializeStateBytes(random, 43ULL, 1U);

  EXPECT_NE(first, second);
}

// =============================================================================
// =============================================================================

TEST_P(GGEMSRandomStateInitializationTest,
       DifferentSeedsProduceDifferentStateBytes) {
  GGEMSRandom random{};
  random.SetEngine(GetParam()).SetSeed(77'777ULL);
  auto first = InitializeStateBytes(random, 42ULL, 1U);

  random.SetSeed(77'778ULL);
  auto second = InitializeStateBytes(random, 42ULL, 1U);

  EXPECT_NE(first, second);
}

// =============================================================================
// =============================================================================

TEST_P(GGEMSRandomStateInitializationTest,
       BatchInitializationMatchesIndividualStates) {
  constexpr std::uint64_t k_first_stream_id{123ULL};
  constexpr std::size_t k_state_count{4U};

  GGEMSRandom random{};
  random.SetEngine(GetParam()).SetSeed(77'777ULL);

  auto batch = InitializeStateBytes(random, k_first_stream_id, k_state_count);

  std::size_t state_size = random.GetStateSize();

  for (std::size_t index = 0U; index < k_state_count; ++index) {
    auto individual = InitializeStateBytes(
        random, k_first_stream_id + static_cast<std::uint64_t>(index), 1U);

    auto batch_begin =
        batch.begin() + static_cast<std::ptrdiff_t>(index * state_size);

    EXPECT_TRUE(std::equal(individual.begin(), individual.end(), batch_begin));
  }
}

// =============================================================================
// =============================================================================

TEST_P(GGEMSRandomStateInitializationTest,
       AcceptsBoundaryStreamRangeWithoutOverflow) {
  GGEMSRandom random{};
  random.SetEngine(GetParam()).SetSeed(77'777ULL);

  EXPECT_NO_THROW((void)InitializeStateBytes(random, 0ULL, 1U));

  std::uint64_t first_stream_id =
      GetParam() == GGEMSRandomEngine::JKISS
          ? static_cast<std::uint64_t>(
                std::numeric_limits<std::uint32_t>::max()) -
                3ULL
          : std::numeric_limits<std::uint64_t>::max() - 3ULL;

  EXPECT_NO_THROW((void)InitializeStateBytes(random, first_stream_id, 4U));
}

// =============================================================================
// =============================================================================

TEST_P(GGEMSRandomStateInitializationTest,
       RejectsOverflowingStreamRangeBeforeWriting) {
  GGEMSRandom random{};
  random.SetEngine(GetParam()).SetSeed(77'777ULL);

  std::array<std::byte, 3U * sizeof(GGEMSPhiloxState)> storage{};
  std::fill(storage.begin(), storage.end(), std::byte{0x5A});
  auto before = storage;

  std::span<std::byte> selected_storage{storage.data(),
                                        2U * random.GetStateSize()};

  EXPECT_THROW(random.InitializeStates(
                   std::numeric_limits<std::uint64_t>::max(), selected_storage),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(storage, before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomStateInitializationContractTest,
     JKISSRejectsStreamIdentifiersOutsideUInt32) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(77'777ULL);

  auto storage = std::vector<std::byte>(random.GetStateSize(), std::byte{0x5A});
  auto before = storage;

  EXPECT_THROW(random.InitializeStates(
                   static_cast<std::uint64_t>(
                       std::numeric_limits<std::uint32_t>::max()) +
                       1ULL,
                   std::span<std::byte>{storage.data(), storage.size()}),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(storage, before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomStateInitializationContractTest,
     JKISSUsesTheLow32BitsOfTheSeed) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(23ULL);
  auto low_seed = InitializeStateBytes(random, 42ULL, 1U);

  random.SetSeed((1ULL << 32U) + 23ULL);
  auto high_seed = InitializeStateBytes(random, 42ULL, 1U);

  EXPECT_EQ(low_seed, high_seed);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomStateInitializationContractTest,
     RejectsStorageWithPartialStateBeforeWriting) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(77'777ULL);

  std::array<std::byte, sizeof(GGEMSPCG32State) + 1U> storage{};
  std::fill(storage.begin(), storage.end(), std::byte{0x5A});
  auto before = storage;

  EXPECT_THROW(random.InitializeStates(
                   0ULL, std::span<std::byte>{storage.data(), storage.size()}),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(storage, before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomStateInitializationContractTest,
     RejectsInvalidEngineBeforeWriting) {
  GGEMSRandom random{};
  random.SetEngine(static_cast<GGEMSRandomEngine>(0U));

  std::array<std::byte, sizeof(GGEMSPhiloxState)> storage{};
  std::fill(storage.begin(), storage.end(), std::byte{0x5A});
  auto before = storage;

  EXPECT_THROW(random.InitializeStates(
                   0ULL, std::span<std::byte>{storage.data(), storage.size()}),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(storage, before);
}

// =============================================================================
// =============================================================================

INSTANTIATE_TEST_SUITE_P(AllEngines, GGEMSRandomStateInitializationTest,
                         ::testing::Values(GGEMSRandomEngine::JKISS,
                                           GGEMSRandomEngine::PCG32,
                                           GGEMSRandomEngine::Philox));
