#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideLibrary.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideLibrary;
using ggems::core::radioactivity::builtins::BuildC11Radionuclide;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::radioactivity::builtins::BuildO15Radionuclide;
using ggems::core::sources::GGEMSEnergyDistribution;

static_assert(std::is_const_v<std::remove_reference_t<
                  decltype(*std::declval<
                           GGEMSRadionuclideLibrary::DefinitionPointer>())>>);

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDefinition(std::string canonical_name,
                                  std::vector<std::string> aliases = {},
                                  long double yield_per_decay = 1.0L)
    -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.emplace_back(GGEMSParticleType::Gamma, yield_per_decay,
                         GGEMSEnergyDistribution::BuildMono(1ULL));

  return {std::move(canonical_name), std::move(aliases), 100.0L,
          std::move(emissions)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeLargeDefinition() -> GGEMSRadionuclideDefinition {
  constexpr std::size_t table_count{4096U};
  std::vector<double> centers;
  std::vector<double> weights(table_count, 1.0);
  centers.reserve(table_count);

  for (std::size_t index = 0U; index < table_count; ++index) {
    centers.push_back(20.0 + (static_cast<double>(index) * 2.0));
  }

  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.emplace_back(
      GGEMSParticleType::Electron, 1.0L,
      GGEMSEnergyDistribution::BuildRegularSpectrum(centers, weights, "keV"));

  return {"Synthetic-Large", {"SL"}, 100.0L, std::move(emissions)};
}
} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest, EmptyLibraryIsValid) {
  GGEMSRadionuclideLibrary const library;

  EXPECT_EQ(library.GetCount(), 0U);
  EXPECT_TRUE(library.GetDefinitions().empty());
  EXPECT_EQ(library.Find("missing"), nullptr);
  EXPECT_EQ(library.Find(" \t\r\n"), nullptr);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest,
     RegistersAndFindsCanonicalNameAndExplicitAliases) {
  GGEMSRadionuclideLibrary library;
  auto const registered =
      library.Add(MakeDefinition("Synthetic-One", {"S1", "First synthetic"}));

  ASSERT_NE(registered, nullptr);
  EXPECT_EQ(library.GetCount(), 1U);
  ASSERT_EQ(library.GetDefinitions().size(), 1U);
  EXPECT_EQ(library.GetDefinitions()[0U], registered);
  EXPECT_EQ(library.Find("Synthetic-One"), registered);
  EXPECT_EQ(library.Find("S1"), registered);
  EXPECT_EQ(library.Find("First synthetic"), registered);
  EXPECT_EQ(registered->GetCanonicalName(), "Synthetic-One");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest,
     LookupTrimsAsciiWhitespaceAndFoldsAsciiCaseOnly) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(
      MakeDefinition("Synthetic-One", {"Alias_One", "Name With Space"}));

  EXPECT_EQ(library.Find(" \tSYNTHETIC-ONE\r\n"), registered);
  EXPECT_EQ(library.Find("\falias_one\v"), registered);
  EXPECT_EQ(library.Find("name with space"), registered);

  EXPECT_EQ(library.Find("SyntheticOne"), nullptr);
  EXPECT_EQ(library.Find("Synthetic_One"), nullptr);
  EXPECT_EQ(library.Find("NameWithSpace"), nullptr);
  EXPECT_EQ(registered->GetCanonicalName(), "Synthetic-One");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest, RejectsEveryLookupCollisionDirection) {
  GGEMSRadionuclideLibrary library;
  auto const first =
      library.Add(MakeDefinition("Synthetic-One", {"S1", "Common alias"}));

  EXPECT_THROW((void)library.Add(MakeDefinition(" synthetic-one ")),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW((void)library.Add(MakeDefinition(" COMMON ALIAS ")),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW((void)library.Add(MakeDefinition("Synthetic-Two", {" s1 "})),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(library.GetCount(), 1U);
  EXPECT_EQ(library.Find("Synthetic-One"), first);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest,
     RejectsInvalidAndDuplicateAliasesWithinOneDefinition) {
  GGEMSRadionuclideLibrary library;

  EXPECT_THROW(
      (void)library.Add(MakeDefinition("Synthetic-One", {"Alias", " alias "})),
      ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(
      (void)library.Add(MakeDefinition("Synthetic-One", {" synthetic-one "})),
      ggems::core::GGEMSExceptionBase);
  EXPECT_THROW((void)library.Add(MakeDefinition("Synthetic-One", {" \t"})),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(library.GetCount(), 0U);
  EXPECT_TRUE(library.GetDefinitions().empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest,
     FailedRegistrationPreservesDefinitionsOrderAndAllLookupKeys) {
  GGEMSRadionuclideLibrary library;
  auto const first = library.Add(MakeDefinition("Synthetic-One", {"S1"}));
  auto const second = library.Add(MakeDefinition("Synthetic-Two", {"S2"}));

  EXPECT_THROW((void)library.Add(MakeDefinition("Synthetic-Rejected",
                                                {"Fresh alias", " s2 "})),
               ggems::core::GGEMSExceptionBase);

  ASSERT_EQ(library.GetCount(), 2U);
  ASSERT_EQ(library.GetDefinitions().size(), 2U);
  EXPECT_EQ(library.GetDefinitions()[0U], first);
  EXPECT_EQ(library.GetDefinitions()[1U], second);
  EXPECT_EQ(library.Find("Synthetic-One"), first);
  EXPECT_EQ(library.Find("S1"), first);
  EXPECT_EQ(library.Find("Synthetic-Two"), second);
  EXPECT_EQ(library.Find("S2"), second);
  EXPECT_EQ(library.Find("Synthetic-Rejected"), nullptr);
  EXPECT_EQ(library.Find("Fresh alias"), nullptr);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest, PreservesDeterministicRegistrationOrder) {
  GGEMSRadionuclideLibrary library;
  auto const third = library.Add(MakeDefinition("Synthetic-Three"));
  auto const first = library.Add(MakeDefinition("Synthetic-One"));
  auto const second = library.Add(MakeDefinition("Synthetic-Two"));

  auto const definitions = library.GetDefinitions();
  ASSERT_EQ(definitions.size(), 3U);
  EXPECT_EQ(definitions[0U], third);
  EXPECT_EQ(definitions[1U], first);
  EXPECT_EQ(definitions[2U], second);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest,
     LookupKeepsStableImmutableDefinitionWithoutCopyingEnergyTables) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(MakeLargeDefinition());
  auto const original_table = registered->GetEmissions()[0U]
                                  .GetEnergyDistribution()
                                  .GetEnergyValuesMilliElectronVolt();
  auto const *original_data = original_table.data();

  auto const by_alias = library.Find("sl");
  ASSERT_EQ(by_alias, registered);
  EXPECT_EQ(by_alias.get(), registered.get());
  EXPECT_EQ(by_alias->GetEmissions()[0U]
                .GetEnergyDistribution()
                .GetEnergyValuesMilliElectronVolt()
                .data(),
            original_data);

  for (std::size_t index = 0U; index < 64U; ++index) {
    static_cast<void>(library.Add(
        MakeDefinition("Synthetic-Additional-" + std::to_string(index))));
  }

  auto const after_growth = library.Find("Synthetic-Large");
  ASSERT_EQ(after_growth, registered);
  EXPECT_EQ(after_growth.get(), registered.get());
  EXPECT_EQ(after_growth->GetEmissions()[0U]
                .GetEnergyDistribution()
                .GetEnergyValuesMilliElectronVolt()
                .data(),
            original_data);
  EXPECT_EQ(
      after_growth->GetEmissions()[0U].GetEnergyDistribution().GetTableCount(),
      4096U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest,
     RealBuiltInsCoexistInDeterministicStableOrder) {
  GGEMSRadionuclideLibrary library;
  auto const f18 = library.Add(BuildF18Radionuclide());
  auto const c11 = library.Add(BuildC11Radionuclide());
  auto const o15 = library.Add(BuildO15Radionuclide());

  auto const definitions = library.GetDefinitions();
  ASSERT_EQ(definitions.size(), 3U);
  EXPECT_EQ(definitions[0U], f18);
  EXPECT_EQ(definitions[1U], c11);
  EXPECT_EQ(definitions[2U], o15);

  EXPECT_EQ(library.Find("18f"), f18);
  EXPECT_EQ(library.Find("carbon-11"), c11);
  EXPECT_EQ(library.Find("15o"), o15);
  EXPECT_EQ(library.Find("F-18").get(), f18.get());
  EXPECT_EQ(library.Find("C-11").get(), c11.get());
  EXPECT_EQ(library.Find("O-15").get(), o15.get());

  EXPECT_NE(f18->GetHalfLifeSeconds(), c11->GetHalfLifeSeconds());
  EXPECT_NE(f18->GetHalfLifeSeconds(), o15->GetHalfLifeSeconds());
  EXPECT_NE(c11->GetHalfLifeSeconds(), o15->GetHalfLifeSeconds());

  auto const &f18_energy = f18->GetEmissions()[0U].GetEnergyDistribution();
  auto const &c11_energy = c11->GetEmissions()[0U].GetEnergyDistribution();
  auto const &o15_energy = o15->GetEmissions()[0U].GetEnergyDistribution();
  EXPECT_EQ(f18_energy.GetTableCount(), 1'268U);
  EXPECT_EQ(c11_energy.GetTableCount(), 1'921U);
  EXPECT_EQ(o15_energy.GetTableCount(), 3'465U);

  auto const endpoint =
      [](GGEMSEnergyDistribution const &distribution) -> std::uint64_t {
    auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
    return centers.back() +
           (distribution.GetRegularBinWidthMilliElectronVolt() / 2ULL);
  };
  EXPECT_EQ(endpoint(f18_energy), 633'900'000ULL);
  EXPECT_EQ(endpoint(c11_energy), 960'500'000ULL);
  EXPECT_EQ(endpoint(o15_energy), 1'732'180'000ULL);
}
