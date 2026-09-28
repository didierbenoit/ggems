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
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/processes/GGEMSProductionCutConverter.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace processes = ggems::core::processes;
namespace units = ggems::units;

using namespace ggems::units;
using Channel = processes::GGEMSProductionCutChannel;

constexpr std::uint32_t k_water{0U};
constexpr std::uint32_t k_aluminum{1U};
constexpr std::uint32_t k_tungsten{2U};
constexpr std::uint32_t k_vacuum{3U};

auto MakeMaterialPackage() -> materials::GGEMSEMMaterialPackage {
  std::vector<materials::GGEMSMaterial> const list{
    materials::builtins::BuildBuiltInMaterial("Water"),
    materials::builtins::BuildBuiltInMaterial("Aluminum"),
    materials::builtins::BuildBuiltInMaterial("Tungsten"),
    materials::builtins::BuildBuiltInMaterial("Vacuum"),
  };
  return materials::GGEMSEMMaterialPackage{list};
}

auto ChannelLabel(Channel channel) -> std::string_view {
  switch (channel) {
  case Channel::Gamma:
    return "Gamma";
  case Channel::Electron:
    return "Electron";
  case Channel::Positron:
    return "Positron";
  case Channel::Proton:
    return "Proton";
  }
  return "?";
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, ReturnsCanonicalEnergy) {
  static_assert(
    std::is_same_v<decltype(processes::ConvertProductionCutLength(
                     Channel::Proton, units::Length{},
                     std::declval<materials::GGEMSEMMaterialPackage const &>(),
                     0U)),
                   units::Energy>);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest,
     SameLengthIsMaterialDependentExceptProton) {
  auto const package = MakeMaterialPackage();

  for (auto const channel : processes::k_production_cut_channels) {
    SCOPED_TRACE(ChannelLabel(channel));

    auto const water =
      processes::ConvertProductionCutLength(channel, 1_mm, package, k_water);
    auto const aluminum =
      processes::ConvertProductionCutLength(channel, 1_mm, package, k_aluminum);
    auto const tungsten =
      processes::ConvertProductionCutLength(channel, 1_mm, package, k_tungsten);

    if (channel == Channel::Proton) {
      EXPECT_EQ(water, aluminum);
      EXPECT_EQ(water, tungsten);
    } else {
      EXPECT_LT(water, aluminum);
      EXPECT_LT(aluminum, tungsten);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, EnergyIncreasesWithCutLength) {
  auto const package = MakeMaterialPackage();

  for (auto const material_id : {k_water, k_aluminum, k_tungsten}) {
    for (auto const channel : processes::k_production_cut_channels) {
      SCOPED_TRACE(ChannelLabel(channel));
      SCOPED_TRACE(material_id);

      auto const shorter = processes::ConvertProductionCutLength(
        channel, 1_mm, package, material_id);
      auto const longer = processes::ConvertProductionCutLength(
        channel, 10_mm, package, material_id);

      EXPECT_GT(shorter.value, 0U);
      EXPECT_GT(longer, shorter);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, ProtonIsExactLinearConvention) {
  auto const package = MakeMaterialPackage();

  for (auto const material_id : {k_water, k_tungsten, k_vacuum}) {
    EXPECT_EQ(processes::ConvertProductionCutLength(Channel::Proton, 1_mm,
                                                    package, material_id),
              100_keV);
    EXPECT_EQ(processes::ConvertProductionCutLength(Channel::Proton, 0_pm,
                                                    package, material_id),
              0_eV);
    EXPECT_EQ(processes::ConvertProductionCutLength(Channel::Proton, 1_pm,
                                                    package, material_id)
                .value,
              100U);
    EXPECT_EQ(processes::ConvertProductionCutLength(Channel::Proton, 25_mm,
                                                    package, material_id),
              2500_keV);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, ProtonOverflowIsRejected) {
  auto const package = MakeMaterialPackage();

  EXPECT_THROW(static_cast<void>(processes::ConvertProductionCutLength(
                 Channel::Proton, 1000_km, package, k_water)),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, OutOfDomainLengthsAreRejected) {
  auto const package = MakeMaterialPackage();

  for (auto const channel :
       {Channel::Gamma, Channel::Electron, Channel::Positron}) {
    for (auto const material_id : {k_water, k_aluminum, k_tungsten}) {
      SCOPED_TRACE(std::string{ChannelLabel(channel)} + " material " +
                   std::to_string(material_id));

      EXPECT_THROW(static_cast<void>(processes::ConvertProductionCutLength(
                     channel, 0_pm, package, material_id)),
                   ggems::core::GGEMSRecoverable);
      EXPECT_THROW(static_cast<void>(processes::ConvertProductionCutLength(
                     channel, 1_nm, package, material_id)),
                   ggems::core::GGEMSRecoverable);
      EXPECT_THROW(static_cast<void>(processes::ConvertProductionCutLength(
                     channel, 1000_km, package, material_id)),
                   ggems::core::GGEMSRecoverable);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, MatterlessMaterialIsRejected) {
  auto const package = MakeMaterialPackage();

  for (auto const channel :
       {Channel::Gamma, Channel::Electron, Channel::Positron}) {
    SCOPED_TRACE(ChannelLabel(channel));
    EXPECT_THROW(static_cast<void>(processes::ConvertProductionCutLength(
                   channel, 1_mm, package, k_vacuum)),
                 ggems::core::GGEMSRecoverable);
  }
}
