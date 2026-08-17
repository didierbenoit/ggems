#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideLibrary.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideLibrary;
using ggems::core::radioactivity::builtins::BuildC11Radionuclide;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::radioactivity::builtins::BuildLu177Radionuclide;
using ggems::core::radioactivity::builtins::BuildO15Radionuclide;
using ggems::core::sources::GGEMSEnergyDistribution;

static_assert(std::is_const_v<std::remove_reference_t<
                  decltype(*std::declval<
                           GGEMSRadionuclideLibrary::DefinitionPointer>())>>);

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDefinition(std::string canonical_name,
                                  long double yield_per_decay = 1.0L)
    -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.emplace_back(GGEMSParticleType::Gamma, yield_per_decay,
                         GGEMSEnergyDistribution::BuildMono(1ULL));

  return {std::move(canonical_name), 100.0L, std::move(emissions)};
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

  return {"Synthetic-Large", 100.0L, std::move(emissions)};
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

TEST(GGEMSRadionuclideLibraryTest, RegistersAndFindsCanonicalName) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(MakeDefinition("Synthetic-One"));

  ASSERT_NE(registered, nullptr);
  EXPECT_EQ(library.GetCount(), 1U);
  ASSERT_EQ(library.GetDefinitions().size(), 1U);
  EXPECT_EQ(library.GetDefinitions()[0U], registered);
  EXPECT_EQ(library.Find("Synthetic-One"), registered);
  EXPECT_EQ(registered->GetCanonicalName(), "Synthetic-One");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest, LookupUsesExactCanonicalName) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(MakeDefinition("Synthetic-One"));

  EXPECT_EQ(library.Find("Synthetic-One"), registered);

  EXPECT_EQ(library.Find("synthetic-one"), nullptr);
  EXPECT_EQ(library.Find(" SYNTHETIC-ONE "), nullptr);
  EXPECT_EQ(library.Find("SyntheticOne"), nullptr);
  EXPECT_EQ(library.Find("Synthetic_One"), nullptr);
  EXPECT_EQ(registered->GetCanonicalName(), "Synthetic-One");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest, RejectsDuplicateCanonicalName) {
  GGEMSRadionuclideLibrary library;
  auto const first = library.Add(MakeDefinition("Synthetic-One"));

  EXPECT_THROW((void)library.Add(MakeDefinition("Synthetic-One")),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(library.GetCount(), 1U);
  EXPECT_EQ(library.Find("Synthetic-One"), first);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideLibraryTest,
     FailedRegistrationPreservesDefinitionsAndOrder) {
  GGEMSRadionuclideLibrary library;
  auto const first = library.Add(MakeDefinition("Synthetic-One"));
  auto const second = library.Add(MakeDefinition("Synthetic-Two"));

  EXPECT_THROW((void)library.Add(MakeDefinition("Synthetic-One")),
               ggems::core::GGEMSExceptionBase);

  ASSERT_EQ(library.GetCount(), 2U);
  ASSERT_EQ(library.GetDefinitions().size(), 2U);
  EXPECT_EQ(library.GetDefinitions()[0U], first);
  EXPECT_EQ(library.GetDefinitions()[1U], second);
  EXPECT_EQ(library.Find("Synthetic-One"), first);
  EXPECT_EQ(library.Find("Synthetic-Two"), second);
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

  auto const by_name = library.Find("Synthetic-Large");
  ASSERT_EQ(by_name, registered);
  EXPECT_EQ(by_name.get(), registered.get());
  EXPECT_EQ(by_name->GetEmissions()[0U]
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
  auto const lu177 = library.Add(BuildLu177Radionuclide());

  auto const definitions = library.GetDefinitions();
  ASSERT_EQ(definitions.size(), 4U);
  EXPECT_EQ(definitions[0U], f18);
  EXPECT_EQ(definitions[1U], c11);
  EXPECT_EQ(definitions[2U], o15);
  EXPECT_EQ(definitions[3U], lu177);

  EXPECT_EQ(library.Find("F-18"), f18);
  EXPECT_EQ(library.Find("C-11"), c11);
  EXPECT_EQ(library.Find("O-15"), o15);
  EXPECT_EQ(library.Find("Lu-177"), lu177);

  EXPECT_EQ(library.Find("18f"), nullptr);
  EXPECT_EQ(library.Find("carbon-11"), nullptr);
  EXPECT_EQ(library.Find("15o"), nullptr);
  EXPECT_EQ(library.Find("lu-177"), nullptr);

  EXPECT_NE(f18->GetHalfLifeSeconds(), c11->GetHalfLifeSeconds());
  EXPECT_NE(f18->GetHalfLifeSeconds(), o15->GetHalfLifeSeconds());
  EXPECT_NE(c11->GetHalfLifeSeconds(), o15->GetHalfLifeSeconds());
  EXPECT_NE(o15->GetHalfLifeSeconds(), lu177->GetHalfLifeSeconds());

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
