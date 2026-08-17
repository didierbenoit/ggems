#include <array>
#include <cstddef>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace {

struct ExpectedBuiltIn {
  std::string_view name;
  long double half_life_seconds;
  std::size_t emission_count;
};

} // namespace

TEST(GGEMSBuiltInRadionuclides, DispatchesOnlyExactCanonicalNames) {
  constexpr std::array<ExpectedBuiltIn, 4U> expected{{
      {.name = "F-18", .half_life_seconds = 6584.04L, .emission_count = 3U},
      {.name = "C-11", .half_life_seconds = 1221.66L, .emission_count = 1U},
      {.name = "O-15", .half_life_seconds = 122.266L, .emission_count = 1U},
      {.name = "Lu-177",
       .half_life_seconds = 574'067.52L,
       .emission_count = 8U},
  }};

  for (auto const &entry : expected) {
    auto definition =
        ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(
            entry.name);
    ASSERT_TRUE(definition.has_value());
    EXPECT_EQ(definition->GetCanonicalName(), entry.name);
    EXPECT_EQ(definition->GetHalfLifeSeconds(), entry.half_life_seconds);
    EXPECT_EQ(definition->GetEmissions().size(), entry.emission_count);
  }

  EXPECT_FALSE(
      ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("F18")
          .has_value());
  EXPECT_FALSE(
      ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("f-18")
          .has_value());
  EXPECT_FALSE(
      ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(" F-18 ")
          .has_value());
}
