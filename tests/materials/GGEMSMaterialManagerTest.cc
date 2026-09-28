// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

namespace materials = ggems::core::materials;

using namespace ggems::units;

} // namespace

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

  auto const first_index = manager.GetOrAdd("Brain");
  auto const size_after_first = manager.GetMaterials().size();

  auto const second_index = manager.GetOrAdd("Brain");

  EXPECT_EQ(second_index, first_index);
  EXPECT_EQ(manager.GetMaterials().size(), size_after_first);
  EXPECT_EQ(manager.Require(first_index).GetName(), "Brain");
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, DefinesCustomMaterialAndRegistersItOnDemand) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  auto const registered_before = manager.GetMaterials().size();
  auto const custom_before = manager.GetCustomMaterials().size();

  manager.AddCustomMaterial(materials::GGEMSMaterial{
    "GGEMSTestCustomMaterial",
    5.0_g_cm3,
    {
      {.atomic_number = 48U, .mass_fraction = 0.5L},
      {.atomic_number = 52U, .mass_fraction = 0.5L},
    },
  });

  EXPECT_EQ(manager.GetCustomMaterials().size(), custom_before + 1U);
  EXPECT_EQ(manager.GetMaterials().size(), registered_before);
  EXPECT_FALSE(manager.FindIndex("GGEMSTestCustomMaterial").has_value());
  EXPECT_EQ(manager.Find("GGEMSTestCustomMaterial"), nullptr);

  auto const *definition = manager.FindCustom("GGEMSTestCustomMaterial");

  ASSERT_NE(definition, nullptr);
  EXPECT_EQ(definition->GetName(), "GGEMSTestCustomMaterial");
  EXPECT_EQ(definition->GetDensity(), 5.0_g_cm3);

  auto const material_index = manager.GetOrAdd("GGEMSTestCustomMaterial");

  EXPECT_EQ(material_index, registered_before);
  EXPECT_EQ(manager.GetMaterials().size(), registered_before + 1U);
  EXPECT_EQ(manager.Require(material_index).GetName(),
            "GGEMSTestCustomMaterial");

  auto const found_index = manager.FindIndex("GGEMSTestCustomMaterial");

  ASSERT_TRUE(found_index.has_value());
  EXPECT_EQ(*found_index, material_index);

  auto const *found = manager.Find("GGEMSTestCustomMaterial");

  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found, &manager.Require(material_index));
  EXPECT_EQ(found->GetDensity(), 5.0_g_cm3);

  EXPECT_EQ(manager.GetOrAdd("GGEMSTestCustomMaterial"), material_index);
  EXPECT_EQ(manager.GetMaterials().size(), registered_before + 1U);
  EXPECT_EQ(manager.GetCustomMaterials().size(), custom_before + 1U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsDuplicateCustomMaterial) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  manager.AddCustomMaterial(materials::GGEMSMaterial{
    "GGEMSTestDuplicateMaterial",
    1.0_g_cm3,
    {{.atomic_number = 6U, .mass_fraction = 1.0L}},
  });

  auto const registered_before = manager.GetMaterials().size();
  auto const custom_before = manager.GetCustomMaterials().size();

  EXPECT_THROW(manager.AddCustomMaterial(materials::GGEMSMaterial{
                 "GGEMSTestDuplicateMaterial",
                 2.0_g_cm3,
                 {{.atomic_number = 8U, .mass_fraction = 1.0L}},
               }),
               ggems::core::GGEMSRecoverable);

  EXPECT_EQ(manager.GetMaterials().size(), registered_before);
  EXPECT_EQ(manager.GetCustomMaterials().size(), custom_before);

  auto const *definition = manager.FindCustom("GGEMSTestDuplicateMaterial");

  ASSERT_NE(definition, nullptr);
  EXPECT_EQ(definition->GetDensity(), 1.0_g_cm3);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsBuiltInNameForCustomMaterial) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  auto const registered_before = manager.GetMaterials().size();
  auto const custom_before = manager.GetCustomMaterials().size();

  EXPECT_THROW(manager.AddCustomMaterial(materials::GGEMSMaterial{
                 "Water",
                 1.0_g_cm3,
                 {
                   {.atomic_number = 1U, .mass_fraction = 0.1L},
                   {.atomic_number = 8U, .mass_fraction = 0.9L},
                 },
               }),
               ggems::core::GGEMSRecoverable);

  EXPECT_EQ(manager.GetMaterials().size(), registered_before);
  EXPECT_EQ(manager.GetCustomMaterials().size(), custom_before);
  EXPECT_EQ(manager.FindCustom("Water"), nullptr);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsUnknownMaterialWithoutMutation) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  auto const registered_before = manager.GetMaterials().size();
  auto const custom_before = manager.GetCustomMaterials().size();

  EXPECT_THROW(static_cast<void>(manager.GetOrAdd("Unobtainium")),
               ggems::core::GGEMSRecoverable);

  EXPECT_EQ(manager.GetMaterials().size(), registered_before);
  EXPECT_EQ(manager.GetCustomMaterials().size(), custom_before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialManagerTest, RejectsUnknownLookup) {
  auto &manager = materials::GGEMSMaterialManager::GetInstance();

  EXPECT_FALSE(manager.FindIndex("GGEMSTestUnknownMaterial").has_value());
  EXPECT_EQ(manager.Find("GGEMSTestUnknownMaterial"), nullptr);
  EXPECT_EQ(manager.FindCustom("GGEMSTestUnknownMaterial"), nullptr);

  auto const invalid_index =
    static_cast<std::uint32_t>(manager.GetMaterials().size());

  EXPECT_THROW(static_cast<void>(manager.Require(invalid_index)),
               ggems::core::GGEMSRecoverable);
}
