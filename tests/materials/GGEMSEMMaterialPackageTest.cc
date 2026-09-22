#include <algorithm>
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace builtins = ggems::core::materials::builtins;
namespace units = ggems::units;

using namespace ggems::units;

static_assert(
  std::is_trivially_copyable_v<materials::GGEMSEMMaterialDescriptor>);
static_assert(
  std::is_trivially_copyable_v<materials::GGEMSEMElementalConstituent>);

// =============================================================================
// =============================================================================

auto MakeMaterial(
  std::string name, units::Density density,
  std::vector<materials::GGEMSMaterialComponent> const &composition)
  -> materials::GGEMSMaterial {
  return materials::GGEMSMaterial{std::move(name), density, composition};
}

// =============================================================================
// =============================================================================

auto MakeEnrichedBoron(std::string name, units::Density density,
                       long double boron_10_fraction)
  -> materials::GGEMSMaterial {
  return materials::GGEMSMaterial::FromIsotopicComposition(
    std::move(name), density,
    {
      {
        .mass_fraction = 1.0L,
        .isotopic_composition =
          materials::GGEMSIsotopicComposition{
            materials::GGEMSFractionBasis::AtomFraction,
            {
              {
                .isotope = {5U, 10U, 0U},
                .fraction = boron_10_fraction,
              },
              {
                .isotope = {5U, 11U, 0U},
                .fraction = 1.0L - boron_10_fraction,
              },
            }},
      },
    });
}

// =============================================================================
// =============================================================================

