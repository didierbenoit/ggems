#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialDescription.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

namespace materials = ggems::core::materials;
namespace builtins = ggems::core::materials::builtins;

using namespace ggems::units;
using Registration = materials::GGEMSMaterialRegistration;

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesMaterial) {
  auto const material = builtins::BuildBuiltInMaterial("Water");

  auto const description = materials::DescribeMaterial(material);

  EXPECT_TRUE(description.contains("Material: Water"));
  EXPECT_TRUE(description.contains("H Hydrogen Z=1"));
  EXPECT_TRUE(description.contains("O Oxygen Z=8"));
  EXPECT_TRUE(description.contains("Atom density"));
  EXPECT_TRUE(description.contains("Electron density"));
  EXPECT_FALSE(description.contains("Registration"));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, InspectsWaterFromTheMaterialAuthorities) {
  auto const material = builtins::BuildBuiltInMaterial("Water");

  auto const inspection = materials::InspectMaterial(material);

  EXPECT_EQ(inspection.name, "Water");
  EXPECT_EQ(inspection.density.value, material.GetDensity().value);
  EXPECT_EQ(inspection.registration, Registration::Unknown);
  EXPECT_FALSE(inspection.manager_index.has_value());

  auto const elements = material.GetElementalConstituents();
  ASSERT_EQ(inspection.elements.size(), elements.size());
  for (std::size_t index = 0U; index < elements.size(); ++index) {
    auto const &element = inspection.elements[index];
    EXPECT_EQ(element.values.atomic_number, elements[index].atomic_number);
    EXPECT_EQ(element.values.mass_fraction, elements[index].mass_fraction);
    EXPECT_EQ(element.values.number_density_per_cubic_centimeter,
              elements[index].number_density_per_cubic_centimeter);
    EXPECT_EQ(element.values.electron_density_per_cubic_centimeter,
              elements[index].electron_density_per_cubic_centimeter);
    EXPECT_EQ(element.isotope_profile,
              materials::GGEMSIsotopeProfile::Nist41Natural);
  }
  EXPECT_EQ(inspection.elements[0].symbol, "H");
  EXPECT_EQ(inspection.elements[1].name, "Oxygen");

  auto const isotopes = material.GetIsotopeConstituents();
  ASSERT_EQ(inspection.isotopes.size(), 5U);
  for (std::size_t index = 0U; index < isotopes.size(); ++index) {
    EXPECT_EQ(inspection.isotopes[index], isotopes[index]);
  }

  EXPECT_EQ(inspection.total_atom_density_per_cubic_centimeter,
            material.GetTotalAtomDensityPerCubicCentimeter());
  EXPECT_EQ(inspection.electron_density_per_cubic_centimeter,
            material.GetElectronDensityPerCubicCentimeter());
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesIsotopesAndDerivedElementalView) {
  auto const material = builtins::BuildBuiltInMaterial("Water");

  auto const description = materials::DescribeMaterial(material);

  for (auto const *isotope : {"H-1 ", "H-2 ", "O-16 ", "O-17 ", "O-18 "}) {
    EXPECT_TRUE(description.contains(isotope)) << isotope;
  }
  EXPECT_TRUE(description.contains("Z=8 A=17 M=0"));

  for (auto const &isotope : material.GetIsotopeConstituents()) {
    EXPECT_TRUE(description.contains(
      std::format("atom fraction in element {:.8g}, number density {:.8g}",
                  isotope.atom_fraction_in_element,
                  isotope.number_density_per_cubic_centimeter)));
  }

  for (auto const &element : material.GetElementalConstituents()) {
    EXPECT_TRUE(description.contains(
      std::format("derived mass fraction : {:.8g}", element.mass_fraction)));
    EXPECT_TRUE(description.contains(
      std::format("electron density      : {:.8g}",
                  element.electron_density_per_cubic_centimeter)));
  }

  EXPECT_TRUE(description.contains(
    "isotope profile       : NIST 4.1 representative natural composition"));
  EXPECT_TRUE(description.contains(
    std::format("Atom density     : {:.8g}",
                material.GetTotalAtomDensityPerCubicCentimeter())));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesSingleElementMaterial) {
  auto const inspection =
    materials::InspectMaterial(builtins::BuildBuiltInMaterial("Aluminum"));

  ASSERT_EQ(inspection.elements.size(), 1U);
  EXPECT_EQ(inspection.elements[0].values.atomic_number, 13U);
  EXPECT_EQ(inspection.isotopes.size(), 1U);
  EXPECT_TRUE(materials::DescribeMaterial(inspection).contains("Al-27 "));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, ReportsRetainedProfilesTruthfully) {
  auto const technetium =
    materials::InspectMaterial(builtins::BuildBuiltInMaterial("Technetium"));
  ASSERT_EQ(technetium.elements.size(), 1U);
  EXPECT_EQ(technetium.elements[0].isotope_profile,
            materials::GGEMSIsotopeProfile::LegacyReferenceIsotope);

  auto const description = materials::DescribeMaterial(technetium);
  EXPECT_TRUE(description.contains("Tc-97 "));
  EXPECT_TRUE(description.contains("legacy reference isotope"));
  EXPECT_FALSE(description.contains("NIST"));

  auto const tantalum =
    materials::DescribeMaterial(builtins::BuildBuiltInMaterial("Tantalum"));
  EXPECT_TRUE(tantalum.contains("Ta-180m "));
  EXPECT_TRUE(tantalum.contains("Z=73 A=180 M=1"));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, ExplicitCompositionRetainsNoProfile) {
  // Numerically the natural boron profile, but authored explicitly: no profile
  // is retained, so none may be reported.
  auto const material = materials::GGEMSMaterial::FromIsotopicComposition(
    "explicit boron", 2.34_g_cm3,
    {
      {
        .mass_fraction = 1.0L,
        .isotopic_composition = materials::ResolveIsotopeProfile(
          materials::GGEMSIsotopeProfile::Nist41Natural, 5U),
      },
    });

  auto const inspection = materials::InspectMaterial(material);
  ASSERT_EQ(inspection.elements.size(), 1U);
  EXPECT_FALSE(inspection.elements[0].isotope_profile.has_value());

  auto const description = materials::DescribeMaterial(inspection);
  EXPECT_TRUE(description.contains("isotope profile       : not retained"));
  EXPECT_TRUE(description.contains("B-10 "));
  EXPECT_FALSE(description.contains("natural"));
  EXPECT_FALSE(description.contains("enrich"));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesVacuum) {
  auto const inspection =
    materials::InspectMaterial(builtins::BuildBuiltInMaterial("Vacuum"));

  EXPECT_EQ(inspection.density.value, 0.0L);
  EXPECT_TRUE(inspection.elements.empty());
  EXPECT_TRUE(inspection.isotopes.empty());
  EXPECT_EQ(inspection.total_atom_density_per_cubic_centimeter, 0.0L);
  EXPECT_EQ(inspection.electron_density_per_cubic_centimeter, 0.0L);

  auto const description = materials::DescribeMaterial(inspection);
  EXPECT_TRUE(description.contains("Material: Vacuum"));
  EXPECT_TRUE(description.contains("Elements         : none"));
  EXPECT_FALSE(description.contains("Z="));
  EXPECT_FALSE(description.contains("isotope profile"));
  EXPECT_FALSE(description.contains("emperature"));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, InspectsRegisteredCustomMaterial) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();
  auto const manager_index = manager.AddCustomMaterial(materials::GGEMSMaterial{
    "DescriptionTestCustomWater",
    1.0_g_cm3,
    {
      {.atomic_number = 1U, .mass_fraction = 0.111898L},
      {.atomic_number = 8U, .mass_fraction = 0.888102L},
    },
  });

  auto const by_name =
    materials::InspectMaterial(manager, "DescriptionTestCustomWater");
  auto const by_index = materials::InspectMaterial(manager, manager_index);

  EXPECT_EQ(by_name.registration, Registration::Registered);
  EXPECT_EQ(by_name.manager_index, manager_index);
  EXPECT_EQ(materials::DescribeMaterial(by_name),
            materials::DescribeMaterial(by_index));
  EXPECT_TRUE(materials::DescribeMaterial(by_name).contains(
    std::format("Registration     : registered, manager material index {}",
                manager_index)));
  EXPECT_TRUE(
    materials::DescribeRegisteredMaterials(manager).contains(std::format(
      "manager material index {}: DescriptionTestCustomWater", manager_index)));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, InspectingAvailableBuiltInDoesNotRegister) {
  auto const &manager = materials::GGEMSMaterialManager::GetInstance();
  ASSERT_FALSE(manager.FindIndex("Technetium").has_value());

  auto const registered_count = manager.GetMaterials().size();

  auto const inspection = materials::InspectMaterial(manager, "Technetium");
  materials::VerboseMaterial(inspection);

  EXPECT_EQ(inspection.registration, Registration::Unregistered);
  EXPECT_FALSE(inspection.manager_index.has_value());
  EXPECT_TRUE(
    materials::DescribeMaterial(inspection)
      .contains("Registration     : available built-in, not registered"));
  EXPECT_EQ(manager.GetMaterials().size(), registered_count);
  EXPECT_FALSE(manager.FindIndex("Technetium").has_value());
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest,
     DistinctManagerEntriesCanShareOneSnapshotMaterialId) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();
  auto const water = manager.GetOrAddBuiltIn("Water");
  auto const copy = manager.AddCustomMaterial(materials::GGEMSMaterial{
    "DescriptionTestWaterCopy",
    1.0_g_cm3,
    {
      {.atomic_number = 1U, .mass_fraction = 0.111898L},
      {.atomic_number = 8U, .mass_fraction = 0.888102L},
    },
  });

  ASSERT_NE(water, copy);
  ASSERT_TRUE(materials::HasSameScientificIdentity(manager.Require(water),
                                                   manager.Require(copy)));

  materials::GGEMSEMMaterialPackage const package{manager.GetMaterials()};
  EXPECT_EQ(package.GetMaterialIds()[water], package.GetMaterialIds()[copy]);

  // A Material report names its manager index and never a Material ID.
  auto const description =
    materials::DescribeMaterial(materials::InspectMaterial(manager, copy));
  EXPECT_TRUE(
    description.contains(std::format("manager material index {}", copy)));
  EXPECT_FALSE(description.contains("Material ID"));
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, UnknownMaterialsAreRejected) {
  auto const &manager = materials::GGEMSMaterialManager::GetInstance();
  auto const registered_count = manager.GetMaterials().size();

  EXPECT_THROW(
    static_cast<void>(materials::InspectMaterial(manager, "Unobtainable")),
    ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(materials::InspectMaterial(
                 manager, static_cast<std::uint32_t>(registered_count))),
               ggems::core::GGEMSRecoverable);
  EXPECT_EQ(manager.GetMaterials().size(), registered_count);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesAvailableMaterials) {
  auto const description = materials::DescribeAvailableMaterials();

  EXPECT_TRUE(description.contains("Available built-in Materials: 117"));
  EXPECT_TRUE(description.contains("Vacuum"));
  EXPECT_TRUE(description.contains("Hydrogen"));
  EXPECT_TRUE(description.contains("Uranium"));
  EXPECT_TRUE(description.contains("Brain"));
  EXPECT_TRUE(description.contains("LSO"));
  EXPECT_TRUE(description.contains("CdTe"));
}
