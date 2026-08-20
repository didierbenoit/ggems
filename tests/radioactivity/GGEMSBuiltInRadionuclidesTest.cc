#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace {

struct ExpectedBuiltIn {
  std::string_view name;
  long double half_life_seconds;
  std::size_t emission_count;
};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, DispatchesOnlyExactCanonicalNames) {
  constexpr std::array<ExpectedBuiltIn, 14U> expected{{
      {.name = "H-3",
       .half_life_seconds = 388'500'000.0L,
       .emission_count = 1U},
      {.name = "C-14",
       .half_life_seconds = 179'900'000'000.0L,
       .emission_count = 1U},
      {.name = "F-18", .half_life_seconds = 6584.04L, .emission_count = 3U},
      {.name = "C-11", .half_life_seconds = 1221.66L, .emission_count = 1U},
      {.name = "O-15", .half_life_seconds = 122.266L, .emission_count = 1U},
      {.name = "Ga-68", .half_life_seconds = 4'069.8L, .emission_count = 7U},
      {.name = "Co-60",
       .half_life_seconds = 166'340'000.0L,
       .emission_count = 7U},
      {.name = "Lu-177",
       .half_life_seconds = 574'067.52L,
       .emission_count = 8U},
      {.name = "I-123", .half_life_seconds = 47'604.24L, .emission_count = 4U},
      {.name = "I-124", .half_life_seconds = 360'806.4L, .emission_count = 13U},
      {.name = "I-125",
       .half_life_seconds = 5'131'123.2L,
       .emission_count = 4U},
      {.name = "I-131",
       .half_life_seconds = 693'213.12L,
       .emission_count = 10U},
      {.name = "Am-241",
       .half_life_seconds = 13'652'000'000.0L,
       .emission_count = 6U},
      {.name = "Tc-99m", .half_life_seconds = 21'624.12L, .emission_count = 6U},
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

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides,
     ListsEveryAvailableCanonicalNameInDispatchOrder) {
  constexpr std::array<std::string_view, 14U> expected{
      "H-3",    "C-14",  "F-18",  "C-11",  "O-15",  "Ga-68",  "Co-60",
      "Lu-177", "I-123", "I-124", "I-125", "I-131", "Am-241", "Tc-99m"};

  auto const available =
      ggems::core::radioactivity::builtins::GetAvailableRadionuclideNames();
  ASSERT_EQ(available.size(), expected.size());

  for (std::size_t index = 0U; index < expected.size(); ++index) {
    EXPECT_EQ(available[index], expected[index]);
    EXPECT_TRUE(ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(
                    available[index])
                    .has_value());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, DescriptionRejectsUnknownCanonicalName) {
  EXPECT_THROW(
      static_cast<void>(
          ggems::core::radioactivity::builtins::DescribeBuiltInRadionuclide(
              "F18")),
      ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, DescriptionReflectsBuiltDefinition) {
  std::string const description =
      ggems::core::radioactivity::builtins::DescribeBuiltInRadionuclide("H-3");

  EXPECT_NE(description.find("H-3"), std::string::npos);
  EXPECT_NE(description.find("Electron | yield 1"), std::string::npos);
  EXPECT_NE(description.find("Regular spectrum | 38 bins"), std::string::npos);
  EXPECT_NE(description.find("18.5910000 keV"), std::string::npos);
  EXPECT_NE(description.find("489.2360000 eV"), std::string::npos);
  EXPECT_NE(description.find("Total yield    : 1 particles/decay"),
            std::string::npos);
}
