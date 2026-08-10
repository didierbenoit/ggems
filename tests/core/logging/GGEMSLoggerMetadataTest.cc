#include <array>
#include <barrier>
#include <cstddef>
#include <string>
#include <string_view>
#include <thread>
#include <algorithm>

#include <gtest/gtest.h>

#include "GGEMS/core/detail/GGEMSLoggerMetadata.hh"

namespace {

using ggems::core::logging::detail::SimplifyFunctionName;
using ggems::core::logging::detail::ThreadTag;

// =============================================================================
// =============================================================================

struct FunctionNameCase {
  char const *label;
  std::string_view input;
  std::string_view expected;
};

// =============================================================================
// =============================================================================

struct ThreadTagPair {
  std::string first;
  std::string second;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto IsDecimalThreadTag(std::string_view tag) noexcept -> bool {
  return tag.size() >= 2U && tag.front() == 'T' &&
         std::ranges::all_of(tag.substr(1U), [](char character) -> bool {
           return character >= '0' && character <= '9';
         });
}

static_assert(noexcept(SimplifyFunctionName(std::string_view{})));

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSLoggerMetadataTest, SimplifiesFunctionNamesWithCurrentHeuristics) {
  constexpr std::array<FunctionNameCase, 16U> test_cases{{
      {.label = "GGEMS free function",
       .input = "void ggems::core::FreeFunction()",
       .expected = "FreeFunction"},
      {.label = "GGEMS class method",
       .input = "void ggems::core::GGEMSLogger::Dispatch()",
       .expected = "GGEMSLogger::Dispatch"},
      {.label = "nested scopes",
       .input = "void ggems::core::logging::detail::Parse()",
       .expected = "detail::Parse"},
      {.label = "nested template arguments",
       .input = "void ggems::core::Widget<std::vector<std::pair<int, float>>>::"
                "Run<std::tuple<int, float>>(int)",
       .expected = "Widget<std::vector<std::pair<int, float>>>::Run"},
      {.label = "call operator",
       .input = "void ggems::core::Functor::operator()()",
       .expected = "Functor::operator()"},
      {.label = "angle operator defect",
       .input = "bool ggems::core::Comparator::operator<(int)",
       .expected = "Comparator::operator"},
      {.label = "function-pointer parameter defect",
       .input = "void ggems::core::Handler::Register(void (*callback)(int))",
       .expected = "Handler::Register(void (*callback)"},
      {.label = "external calling convention",
       .input = "class std::vector<int> __cdecl external::Widget::Run(double)",
       .expected = "Widget::Run"},
      {.label = "Clang-like lambda",
       .input = "void ggems::core::Worker::Run()::<lambda()>()",
       .expected = "Worker::Run"},
      {.label = "alternate lambda",
       .input = "void ggems::core::Worker::Run()::(lambda)()",
       .expected = "Worker::Run"},
      {.label = "trailing space",
       .input = "void ggems::core::Whitespace::Function ",
       .expected = "Whitespace::Function"},
      {.label = "trailing tab",
       .input = "void ggems::core::Whitespace::Function\t",
       .expected = "Whitespace::Function"},
      {.label = "trailing LF",
       .input = "void ggems::core::Whitespace::Function\n",
       .expected = "Whitespace::Function"},
      {.label = "trailing CR",
       .input = "void ggems::core::Whitespace::Function\r",
       .expected = "Whitespace::Function"},
      {.label = "empty string", .input = "", .expected = ""},
      {.label = "no parameter list",
       .input = "void ggems::core::Worker::Tick",
       .expected = "Worker::Tick"},
  }};

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.label);
    EXPECT_EQ(SimplifyFunctionName(test_case.input), test_case.expected);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSLoggerMetadataTest, ThreadTagIsStableForCurrentThread) {
  auto const first_tag = ThreadTag();
  auto const second_tag = ThreadTag();

  EXPECT_TRUE(IsDecimalThreadTag(first_tag));
  EXPECT_EQ(first_tag, second_tag);
}

// =============================================================================
// =============================================================================

TEST(GGEMSLoggerMetadataTest, SimultaneousWorkersHaveStableDistinctTags) {
  std::barrier<> rendezvous{3};
  std::array<ThreadTagPair, 2U> worker_tags{};

  auto collect_tags = [&](std::size_t worker_index) -> void {
    rendezvous.arrive_and_wait();
    worker_tags[worker_index].first = ThreadTag();
    rendezvous.arrive_and_wait();
    worker_tags[worker_index].second = ThreadTag();
  };

  std::thread first_worker{collect_tags, 0U};
  std::thread second_worker{collect_tags, 1U};

  rendezvous.arrive_and_wait();
  rendezvous.arrive_and_wait();

  first_worker.join();
  second_worker.join();

  for (auto const &worker_tag : worker_tags) {
    EXPECT_TRUE(IsDecimalThreadTag(worker_tag.first));
    EXPECT_EQ(worker_tag.first, worker_tag.second);
  }

  EXPECT_NE(worker_tags[0U].first, worker_tags[1U].first);
}
