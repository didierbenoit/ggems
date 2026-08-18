#include <array>
#include <cstddef>
#include <cstdint>
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

struct ExpectedIdentity {
  std::string_view name;
  std::string_view element_name;
  std::uint32_t atomic_number;
  std::uint32_t mass_number;
  std::string_view daughter_name;
  std::string_view decay_mode;
  long double q_value_kilo_electron_volt;
};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, DispatchesOnlyExactCanonicalNames) {
  constexpr std::array<ExpectedBuiltIn, 11U> expected{{
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
  constexpr std::array<std::string_view, 11U> expected{
      "H-3",   "C-14",   "F-18",  "C-11",   "O-15",  "Ga-68",
      "Co-60", "Lu-177", "I-131", "Am-241", "Tc-99m"};

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

TEST(GGEMSBuiltInRadionuclides, F18CatalogProvidesDetailedIdentityCard) {
  auto const *const info =
      ggems::core::radioactivity::builtins::FindBuiltInRadionuclideInfo("F-18");
  ASSERT_NE(info, nullptr);
  EXPECT_EQ(info->canonical_name, "F-18");
  EXPECT_EQ(info->element_name, "Fluorine");
  EXPECT_EQ(info->atomic_number, 9U);
  EXPECT_EQ(info->mass_number, 18U);
  EXPECT_EQ(info->daughter_name, "O-18");
  EXPECT_EQ(info->decay_mode, "beta+ / electron capture");
  EXPECT_EQ(info->q_value_kilo_electron_volt, 1'655.9L);
  EXPECT_EQ(info->nuclear_data_source, "LNHB / DDEP");
  EXPECT_EQ(info->beta_spectrum_source,
            "BetaShape 2.2 (experimental shape factor)");
  EXPECT_EQ(info->atomic_data_source, "LNHB / DDEP");

  std::string const description =
      ggems::core::radioactivity::builtins::DescribeBuiltInRadionuclide("F-18");
  EXPECT_NE(description.find("F-18"), std::string::npos);
  EXPECT_NE(description.find("Fluorine"), std::string::npos);
  EXPECT_NE(description.find("Z / A          : 9 / 18"), std::string::npos);
  EXPECT_NE(description.find("Daughter       : O-18"), std::string::npos);
  EXPECT_NE(description.find("beta+ / electron capture"), std::string::npos);
  EXPECT_NE(description.find("Q value        : 1655.9 keV"), std::string::npos);
  EXPECT_NE(description.find("flattened radioactive emission model"),
            std::string::npos);
  EXPECT_NE(description.find("Decay history  : not tracked"),
            std::string::npos);
  EXPECT_NE(description.find("3 flattened emission channels"),
            std::string::npos);
  EXPECT_NE(description.find("Positron | yield 0.9686"), std::string::npos);
  EXPECT_NE(description.find("endpoint 633.9000000 keV"), std::string::npos);
  EXPECT_NE(description.find("Electron | yield 0.00229 | Mono 14.3000000 eV"),
            std::string::npos);
  EXPECT_NE(description.find("Gamma | yield 0.0002 | Mono 525.0000000 eV"),
            std::string::npos);
  EXPECT_NE(description.find("Total yield    : 0.97109 particles/decay"),
            std::string::npos);
  EXPECT_NE(description.find("Data sources"), std::string::npos);
  EXPECT_NE(description.find("Nuclear data : LNHB / DDEP"), std::string::npos);
  EXPECT_NE(description.find(
                "Beta spectrum: BetaShape 2.2 (experimental shape factor)"),
            std::string::npos);
  EXPECT_NE(description.find("Atomic data  : LNHB / DDEP"), std::string::npos);
  EXPECT_NE(description.find("Model notes"), std::string::npos);
  EXPECT_NE(description.find("No source 511 keV annihilation photons"),
            std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, EveryBuiltInProvidesIdentityCardMetadata) {
  constexpr std::array<ExpectedIdentity, 11U> expected{{
      {.name = "H-3",
       .element_name = "Hydrogen",
       .atomic_number = 1U,
       .mass_number = 3U,
       .daughter_name = "He-3",
       .decay_mode = "beta-",
       .q_value_kilo_electron_volt = 18.591L},
      {.name = "C-14",
       .element_name = "Carbon",
       .atomic_number = 6U,
       .mass_number = 14U,
       .daughter_name = "N-14",
       .decay_mode = "beta-",
       .q_value_kilo_electron_volt = 156.476L},
      {.name = "F-18",
       .element_name = "Fluorine",
       .atomic_number = 9U,
       .mass_number = 18U,
       .daughter_name = "O-18",
       .decay_mode = "beta+ / electron capture",
       .q_value_kilo_electron_volt = 1'655.9L},
      {.name = "C-11",
       .element_name = "Carbon",
       .atomic_number = 6U,
       .mass_number = 11U,
       .daughter_name = "B-11",
       .decay_mode = "beta+ / electron capture",
       .q_value_kilo_electron_volt = 1'982.5L},
      {.name = "O-15",
       .element_name = "Oxygen",
       .atomic_number = 8U,
       .mass_number = 15U,
       .daughter_name = "N-15",
       .decay_mode = "beta+ / electron capture",
       .q_value_kilo_electron_volt = 2'754.18L},
      {.name = "Ga-68",
       .element_name = "Gallium",
       .atomic_number = 31U,
       .mass_number = 68U,
       .daughter_name = "Zn-68",
       .decay_mode = "beta+ / electron capture",
       .q_value_kilo_electron_volt = 2'921.1L},
      {.name = "Co-60",
       .element_name = "Cobalt",
       .atomic_number = 27U,
       .mass_number = 60U,
       .daughter_name = "Ni-60",
       .decay_mode = "beta-",
       .q_value_kilo_electron_volt = 2'823.07L},
      {.name = "Lu-177",
       .element_name = "Lutetium",
       .atomic_number = 71U,
       .mass_number = 177U,
       .daughter_name = "Hf-177",
       .decay_mode = "beta-",
       .q_value_kilo_electron_volt = 496.8L},
      {.name = "I-131",
       .element_name = "Iodine",
       .atomic_number = 53U,
       .mass_number = 131U,
       .daughter_name = "Xe-131",
       .decay_mode = "beta-",
       .q_value_kilo_electron_volt = 970.8L},
      {.name = "Am-241",
       .element_name = "Americium",
       .atomic_number = 95U,
       .mass_number = 241U,
       .daughter_name = "Np-237",
       .decay_mode = "alpha",
       .q_value_kilo_electron_volt = 5'637.82L},
      {.name = "Tc-99m",
       .element_name = "Technetium",
       .atomic_number = 43U,
       .mass_number = 99U,
       .daughter_name = "Tc-99 / Ru-99",
       .decay_mode = "internal transition / beta-",
       .q_value_kilo_electron_volt = 436.3L},
  }};

  for (ExpectedIdentity const &entry : expected) {
    auto const *const info =
        ggems::core::radioactivity::builtins::FindBuiltInRadionuclideInfo(
            entry.name);
    ASSERT_NE(info, nullptr) << entry.name;
    EXPECT_EQ(info->canonical_name, entry.name);
    EXPECT_EQ(info->element_name, entry.element_name);
    EXPECT_EQ(info->atomic_number, entry.atomic_number);
    EXPECT_EQ(info->mass_number, entry.mass_number);
    EXPECT_EQ(info->daughter_name, entry.daughter_name);
    EXPECT_EQ(info->decay_mode, entry.decay_mode);
    EXPECT_EQ(info->q_value_kilo_electron_volt,
              entry.q_value_kilo_electron_volt);
    EXPECT_FALSE(info->nuclear_data_source.empty());
    EXPECT_FALSE(info->beta_spectrum_source.empty());
    EXPECT_FALSE(info->atomic_data_source.empty());
    EXPECT_FALSE(info->model_notes.empty());

    std::string const description =
        ggems::core::radioactivity::builtins::DescribeBuiltInRadionuclide(
            entry.name);
    EXPECT_EQ(description.find("Catalog details: not populated"),
              std::string::npos);
    EXPECT_NE(description.find("GGEMS model"), std::string::npos);
    EXPECT_NE(description.find("Decay history  : not tracked"),
              std::string::npos);
    EXPECT_NE(description.find("Data sources"), std::string::npos);
    EXPECT_NE(description.find("Model notes"), std::string::npos);
  }

  EXPECT_EQ(
      ggems::core::radioactivity::builtins::FindBuiltInRadionuclideInfo("F18"),
      nullptr);
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
