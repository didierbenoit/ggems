#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSObserverCounterArithmetic.hh"

namespace {

using ggems::core::observer::detail::AddSaturatedObserverCounter;
using ggems::core::observer::detail::SaturateObserverCounter;

// =============================================================================
// =============================================================================

constexpr std::uint32_t k_uint32_maximum =
    std::numeric_limits<std::uint32_t>::max();
constexpr std::uint64_t k_uint64_maximum =
    std::numeric_limits<std::uint64_t>::max();

// =============================================================================
// =============================================================================

struct SaturationCase {
  char const *label;
  std::uint64_t value;
  std::uint32_t expected;
};

// =============================================================================
// =============================================================================

struct AdditionCase {
  char const *label;
  std::uint32_t destination;
  std::uint64_t value;
  std::uint32_t expected;
};

// =============================================================================
// =============================================================================

constexpr auto AdditionWorksAtCompileTime() noexcept -> bool {
  std::uint32_t destination{1U};
  AddSaturatedObserverCounter(destination, 2ULL);
  return destination == 3U;
}

static_assert(SaturateObserverCounter(k_uint64_maximum) == k_uint32_maximum);
static_assert(AdditionWorksAtCompileTime());

// =============================================================================
// =============================================================================

TEST(GGEMSObserverCounterArithmetic, SaturatesWideValues) {
  constexpr std::array<SaturationCase, 7U> k_cases{{
      {.label = "zero", .value = 0ULL, .expected = 0U},
      {.label = "one", .value = 1ULL, .expected = 1U},
      {.label = "below uint32 maximum",
       .value = static_cast<std::uint64_t>(k_uint32_maximum) - 1ULL,
       .expected = k_uint32_maximum - 1U},
      {.label = "at uint32 maximum",
       .value = k_uint32_maximum,
       .expected = k_uint32_maximum},
      {.label = "above uint32 maximum",
       .value = static_cast<std::uint64_t>(k_uint32_maximum) + 1ULL,
       .expected = k_uint32_maximum},
      {.label = "substantially larger 64-bit value",
       .value = 0x1'0000'0000'0000ULL,
       .expected = k_uint32_maximum},
      {.label = "uint64 maximum",
       .value = k_uint64_maximum,
       .expected = k_uint32_maximum},
  }};

  for (auto const &test_case : k_cases) {
    SCOPED_TRACE(test_case.label);
    EXPECT_EQ(SaturateObserverCounter(test_case.value), test_case.expected);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSObserverCounterArithmetic, AddsWithoutIntermediateOverflow) {
  constexpr std::array<AdditionCase, 11U> k_cases{{
      {.label = "zero plus zero",
       .destination = 0U,
       .value = 0ULL,
       .expected = 0U},
      {.label = "zero plus normal value",
       .destination = 0U,
       .value = 17ULL,
       .expected = 17U},
      {.label = "normal exact sum",
       .destination = 40U,
       .value = 2ULL,
       .expected = 42U},
      {.label = "sum exactly at maximum",
       .destination = k_uint32_maximum - 10U,
       .value = 10ULL,
       .expected = k_uint32_maximum},
      {.label = "sum exceeds maximum by one",
       .destination = k_uint32_maximum - 10U,
       .value = 11ULL,
       .expected = k_uint32_maximum},
      {.label = "maximum minus one plus one",
       .destination = k_uint32_maximum - 1U,
       .value = 1ULL,
       .expected = k_uint32_maximum},
      {.label = "maximum minus one plus two",
       .destination = k_uint32_maximum - 1U,
       .value = 2ULL,
       .expected = k_uint32_maximum},
      {.label = "saturated plus zero",
       .destination = k_uint32_maximum,
       .value = 0ULL,
       .expected = k_uint32_maximum},
      {.label = "saturated plus positive value",
       .destination = k_uint32_maximum,
       .value = 1ULL,
       .expected = k_uint32_maximum},
      {.label = "normal destination plus uint64 maximum",
       .destination = 42U,
       .value = k_uint64_maximum,
       .expected = k_uint32_maximum},
      {.label = "zero plus uint64 maximum",
       .destination = 0U,
       .value = k_uint64_maximum,
       .expected = k_uint32_maximum},
  }};

  for (auto const &test_case : k_cases) {
    SCOPED_TRACE(test_case.label);
    std::uint32_t destination{test_case.destination};
    AddSaturatedObserverCounter(destination, test_case.value);
    EXPECT_EQ(destination, test_case.expected);
  }
}

} // namespace