auto Compile(std::vector<materials::GGEMSMaterial> const &materials)
  -> materials::GGEMSEMMaterialPackage {
  return materials::GGEMSEMMaterialPackage{materials};
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, InternsIdenticalMaterialsUnderAnyName) {
  std::vector<materials::GGEMSMaterial> authored;
  authored.push_back(
    MakeMaterial("water", 1.0_g_cm3,
                 {
                   {.atomic_number = 1U, .mass_fraction = 0.111898L},
                   {.atomic_number = 8U, .mass_fraction = 0.888102L},
                 }));
  authored.push_back(
    MakeMaterial("same water under another name", 1.0_g_cm3,
                 {
                   {.atomic_number = 1U, .mass_fraction = 0.111898L},
                   {.atomic_number = 8U, .mass_fraction = 0.888102L},
                 }));
  authored.push_back(
    MakeMaterial("reversed element order", 1.0_g_cm3,
                 {
                   {.atomic_number = 8U, .mass_fraction = 0.888102L},
                   {.atomic_number = 1U, .mass_fraction = 0.111898L},
                 }));

  EXPECT_TRUE(HasSameScientificIdentity(authored[0], authored[1]));
  EXPECT_TRUE(HasSameScientificIdentity(authored[0], authored[2]));
  EXPECT_NE(authored[0].GetName(), authored[1].GetName());

  auto const package = Compile(authored);

  ASSERT_EQ(package.GetMaterialIds().size(), 3U);
  EXPECT_EQ(package.GetMaterialIds()[0], 0U);
  EXPECT_EQ(package.GetMaterialIds()[1], 0U);
  EXPECT_EQ(package.GetMaterialIds()[2], 0U);
  EXPECT_EQ(package.GetDescriptors().size(), 1U);
  EXPECT_EQ(package.GetElementalConstituents().size(), 2U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, SeparatesPhysicallyDifferentMaterials) {
  std::vector<materials::GGEMSMaterial> authored;
  authored.push_back(MakeMaterial(
    "boron", 2.34_g_cm3, {{.atomic_number = 5U, .mass_fraction = 1.0L}}));
  authored.push_back(
    MakeMaterial("denser boron", 2.40_g_cm3,
                 {{.atomic_number = 5U, .mass_fraction = 1.0L}}));
  authored.push_back(MakeEnrichedBoron("enriched boron", 2.34_g_cm3, 0.9L));
  authored.push_back(MakeEnrichedBoron("other enrichment", 2.34_g_cm3, 0.5L));

  auto const package = Compile(authored);

  auto const ids = package.GetMaterialIds();
  ASSERT_EQ(ids.size(), 4U);
  EXPECT_EQ(ids[0], 0U);
  EXPECT_EQ(ids[1], 1U);
  EXPECT_EQ(ids[2], 2U);
  EXPECT_EQ(ids[3], 3U);
  EXPECT_EQ(package.GetDescriptors().size(), 4U);

  EXPECT_FALSE(HasSameScientificIdentity(authored[0], authored[1]));
  EXPECT_FALSE(HasSameScientificIdentity(authored[0], authored[2]));
  EXPECT_FALSE(HasSameScientificIdentity(authored[2], authored[3]));
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, EquivalentAuthoringRoutesShareOneIdentity) {
  std::vector<materials::GGEMSMaterial> authored;
  authored.push_back(
    MakeMaterial("natural boron", 2.34_g_cm3,
                 {{.atomic_number = 5U, .mass_fraction = 1.0L}}));
  authored.push_back(materials::GGEMSMaterial::FromIsotopicComposition(
    "explicit natural boron", 2.34_g_cm3,
    {
      {
        .mass_fraction = 1.0L,
        .isotopic_composition = materials::ResolveIsotopeProfile(
          materials::GGEMSIsotopeProfile::Nist41Natural, 5U),
      },
    }));

  EXPECT_TRUE(HasSameScientificIdentity(authored[0], authored[1]));

  auto const package = Compile(authored);
  EXPECT_EQ(package.GetDescriptors().size(), 1U);
  EXPECT_EQ(package.GetMaterialIds()[1], 0U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest,
     PublishesTheDerivedElementalViewOfTheIdentity) {
  long double const first_hydrogen{0.111898000000000094611L};
  long double const second_hydrogen{0.111898000000000108489L};
  long double const oxygen{0.888102L};

  auto const first =
    MakeMaterial("first water", 1.0_g_cm3,
                 {
                   {.atomic_number = 1U, .mass_fraction = first_hydrogen},
                   {.atomic_number = 8U, .mass_fraction = oxygen},
                 });
  auto const second =
    MakeMaterial("second water", 1.0_g_cm3,
                 {
                   {.atomic_number = 1U, .mass_fraction = second_hydrogen},
                   {.atomic_number = 8U, .mass_fraction = oxygen},
                 });

  ASSERT_NE(first_hydrogen, second_hydrogen);
  ASSERT_TRUE(HasSameScientificIdentity(first, second));
  // The authored elemental shares still differ; the derived view does not.
  ASSERT_NE(first.GetConstituents()[0].mass_fraction,
            second.GetConstituents()[0].mass_fraction);
  ASSERT_EQ(first.GetElementalConstituents()[0].mass_fraction,
            second.GetElementalConstituents()[0].mass_fraction);

  auto const forward = Compile({first, second});
  auto const reversed = Compile({second, first});

  ASSERT_EQ(forward.GetDescriptors().size(), 1U);
  ASSERT_EQ(reversed.GetDescriptors().size(), 1U);
  ASSERT_EQ(forward.GetElementalConstituents().size(), 2U);
  ASSERT_EQ(reversed.GetElementalConstituents().size(), 2U);

  for (std::size_t index = 0U; index < 2U; ++index) {
    auto const &row = forward.GetElementalConstituents()[index];
    auto const &mirrored = reversed.GetElementalConstituents()[index];
    EXPECT_EQ(row.mass_fraction, mirrored.mass_fraction);
    EXPECT_EQ(row.number_density_per_cubic_centimeter,
              mirrored.number_density_per_cubic_centimeter);
    EXPECT_EQ(row.electron_density_per_cubic_centimeter,
              mirrored.electron_density_per_cubic_centimeter);
    EXPECT_EQ(row.mass_fraction,
              first.GetElementalConstituents()[index].mass_fraction);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, DeduplicatesVacuumAndKeepsItEmpty) {
  std::vector<materials::GGEMSMaterial> authored;
  authored.push_back(builtins::BuildBuiltInMaterial("Vacuum"));
  authored.push_back(
    MakeMaterial("second vacuum", units::Density{.value = 0.0L}, {}));
  authored.push_back(
    MakeMaterial("air-like", 1.205e-3_g_cm3,
                 {{.atomic_number = 7U, .mass_fraction = 1.0L}}));

  auto const package = Compile(authored);

  ASSERT_EQ(package.GetDescriptors().size(), 2U);
  EXPECT_EQ(package.GetMaterialIds()[1], 0U);
  EXPECT_EQ(package.GetMaterialIds()[2], 1U);
  EXPECT_EQ(package.GetDescriptors()[0].constituent_count, 0U);
  EXPECT_EQ(package.GetDescriptors()[0].total_atom_density_per_cubic_centimeter,
            0.0L);
  EXPECT_EQ(package.GetDescriptors()[0].density.value, 0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, CompilesEveryBuiltInIntoADenseFlatPackage) {
  std::vector<materials::GGEMSMaterial> authored;
  for (auto const name : builtins::GetAvailableMaterialNames()) {
    authored.push_back(builtins::BuildBuiltInMaterial(name));
  }
  ASSERT_EQ(authored.size(), 117U);

  auto const package = Compile(authored);
  auto const descriptors = package.GetDescriptors();
  auto const constituents = package.GetElementalConstituents();
  auto const ids = package.GetMaterialIds();

  ASSERT_EQ(ids.size(), authored.size());
  EXPECT_EQ(descriptors.size(), 117U);

  // Dense ids, contiguous non-overlapping ranges covering the flat array.
  std::size_t expected_offset{0U};
  for (std::size_t identifier = 0U; identifier < descriptors.size();
       ++identifier) {
    SCOPED_TRACE(identifier);

    auto const &descriptor = descriptors[identifier];
    EXPECT_EQ(descriptor.first_constituent, expected_offset);
    expected_offset += descriptor.constituent_count;
    EXPECT_LE(expected_offset, constituents.size());
  }
  EXPECT_EQ(expected_offset, constituents.size());

  for (std::size_t entry = 0U; entry < authored.size(); ++entry) {
    SCOPED_TRACE(authored[entry].GetName());

    auto const identifier = ids[entry];
    ASSERT_LT(identifier, descriptors.size());

    auto const &descriptor = descriptors[identifier];
    auto const &material = authored[entry];

    EXPECT_EQ(descriptor.density.value, material.GetDensity().value);
    EXPECT_EQ(descriptor.total_atom_density_per_cubic_centimeter,
              material.GetTotalAtomDensityPerCubicCentimeter());
    EXPECT_EQ(descriptor.electron_density_per_cubic_centimeter,
              material.GetElectronDensityPerCubicCentimeter());

    auto const elemental = material.GetElementalConstituents();
    ASSERT_EQ(descriptor.constituent_count, elemental.size());

    auto const rows = constituents.subspan(descriptor.first_constituent,
                                           descriptor.constituent_count);
    for (std::size_t index = 0U; index < rows.size(); ++index) {
      EXPECT_EQ(rows[index].atomic_number, elemental[index].atomic_number);
      EXPECT_EQ(rows[index].mass_fraction, elemental[index].mass_fraction);
      EXPECT_EQ(rows[index].number_density_per_cubic_centimeter,
                elemental[index].number_density_per_cubic_centimeter);
      EXPECT_EQ(rows[index].electron_density_per_cubic_centimeter,
                elemental[index].electron_density_per_cubic_centimeter);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, CarriesElementalRowsOnlyAndNoIsotopes) {
  std::vector<materials::GGEMSMaterial> authored;
  authored.push_back(builtins::BuildBuiltInMaterial("Water"));

  auto const package = Compile(authored);

  EXPECT_EQ(authored.front().GetIsotopeConstituents().size(), 5U);
  ASSERT_EQ(package.GetElementalConstituents().size(), 2U);
  EXPECT_EQ(package.GetElementalConstituents()[0].atomic_number, 1U);
  EXPECT_EQ(package.GetElementalConstituents()[1].atomic_number, 8U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, RebuildsIdenticalPackageFromTheSameInput) {
  std::vector<materials::GGEMSMaterial> authored;
  for (auto const name : builtins::GetAvailableMaterialNames()) {
    authored.push_back(builtins::BuildBuiltInMaterial(name));
  }
  authored.push_back(MakeEnrichedBoron("enriched boron", 2.34_g_cm3, 0.9L));

  auto const first = Compile(authored);
  auto const second = Compile(authored);

  ASSERT_EQ(first.GetDescriptors().size(), second.GetDescriptors().size());
  for (std::size_t index = 0U; index < first.GetDescriptors().size(); ++index) {
    auto const &left = first.GetDescriptors()[index];
    auto const &right = second.GetDescriptors()[index];
    EXPECT_EQ(left.density.value, right.density.value);
    EXPECT_EQ(left.first_constituent, right.first_constituent);
    EXPECT_EQ(left.constituent_count, right.constituent_count);
    EXPECT_EQ(left.total_atom_density_per_cubic_centimeter,
              right.total_atom_density_per_cubic_centimeter);
    EXPECT_EQ(left.electron_density_per_cubic_centimeter,
              right.electron_density_per_cubic_centimeter);
  }

  ASSERT_EQ(first.GetElementalConstituents().size(),
            second.GetElementalConstituents().size());
  for (std::size_t index = 0U; index < first.GetElementalConstituents().size();
       ++index) {
    auto const &left = first.GetElementalConstituents()[index];
    auto const &right = second.GetElementalConstituents()[index];
    EXPECT_EQ(left.atomic_number, right.atomic_number);
    EXPECT_EQ(left.mass_fraction, right.mass_fraction);
    EXPECT_EQ(left.number_density_per_cubic_centimeter,
              right.number_density_per_cubic_centimeter);
    EXPECT_EQ(left.electron_density_per_cubic_centimeter,
              right.electron_density_per_cubic_centimeter);
  }

  EXPECT_TRUE(
    std::ranges::equal(first.GetMaterialIds(), second.GetMaterialIds()));
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, CompilesManyAuthoredEntriesIntoFewIdentities) {
  constexpr std::size_t k_authored_entries{4096U};
  constexpr std::size_t k_distinct_materials{8U};

  std::vector<materials::GGEMSMaterial> authored;
  authored.reserve(k_authored_entries);
  for (std::size_t entry = 0U; entry < k_authored_entries; ++entry) {
    auto const variant = entry % k_distinct_materials;
    authored.push_back(
      MakeEnrichedBoron("boron " + std::to_string(entry), 2.34_g_cm3,
                        0.5L + (0.05L * static_cast<long double>(variant))));
  }

  auto const package = Compile(authored);

  EXPECT_EQ(package.GetDescriptors().size(), k_distinct_materials);
  EXPECT_EQ(package.GetElementalConstituents().size(), k_distinct_materials);

  auto const ids = package.GetMaterialIds();
  ASSERT_EQ(ids.size(), k_authored_entries);
  for (std::size_t entry = 0U; entry < k_authored_entries; ++entry) {
    EXPECT_EQ(ids[entry], entry % k_distinct_materials);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEMMaterialPackageTest, CompilesTheAuthoringRegistrySnapshot) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();
  static_cast<void>(manager.GetOrAddBuiltIn("Water"));
  static_cast<void>(manager.GetOrAddBuiltIn("Air"));

  auto const authored = manager.GetMaterials();
  materials::GGEMSEMMaterialPackage const package{authored};

  auto const ids = package.GetMaterialIds();
  ASSERT_EQ(ids.size(), authored.size());
  EXPECT_FALSE(package.GetDescriptors().empty());
  EXPECT_LE(package.GetDescriptors().size(), authored.size());

  for (std::size_t entry = 0U; entry < authored.size(); ++entry) {
    SCOPED_TRACE(authored[entry].GetName());
    ASSERT_LT(ids[entry], package.GetDescriptors().size());
    EXPECT_EQ(package.GetDescriptors()[ids[entry]].density.value,
              authored[entry].GetDensity().value);
  }
}
