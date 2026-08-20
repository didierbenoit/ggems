#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

namespace materials = ggems::core::materials;

using namespace ggems::units;

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, IsProcessWideSingleton) {
  auto &first = materials::GGEMSMaterialManager::GetInstance();
  auto &second = materials::GGEMSMaterialManager::GetInstance();

  EXPECT_EQ(&first, &second);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, GetsOrAddsBuiltInOnce) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  auto const first_index = manager.GetOrAddBuiltIn("Brain");
  auto const size_after_first = manager.GetMaterials().size();

  auto const second_index = manager.GetOrAddBuiltIn("Brain");

  EXPECT_EQ(second_index, first_index);
  EXPECT_EQ(manager.GetMaterials().size(), size_after_first);
  EXPECT_EQ(manager.Require(first_index).GetName(), "Brain");
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, AddsCustomMaterial) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  auto const size_before = manager.GetMaterials().size();

  auto const material_index =
      manager.AddCustomMaterial(materials::GGEMSMaterial{
          "GGEMSTestCustomMaterial",
          5.0_g_cm3,
          {{.atomic_number = 48U, .mass_fraction = 0.5L},
           {.atomic_number = 52U, .mass_fraction = 0.5L}}});

  EXPECT_EQ(material_index, size_before);
  EXPECT_EQ(manager.GetMaterials().size(), size_before + 1U);
  EXPECT_EQ(manager.Require(material_index).GetName(),
            "GGEMSTestCustomMaterial");

  auto const found_index = manager.FindIndex("GGEMSTestCustomMaterial");

  ASSERT_TRUE(found_index.has_value());
  EXPECT_EQ(*found_index, material_index);

  auto const *found = manager.Find("GGEMSTestCustomMaterial");

  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->GetName(), "GGEMSTestCustomMaterial");
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsDuplicateCustomMaterial) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  static_cast<void>(manager.AddCustomMaterial(materials::GGEMSMaterial{
      "GGEMSTestDuplicateMaterial",
      1.0_g_cm3,
      {{.atomic_number = 6U, .mass_fraction = 1.0L}}}));

  auto const size_before = manager.GetMaterials().size();

  EXPECT_THROW(
      static_cast<void>(manager.AddCustomMaterial(materials::GGEMSMaterial{
          "GGEMSTestDuplicateMaterial",
          2.0_g_cm3,
          {{.atomic_number = 8U, .mass_fraction = 1.0L}}})),
      ggems::core::GGEMSRecoverable);

  EXPECT_EQ(manager.GetMaterials().size(), size_before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsBuiltInNameForCustomMaterial) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  auto const size_before = manager.GetMaterials().size();

  EXPECT_THROW(
      static_cast<void>(manager.AddCustomMaterial(materials::GGEMSMaterial{
          "Water",
          1.0_g_cm3,
          {{.atomic_number = 1U, .mass_fraction = 0.1L},
           {.atomic_number = 8U, .mass_fraction = 0.9L}}})),
      ggems::core::GGEMSRecoverable);

  EXPECT_EQ(manager.GetMaterials().size(), size_before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsUnknownBuiltInWithoutMutation) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  auto const size_before = manager.GetMaterials().size();

  EXPECT_THROW(static_cast<void>(manager.GetOrAddBuiltIn("Unobtainium")),
               ggems::core::GGEMSRecoverable);

  EXPECT_EQ(manager.GetMaterials().size(), size_before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsUnknownLookup) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  EXPECT_FALSE(manager.FindIndex("GGEMSTestUnknownMaterial").has_value());

  EXPECT_EQ(manager.Find("GGEMSTestUnknownMaterial"), nullptr);

  EXPECT_THROW(static_cast<void>(manager.Require("GGEMSTestUnknownMaterial")),
               ggems::core::GGEMSRecoverable);
}

} // namespace
