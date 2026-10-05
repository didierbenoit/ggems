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
 * \brief Tests the independently selected Ac-225 source contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"

namespace {

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::builtins::BuildAc225Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSAc225Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildAc225Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Ac-225");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 856846.0800L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 94U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // alpha
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Alpha);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.99956647L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // nuclear_gamma
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.07457414L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.22586L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_10_79_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 0.0715L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_25_856_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 0.0971L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_36_646_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 0.1990L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_38_546_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[6U].GetYieldPerDecay(), 0.09167L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_46_159_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[7U].GetYieldPerDecay(), 0.00004136L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_49_166_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[8U].GetYieldPerDecay(), 0.00005695L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_50_311_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[9U].GetYieldPerDecay(), 0.00144774L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_53_036_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[10U].GetYieldPerDecay(), 0.00071513L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_57_762_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[11U].GetYieldPerDecay(), 0.00002355L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_62_351_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[12U].GetYieldPerDecay(), 0.00438L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_62_95_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[13U].GetYieldPerDecay(), 0.052992L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_63_106_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[14U].GetYieldPerDecay(), 0.0000768L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_64_251_keV
  EXPECT_EQ(emissions[15U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[15U].GetYieldPerDecay(), 0.01081L);
  EXPECT_EQ(emissions[15U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_69_858_keV
  EXPECT_EQ(emissions[16U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[16U].GetYieldPerDecay(), 0.002249L);
  EXPECT_EQ(emissions[16U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_71_758_keV
  EXPECT_EQ(emissions[17U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[17U].GetYieldPerDecay(), 0.005559L);
  EXPECT_EQ(emissions[17U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_73_74_keV
  EXPECT_EQ(emissions[18U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[18U].GetYieldPerDecay(), 0.007029L);
  EXPECT_EQ(emissions[18U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_73_896_keV
  EXPECT_EQ(emissions[19U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[19U].GetYieldPerDecay(), 0.0007392L);
  EXPECT_EQ(emissions[19U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_75_041_keV
  EXPECT_EQ(emissions[20U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[20U].GetYieldPerDecay(), 0.00180L);
  EXPECT_EQ(emissions[20U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_78_812_keV
  EXPECT_EQ(emissions[21U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[21U].GetYieldPerDecay(), 0.0006915L);
  EXPECT_EQ(emissions[21U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_87_385_keV
  EXPECT_EQ(emissions[22U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[22U].GetYieldPerDecay(), 0.0113118L);
  EXPECT_EQ(emissions[22U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_94_892_keV
  EXPECT_EQ(emissions[23U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[23U].GetYieldPerDecay(), 0.0034309L);
  EXPECT_EQ(emissions[23U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_96_037_keV
  EXPECT_EQ(emissions[24U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[24U].GetYieldPerDecay(), 0.002004L);
  EXPECT_EQ(emissions[24U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_99_596_keV
  EXPECT_EQ(emissions[25U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[25U].GetYieldPerDecay(), 0.02324L);
  EXPECT_EQ(emissions[25U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_99_752_keV
  EXPECT_EQ(emissions[26U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[26U].GetYieldPerDecay(), 0.001161L);
  EXPECT_EQ(emissions[26U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_100_897_keV
  EXPECT_EQ(emissions[27U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[27U].GetYieldPerDecay(), 0.00433L);
  EXPECT_EQ(emissions[27U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_103_488_keV
  EXPECT_EQ(emissions[28U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[28U].GetYieldPerDecay(), 0.0003093L);
  EXPECT_EQ(emissions[28U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_108_404_keV
  EXPECT_EQ(emissions[29U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[29U].GetYieldPerDecay(), 0.026191L);
  EXPECT_EQ(emissions[29U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_111_517_keV
  EXPECT_EQ(emissions[30U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[30U].GetYieldPerDecay(), 0.0011309L);
  EXPECT_EQ(emissions[30U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_112_78_keV
  EXPECT_EQ(emissions[31U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[31U].GetYieldPerDecay(), 0.000007436L);
  EXPECT_EQ(emissions[31U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_114_091_keV
  EXPECT_EQ(emissions[32U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[32U].GetYieldPerDecay(), 0.000085781L);
  EXPECT_EQ(emissions[32U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_119_899_keV
  EXPECT_EQ(emissions[33U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[33U].GetYieldPerDecay(), 0.00024293L);
  EXPECT_EQ(emissions[33U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_121_08_keV
  EXPECT_EQ(emissions[34U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[34U].GetYieldPerDecay(), 0.00005103L);
  EXPECT_EQ(emissions[34U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_123_67_keV
  EXPECT_EQ(emissions[35U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[35U].GetYieldPerDecay(), 0.00024603L);
  EXPECT_EQ(emissions[35U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_124_815_keV
  EXPECT_EQ(emissions[36U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[36U].GetYieldPerDecay(), 0.001753L);
  EXPECT_EQ(emissions[36U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_126_066_keV
  EXPECT_EQ(emissions[37U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[37U].GetYieldPerDecay(), 0.00002056272L);
  EXPECT_EQ(emissions[37U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_129_146_keV
  EXPECT_EQ(emissions[38U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[38U].GetYieldPerDecay(), 0.0001344L);
  EXPECT_EQ(emissions[38U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_133_573_keV
  EXPECT_EQ(emissions[39U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[39U].GetYieldPerDecay(), 0.00004289318L);
  EXPECT_EQ(emissions[39U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_134_874_keV
  EXPECT_EQ(emissions[40U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[40U].GetYieldPerDecay(), 0.00006852504L);
  EXPECT_EQ(emissions[40U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_139_749_keV
  EXPECT_EQ(emissions[41U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[41U].GetYieldPerDecay(), 0.00005436L);
  EXPECT_EQ(emissions[41U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_144_627_keV
  EXPECT_EQ(emissions[42U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[42U].GetYieldPerDecay(), 0.00001746L);
  EXPECT_EQ(emissions[42U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_145_147_keV
  EXPECT_EQ(emissions[43U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[43U].GetYieldPerDecay(), 0.0002611684L);
  EXPECT_EQ(emissions[43U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_150_063_keV
  EXPECT_EQ(emissions[44U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[44U].GetYieldPerDecay(), 0.0010302417L);
  EXPECT_EQ(emissions[44U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_152_654_keV
  EXPECT_EQ(emissions[45U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[45U].GetYieldPerDecay(), 0.00003120912L);
  EXPECT_EQ(emissions[45U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_153_955_keV
  EXPECT_EQ(emissions[46U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[46U].GetYieldPerDecay(), 0.0003180615L);
  EXPECT_EQ(emissions[46U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_157_243_keV
  EXPECT_EQ(emissions[47U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[47U].GetYieldPerDecay(), 0.014035L);
  EXPECT_EQ(emissions[47U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_161_35_keV
  EXPECT_EQ(emissions[48U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[48U].GetYieldPerDecay(), 0.00008867L);
  EXPECT_EQ(emissions[48U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_168_733_keV
  EXPECT_EQ(emissions[49U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[49U].GetYieldPerDecay(), 0.0002573L);
  EXPECT_EQ(emissions[49U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_170_805_keV
  EXPECT_EQ(emissions[50U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[50U].GetYieldPerDecay(), 0.00001542099L);
  EXPECT_EQ(emissions[50U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_178_312_keV
  EXPECT_EQ(emissions[51U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[51U].GetYieldPerDecay(), 0.000017619077L);
  EXPECT_EQ(emissions[51U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_179_756_keV
  EXPECT_EQ(emissions[52U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[52U].GetYieldPerDecay(), 0.00019044L);
  EXPECT_EQ(emissions[52U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_186_286_keV
  EXPECT_EQ(emissions[53U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[53U].GetYieldPerDecay(), 0.000004142246L);
  EXPECT_EQ(emissions[53U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_187_921_keV
  EXPECT_EQ(emissions[54U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[54U].GetYieldPerDecay(), 0.0005123102L);
  EXPECT_EQ(emissions[54U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_195_789_keV
  EXPECT_EQ(emissions[55U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[55U].GetYieldPerDecay(), 0.0022141L);
  EXPECT_EQ(emissions[55U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_197_511_keV
  EXPECT_EQ(emissions[56U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[56U].GetYieldPerDecay(), 0.00002003460L);
  EXPECT_EQ(emissions[56U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_197_824_keV
  EXPECT_EQ(emissions[57U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[57U].GetYieldPerDecay(), 0.00002915524L);
  EXPECT_EQ(emissions[57U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_198_711_keV
  EXPECT_EQ(emissions[58U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[58U].GetYieldPerDecay(), 0.000014308563L);
  EXPECT_EQ(emissions[58U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_216_905_keV
  EXPECT_EQ(emissions[59U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[59U].GetYieldPerDecay(), 0.00019707511L);
  EXPECT_EQ(emissions[59U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_224_567_keV
  EXPECT_EQ(emissions[60U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[60U].GetYieldPerDecay(), 0.00006364123L);
  EXPECT_EQ(emissions[60U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_231_196_keV
  EXPECT_EQ(emissions[61U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[61U].GetYieldPerDecay(), 0.000067050057L);
  EXPECT_EQ(emissions[61U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_238_64_keV
  EXPECT_EQ(emissions[62U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[62U].GetYieldPerDecay(), 0.0000122470104L);
  EXPECT_EQ(emissions[62U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_240_663_keV
  EXPECT_EQ(emissions[63U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[63U].GetYieldPerDecay(), 0.000005661996L);
  EXPECT_EQ(emissions[63U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_243_237_keV
  EXPECT_EQ(emissions[64U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[64U].GetYieldPerDecay(), 0.00003590000305L);
  EXPECT_EQ(emissions[64U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_249_614_keV
  EXPECT_EQ(emissions[65U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[65U].GetYieldPerDecay(), 0.00003475L);
  EXPECT_EQ(emissions[65U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_253_551_keV
  EXPECT_EQ(emissions[66U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[66U].GetYieldPerDecay(), 0.00005658968L);
  EXPECT_EQ(emissions[66U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_256_144_keV
  EXPECT_EQ(emissions[67U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[67U].GetYieldPerDecay(), 1.5545375E-7L);
  EXPECT_EQ(emissions[67U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_279_209_keV
  EXPECT_EQ(emissions[68U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[68U].GetYieldPerDecay(), 0.0000104665993L);
  EXPECT_EQ(emissions[68U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_282_201_keV
  EXPECT_EQ(emissions[69U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[69U].GetYieldPerDecay(), 0.000004227400351L);
  EXPECT_EQ(emissions[69U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_284_896_keV
  EXPECT_EQ(emissions[70U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[70U].GetYieldPerDecay(), 0.00000243070808L);
  EXPECT_EQ(emissions[70U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_298_33_keV
  EXPECT_EQ(emissions[71U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[71U].GetYieldPerDecay(), 0.000008034L);
  EXPECT_EQ(emissions[71U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_317_119_keV
  EXPECT_EQ(emissions[72U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[72U].GetYieldPerDecay(), 0.00000231703519L);
  EXPECT_EQ(emissions[72U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_321_753_keV
  EXPECT_EQ(emissions[73U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[73U].GetYieldPerDecay(), 8.2370578E-7L);
  EXPECT_EQ(emissions[73U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_354_754_keV
  EXPECT_EQ(emissions[74U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[74U].GetYieldPerDecay(), 4.0115172E-7L);
  EXPECT_EQ(emissions[74U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_362_394_keV
  EXPECT_EQ(emissions[75U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[75U].GetYieldPerDecay(), 0.00000103423439L);
  EXPECT_EQ(emissions[75U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_367_74_keV
  EXPECT_EQ(emissions[76U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[76U].GetYieldPerDecay(), 1.767E-7L);
  EXPECT_EQ(emissions[76U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_374_881_keV
  EXPECT_EQ(emissions[77U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[77U].GetYieldPerDecay(), 3.3764440E-7L);
  EXPECT_EQ(emissions[77U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_406_057_keV
  EXPECT_EQ(emissions[78U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[78U].GetYieldPerDecay(), 0.00000118065258L);
  EXPECT_EQ(emissions[78U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_443_408_keV
  EXPECT_EQ(emissions[79U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[79U].GetYieldPerDecay(), 4.95192E-7L);
  EXPECT_EQ(emissions[79U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_450_915_keV
  EXPECT_EQ(emissions[80U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[80U].GetYieldPerDecay(), 0.0000063350945L);
  EXPECT_EQ(emissions[80U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_452_216_keV
  EXPECT_EQ(emissions[81U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[81U].GetYieldPerDecay(), 0.0002250333376L);
  EXPECT_EQ(emissions[81U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_462_266_keV
  EXPECT_EQ(emissions[82U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[82U].GetYieldPerDecay(), 5.0566579E-8L);
  EXPECT_EQ(emissions[82U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_515_165_keV
  EXPECT_EQ(emissions[83U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[83U].GetYieldPerDecay(), 0.00003173646444L);
  EXPECT_EQ(emissions[83U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_525_955_keV
  EXPECT_EQ(emissions[84U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[84U].GetYieldPerDecay(), 0.0000495157238L);
  EXPECT_EQ(emissions[84U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_532_123_keV
  EXPECT_EQ(emissions[85U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[85U].GetYieldPerDecay(), 6.6298710E-8L);
  EXPECT_EQ(emissions[85U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_551_811_keV
  EXPECT_EQ(emissions[86U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[86U].GetYieldPerDecay(), 0.000006432093049L);
  EXPECT_EQ(emissions[86U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_570_669_keV
  EXPECT_EQ(emissions[87U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[87U].GetYieldPerDecay(), 3.019044949E-7L);
  EXPECT_EQ(emissions[87U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_637_599_keV
  EXPECT_EQ(emissions[88U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[88U].GetYieldPerDecay(), 1.0976E-8L);
  EXPECT_EQ(emissions[88U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_645_94_keV
  EXPECT_EQ(emissions[89U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[89U].GetYieldPerDecay(), 1.3334E-8L);
  EXPECT_EQ(emissions[89U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_766_44_keV
  EXPECT_EQ(emissions[90U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[90U].GetYieldPerDecay(), 1.8726E-8L);
  EXPECT_EQ(emissions[90U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_779_32_keV
  EXPECT_EQ(emissions[91U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[91U].GetYieldPerDecay(), 3.315E-9L);
  EXPECT_EQ(emissions[91U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_808_48_keV
  EXPECT_EQ(emissions[92U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[92U].GetYieldPerDecay(), 1.1540E-7L);
  EXPECT_EQ(emissions[92U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_825_keV
  EXPECT_EQ(emissions[93U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[93U].GetYieldPerDecay(), 2.6398E-9L);
  EXPECT_EQ(emissions[93U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_NEAR(definition.GetTotalYieldPerDecay(), 1.9435789610139639L,
              1.0e-14L);
}

TEST(GGEMSAc225Test, PreservesAlpha) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 49U> expected_energies{
    4'903'600'000'000ULL, 4'992'700'000'000ULL, 5'019'300'000'000ULL,
    5'025'500'000'000ULL, 5'035'500'000'000ULL, 5'064'100'000'000ULL,
    5'076'800'000'000ULL, 5'094'100'000'000ULL, 5'129'000'000'000ULL,
    5'162'100'000'000ULL, 5'195'100'000'000ULL, 5'203'300'000'000ULL,
    5'210'200'000'000ULL, 5'239'300'000'000ULL, 5'269'100'000'000ULL,
    5'287'600'000'000ULL, 5'321'200'000'000ULL, 5'341'900'000'000ULL,
    5'356'200'000'000ULL, 5'379'000'000'000ULL, 5'391'200'000'000ULL,
    5'414'500'000'000ULL, 5'428'300'000'000ULL, 5'430'100'000'000ULL,
    5'435'800'000'000ULL, 5'443'300'000'000ULL, 5'468'400'000'000ULL,
    5'487'400'000'000ULL, 5'497'400'000'000ULL, 5'515'200'000'000ULL,
    5'523'700'000'000ULL, 5'540'100'000'000ULL, 5'546'500'000'000ULL,
    5'555'300'000'000ULL, 5'563'300'000'000ULL, 5'580'500'000'000ULL,
    5'599'300'000'000ULL, 5'609'000'000'000ULL, 5'637'300'000'000ULL,
    5'682'200'000'000ULL, 5'686'400'000'000ULL, 5'723'100'000'000ULL,
    5'730'500'000'000ULL, 5'731'600'000'000ULL, 5'731'900'000'000ULL,
    5'791'700'000'000ULL, 5'793'100'000'000ULL, 5'804'200'000'000ULL,
    5'829'600'000'000ULL,
  };
  constexpr std::array<double, 49U> expected_weights{
    1.1e-05, 1.3e-05, 1.5e-06, 8.3e-06, 2.1e-05,  1.14e-05, 3.8e-05,
    0.00015, 5.8e-05, 6.6e-06, 1.5e-06, 0.000101, 0.00022,  2.6e-05,
    0.00048, 0.00214, 7e-05,   2.7e-05, 9.7e-07,  2e-05,    6e-06,
    3e-05,   2.3e-05, 2.8e-05, 8.3e-05, 0.00098,  5.2e-06,  2e-05,
    2.2e-05, 5.2e-05, 0.00013, 7.2e-05, 0.00055,  0.00084,  0.00017,
    0.0095,  0.00114, 0.0109,  0.0416,  0.0131,   0.00021,  0.0203,
    0.016,   0.0124,  0.09,    0.062,   0.189,    0.003,    0.524,
  };
  ASSERT_EQ(energies.size(), 49U);
  EXPECT_EQ(distribution.GetTableCount(), 49U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.99956647L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 5788.5953114693812L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 5788.5953114693812L,
              1.0565457625150681e-05L);
}

TEST(GGEMSAc225Test, PreservesNuclearGamma) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 140U> expected_energies{
    10'790'000'000ULL,  25'856'000'000ULL,  36'646'000'000ULL,
    38'546'000'000ULL,  46'159'000'000ULL,  49'166'000'000ULL,
    50'311'000'000ULL,  53'036'000'000ULL,  57'762'000'000ULL,
    62'351'000'000ULL,  62'950'000'000ULL,  63'106'000'000ULL,
    64'251'000'000ULL,  69'858'000'000ULL,  71'758'000'000ULL,
    73'740'000'000ULL,  73'896'000'000ULL,  75'041'000'000ULL,
    78'812'000'000ULL,  87'385'000'000ULL,  94'892'000'000ULL,
    96'037'000'000ULL,  99'596'000'000ULL,  99'752'000'000ULL,
    100'897'000'000ULL, 103'488'000'000ULL, 108'404'000'000ULL,
    111'517'000'000ULL, 112'780'000'000ULL, 114'091'000'000ULL,
    119'899'000'000ULL, 121'080'000'000ULL, 123'670'000'000ULL,
    124'815'000'000ULL, 126'066'000'000ULL, 129'146'000'000ULL,
    133'573'000'000ULL, 134'874'000'000ULL, 137'400'000'000ULL,
    139'749'000'000ULL, 144'627'000'000ULL, 145'147'000'000ULL,
    150'063'000'000ULL, 152'654'000'000ULL, 153'955'000'000ULL,
    157'243'000'000ULL, 161'350'000'000ULL, 168'733'000'000ULL,
    169'933'000'000ULL, 170'805'000'000ULL, 178'312'000'000ULL,
    179'756'000'000ULL, 186'021'000'000ULL, 186'286'000'000ULL,
    187'263'000'000ULL, 187'921'000'000ULL, 195'789'000'000ULL,
    197'511'000'000ULL, 197'824'000'000ULL, 198'711'000'000ULL,
    205'190'000'000ULL, 216'905'000'000ULL, 220'430'000'000ULL,
    224'567'000'000ULL, 227'695'000'000ULL, 231'196'000'000ULL,
    234'490'000'000ULL, 238'640'000'000ULL, 240'663'000'000ULL,
    243'237'000'000ULL, 249'614'000'000ULL, 253'551'000'000ULL,
    256'144'000'000ULL, 279'209'000'000ULL, 282'201'000'000ULL,
    284'896'000'000ULL, 298'330'000'000ULL, 317'119'000'000ULL,
    321'753'000'000ULL, 348'350'000'000ULL, 354'754'000'000ULL,
    356'023'000'000ULL, 362'394'000'000ULL, 367'740'000'000ULL,
    374'881'000'000ULL, 388'100'000'000ULL, 403'130'000'000ULL,
    406'057'000'000ULL, 417'882'000'000ULL, 429'430'000'000ULL,
    434'762'000'000ULL, 442'100'000'000ULL, 443'408'000'000ULL,
    443'430'000'000ULL, 446'310'000'000ULL, 450'915'000'000ULL,
    452'216'000'000ULL, 458'740'000'000ULL, 462'266'000'000ULL,
    469'773'000'000ULL, 480'988'000'000ULL, 491'778'000'000ULL,
    496'500'000'000ULL, 498'600'000'000ULL, 513'265'000'000ULL,
    515'165'000'000ULL, 517'633'000'000ULL, 522'146'000'000ULL,
    525'955'000'000ULL, 529'653'000'000ULL, 530'954'000'000ULL,
    532'123'000'000ULL, 538'003'000'000ULL, 544'850'000'000ULL,
    551'811'000'000ULL, 564'293'000'000ULL, 567'480'000'000ULL,
    570'669'000'000ULL, 592'004'000'000ULL, 593'904'000'000ULL,
    600'939'000'000ULL, 600'953'000'000ULL, 603'074'000'000ULL,
    629'260'000'000ULL, 637'599'000'000ULL, 645'940'000'000ULL,
    649'077'000'000ULL, 656'290'000'000ULL, 658'030'000'000ULL,
    666'840'000'000ULL, 674'940'000'000ULL, 679'530'000'000ULL,
    702'020'000'000ULL, 747'000'000'000ULL, 752'480'000'000ULL,
    753'460'000'000ULL, 766'440'000'000ULL, 779'320'000'000ULL,
    808'480'000'000ULL, 825'000'000'000ULL,
  };
  constexpr std::array<double, 140U> expected_weights{
    0.00015,  1.59e-05, 0.000181, 0.000107, 4.9e-05,  8e-05,    6.2e-06,
    4e-05,    5.1e-05,  5.3e-05,  0.0049,   0.00021,  0.00047,  4.7e-05,
    0.000132, 0.00019,  0.00309,  0.00015,  0.000123, 0.00271,  0.00105,
    0.00033,  0.0076,   0.0108,   0.00096,  3e-05,    0.00255,  0.00313,
    2.1e-05,  8.7e-06,  0.0008,   0.00017,  0.00087,  0.000292, 7.9e-05,
    2.7e-05,  0.000196, 0.00032,  2.3e-05,  1.39e-05, 4.6e-06,  0.00146,
    0.00693,  0.000197, 0.00205,  0.0036,   3.6e-05,  0.00012,  0.000139,
    0.00013,  0.000161, 0.000108, 0.000127, 4.2e-05,  0.000103, 0.0053,
    0.00148,  0.00026,  0.00038,  0.000188, 1.5e-05,  0.0032,   6e-05,
    0.00112,  4.6e-05,  5e-05,    1.7e-05,  1e-05,    0.000117, 3.1e-05,
    0.000135, 0.00132,  3.7e-06,  0.000305, 5.5e-06,  7.4e-05,  2e-05,
    4.2e-06,  3.3e-05,  3e-05,    2e-05,    2.6e-06,  5.4e-05,  5.2e-06,
    1.9e-05,  1.25e-05, 1.9e-06,  7.8e-05,  5.6e-05,  3.8e-06,  2.9e-05,
    4.5e-05,  1.4e-05,  1e-06,    6e-06,    3e-05,    0.00107,  5.3e-06,
    4.4e-06,  2.8e-05,  0.00034,  3.5e-06,  1.5e-05,  8.3e-06,  5.5e-06,
    0.000214, 0.000159, 2.08e-05, 0.000353, 7.6e-05,  4.7e-05,  7.6e-06,
    3.8e-05,  5.3e-06,  5.2e-05,  2.2e-06,  1.2e-05,  4e-05,    8.3e-06,
    2.9e-05,  2.4e-05,  6e-05,    1.73e-05, 3.2e-06,  1.2e-06,  1.5e-06,
    1.7e-05,  4.9e-06,  1.4e-05,  2.1e-05,  1e-06,    6.6e-06,  1.6e-06,
    1.1e-05,  2.6e-06,  2.3e-06,  3e-06,    5.5e-07,  2.1e-05,  4.9e-07,
  };
  ASSERT_EQ(energies.size(), 140U);
  EXPECT_EQ(distribution.GetTableCount(), 140U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.07457414L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 144.69716310238374L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 144.69716310238374L,
              2.6541225371718404e-05L);
}

TEST(GGEMSAc225Test, PreservesAtomicXray) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    14'089'550'000ULL, 83'230'000'000ULL,  86'100'000'000ULL,
    97'452'700'000ULL, 100'560'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.19, 0.0106, 0.0173, 0.006, 0.00196,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.22586L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 25.81509032143806L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 25.81509032143806L,
              1.0166485265269876e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion1079Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 2U> expected_energies{
    7'042'000'000ULL,
    10'125'000'000ULL,
  };
  constexpr std::array<double, 2U> expected_weights{
    0.054,
    0.0175,
  };
  ASSERT_EQ(energies.size(), 2U);
  EXPECT_EQ(distribution.GetTableCount(), 2U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0715L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 7.7965804195804198L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 7.7965804195804198L,
              2.4356337487697601e-09L);
}

TEST(GGEMSAc225Test, PreservesConversion25856Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    7'222'000'000ULL,  7'957'000'000ULL,  10'831'000'000ULL,
    22'108'000'000ULL, 25'191'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.001, 0.0345, 0.0363, 0.0192, 0.0061,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0971L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 12.904660144181257L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 12.904660144181257L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion36646Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    18'012'000'000ULL, 18'747'000'000ULL, 21'621'000'000ULL,
    32'898'000'000ULL, 35'981'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.0021, 0.073, 0.072, 0.0395, 0.0124,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.1990L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 23.661827135678394L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 23.661827135678394L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion38546Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    19'912'000'000ULL, 20'647'000'000ULL, 23'521'000'000ULL,
    34'798'000'000ULL, 37'881'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00097, 0.0338, 0.033, 0.0182, 0.0057,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.09167L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 25.554942074833644L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 25.554942074833644L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion46159Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    27'525'000'000ULL, 28'260'000'000ULL, 31'134'000'000ULL,
    42'411'000'000ULL, 45'494'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    1.05e-05, 9.7e-06, 1.11e-05, 7.7e-06, 2.36e-06,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00004136L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 32.462583172146999L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 32.462583172146999L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion49166Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    30'532'000'000ULL, 31'267'000'000ULL, 34'141'000'000ULL,
    45'418'000'000ULL, 48'501'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    1.52e-05, 1.32e-05, 1.48e-05, 1.05e-05, 3.25e-06,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00005695L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 35.410269534679543L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 35.410269534679543L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion50311Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    31'677'000'000ULL, 32'412'000'000ULL, 35'286'000'000ULL,
    46'563'000'000ULL, 49'646'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    1.544e-05, 0.00055, 0.000503, 0.0002883, 9.1e-05,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00144774L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 37.303970174202547L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 37.303970174202547L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion53036Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    34'402'000'000ULL, 35'137'000'000ULL, 38'011'000'000ULL,
    49'288'000'000ULL, 52'371'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.000486, 5.42e-05, 3.73e-06, 0.0001296, 4.16e-05,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00071513L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 38.219536070364832L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 38.219536070364832L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion57762Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    39'128'000'000ULL, 39'863'000'000ULL, 42'737'000'000ULL,
    54'014'000'000ULL, 57'097'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    6.9e-06, 5.3e-06, 5.7e-06, 4.3e-06, 1.35e-06,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00002355L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 43.915038216560511L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 43.915038216560511L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion62351Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    43'717'000'000ULL, 44'452'000'000ULL, 47'326'000'000ULL,
    58'603'000'000ULL, 61'686'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    5e-05, 0.00171, 0.00147, 0.00087, 0.00028,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00438L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 49.320703196347033L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 49.320703196347033L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion6295Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    44'316'000'000ULL, 45'051'000'000ULL, 47'925'000'000ULL,
    59'202'000'000ULL, 62'285'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.036, 0.00403, 0.000272, 0.0096, 0.00309,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.052992L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 48.134944519927537L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 48.134944519927537L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion63106Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    44'472'000'000ULL, 45'207'000'000ULL, 48'081'000'000ULL,
    59'358'000'000ULL, 62'441'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    2.38e-05, 1.68e-05, 1.77e-05, 1.41e-05, 4.4e-06,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0000768L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 49.226993489583336L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 49.226993489583336L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion64251Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[15U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    45'617'000'000ULL, 46'352'000'000ULL, 49'226'000'000ULL,
    60'503'000'000ULL, 63'586'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00268, 0.003, 0.0024, 0.00207, 0.00066,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.01081L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 50.569836262719704L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 50.569836262719704L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion69858Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[16U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    51'224'000'000ULL, 51'959'000'000ULL, 54'833'000'000ULL,
    66'110'000'000ULL, 69'193'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    2.7e-05, 0.00089, 0.00074, 0.00045, 0.000142,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.002249L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 56.815421965317917L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 56.815421965317917L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion71758Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[17U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    53'124'000'000ULL, 53'859'000'000ULL, 56'733'000'000ULL,
    68'010'000'000ULL, 71'093'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    6.9e-05, 0.00221, 0.00182, 0.00111, 0.00035,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.005559L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 58.701503148048211L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 58.701503148048211L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion7374Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[18U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    55'106'000'000ULL, 55'841'000'000ULL, 58'715'000'000ULL,
    69'992'000'000ULL, 73'075'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    8.9e-05, 0.0028, 0.0023, 0.0014, 0.00044,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.007029L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 60.669445724854178L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 60.669445724854178L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion73896Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[19U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    55'262'000'000ULL, 55'997'000'000ULL, 58'871'000'000ULL,
    70'148'000'000ULL, 73'231'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.000249, 0.000155, 0.000157, 0.000136, 4.22e-05,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0007392L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 59.947235119047619L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 59.947235119047619L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion75041Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[20U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    56'407'000'000ULL, 57'142'000'000ULL, 60'016'000'000ULL,
    71'293'000'000ULL, 74'376'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00054, 0.00047, 0.00034, 0.00034, 0.00011,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00180L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 61.190522222222221L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 61.190522222222221L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion78812Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[21U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    60'178'000'000ULL, 60'913'000'000ULL, 63'787'000'000ULL,
    75'064'000'000ULL, 78'147'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00047, 5.3e-05, 3.5e-06, 0.000125, 4e-05,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0006915L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 63.982911785972526L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 63.982911785972526L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion87385Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[22U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    68'751'000'000ULL, 69'486'000'000ULL, 72'360'000'000ULL,
    83'637'000'000ULL, 86'720'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.0077, 0.00086, 5.58e-05, 0.00204, 0.000656,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0113118L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 72.551331176293786L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 72.551331176293786L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion94892Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[23U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    76'258'000'000ULL, 76'993'000'000ULL, 79'867'000'000ULL,
    91'144'000'000ULL, 94'227'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00233, 0.000264, 1.69e-05, 0.00062, 0.0002,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0034309L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 80.069872132676551L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 80.069872132676551L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion96037Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[24U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    77'403'000'000ULL, 78'138'000'000ULL, 81'012'000'000ULL,
    92'289'000'000ULL, 95'372'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00046, 0.00059, 0.00043, 0.0004, 0.000124,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.002004L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 82.476890219560872L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 82.476890219560872L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion99596Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[25U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    80'962'000'000ULL, 81'697'000'000ULL, 84'571'000'000ULL,
    95'848'000'000ULL, 98'931'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.0142, 0.00258, 0.00084, 0.00426, 0.00136,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.02324L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 84.954257314974186L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 84.954257314974186L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion99752Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[26U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    81'118'000'000ULL, 81'853'000'000ULL, 84'727'000'000ULL,
    96'004'000'000ULL, 99'087'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00045, 0.000222, 0.00021, 0.000213, 6.6e-05,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.001161L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 85.663850129198963L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 85.663850129198963L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion100897Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[27U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    82'263'000'000ULL, 82'998'000'000ULL,  85'872'000'000ULL,
    97'149'000'000ULL, 100'232'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.0012, 0.0012, 0.0008, 0.00086, 0.00027,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00433L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 87.210526558891459L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 87.210526558891459L,
              2.1918669179081916e-08L);
}

TEST(GGEMSAc225Test, PreservesConversion103488Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[28U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    2'358'000'000ULL,  84'854'000'000ULL, 85'589'000'000ULL,
    88'463'000'000ULL, 99'740'000'000ULL, 102'823'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00016, 2.8e-05, 4.9e-05, 3.4e-05, 2.9e-05, 9.3e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0003093L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 44.628221467830585L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 44.628221467830585L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion108404Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[29U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    7'274'000'000ULL,  89'770'000'000ULL,  90'505'000'000ULL,
    93'379'000'000ULL, 104'656'000'000ULL, 107'739'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0184, 0.00309, 0.00173, 0.00102, 0.00148, 0.000471,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.026191L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 33.167388759497534L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 33.167388759497534L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion111517Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[30U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    10'387'000'000ULL, 92'883'000'000ULL,  93'618'000'000ULL,
    96'492'000'000ULL, 107'769'000'000ULL, 110'852'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00088, 0.000102, 4.64e-05, 4.24e-05, 4.57e-05, 1.44e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0011309L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 29.685271995755592L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 29.685271995755592L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion11278Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[31U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    11'650'000'000ULL, 94'150'000'000ULL,  94'880'000'000ULL,
    97'760'000'000ULL, 109'030'000'000ULL, 112'120'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.8e-06, 6.7e-07, 3.01e-07, 2.74e-07, 2.98e-07, 9.3e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000007436L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 30.784523937600859L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 30.784523937600859L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion114091Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[32U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    12'961'000'000ULL, 95'457'000'000ULL,  96'192'000'000ULL,
    99'066'000'000ULL, 110'343'000'000ULL, 113'426'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    6.9e-05, 1.14e-05, 1.29e-06, 8.1e-08, 3.04e-06, 9.7e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000085781L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 29.844566582343408L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 29.844566582343408L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion119899Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[33U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    18'769'000'000ULL,  101'265'000'000ULL, 102'000'000'000ULL,
    104'874'000'000ULL, 116'151'000'000ULL, 119'234'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00019, 2.21e-05, 9.6e-06, 8.6e-06, 9.6e-06, 3.03e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00024293L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 37.712557197546616L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 37.712557197546616L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion12108Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[34U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    19'950'000'000ULL,  102'450'000'000ULL, 103'180'000'000ULL,
    106'060'000'000ULL, 117'330'000'000ULL, 120'420'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4e-05, 4.6e-06, 2e-06, 1.8e-06, 2e-06, 6.3e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00005103L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 38.743143249069178L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 38.743143249069178L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion12367Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[35U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    22'540'000'000ULL,  105'036'000'000ULL, 105'771'000'000ULL,
    108'645'000'000ULL, 119'922'000'000ULL, 123'005'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000193, 2.24e-05, 9.5e-06, 8.4e-06, 9.7e-06, 3.03e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00024603L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 41.281215502174533L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 41.281215502174533L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion124815Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[36U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    23'685'000'000ULL,  106'181'000'000ULL, 106'916'000'000ULL,
    109'790'000'000ULL, 121'067'000'000ULL, 124'150'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00113, 0.00019, 0.000172, 0.000104, 0.000119, 3.8e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.001753L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 54.689575014261266L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 54.689575014261266L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion126066Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[37U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    24'936'000'000ULL,  107'432'000'000ULL, 108'167'000'000ULL,
    111'041'000'000ULL, 122'318'000'000ULL, 125'401'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.67e-05, 1.95e-06, 8.1e-07, 7.2e-10, 8.4e-07, 2.62e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00002056272L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 41.299059731397399L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 41.299059731397399L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion129146Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[38U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    28'016'000'000ULL,  110'512'000'000ULL, 111'247'000'000ULL,
    114'121'000'000ULL, 125'398'000'000ULL, 128'481'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    8e-05, 1.34e-05, 1.68e-05, 1.03e-05, 1.05e-05, 3.4e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0001344L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 63.393237351190479L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 63.393237351190479L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion133573Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[39U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    32'443'000'000ULL,  114'939'000'000ULL, 115'674'000'000ULL,
    118'548'000'000ULL, 129'825'000'000ULL, 132'908'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.63e-05, 4.25e-06, 1.7e-09, 1.48e-09, 1.78e-06, 5.6e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00004289318L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 45.976090530942216L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 45.976090530942216L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion134874Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[40U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    33'744'000'000ULL,  116'240'000'000ULL, 116'975'000'000ULL,
    119'849'000'000ULL, 131'126'000'000ULL, 134'209'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.8e-05, 6.8e-06, 2.69e-09, 2.35e-09, 2.83e-06, 8.9e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00006852504L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 47.263203318086354L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 47.263203318086354L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion139749Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[41U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    38'619'000'000ULL,  121'115'000'000ULL, 121'850'000'000ULL,
    124'724'000'000ULL, 136'001'000'000ULL, 139'084'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.3e-05, 6e-06, 6e-06, 4.2e-06, 3.9e-06, 1.26e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00005436L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 72.879093818984543L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 72.879093818984543L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion144627Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[42U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    43'497'000'000ULL,  125'993'000'000ULL, 126'728'000'000ULL,
    129'602'000'000ULL, 140'879'000'000ULL, 143'962'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.18e-05, 2e-06, 1.45e-06, 8e-07, 1.07e-06, 3.4e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00001746L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 71.728259450171819L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 71.728259450171819L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion145147Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[43U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    44'017'000'000ULL,  126'513'000'000ULL, 127'248'000'000ULL,
    130'122'000'000ULL, 141'399'000'000ULL, 144'482'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000221, 2.61e-05, 9.9e-09, 8.5e-09, 1.07e-05, 3.35e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0002611684L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 57.545614600388099L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 57.545614600388099L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion150063Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[44U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    48'933'000'000ULL,  131'429'000'000ULL, 132'164'000'000ULL,
    135'038'000'000ULL, 146'315'000'000ULL, 149'398'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000968, 1.153e-06, 4.26e-08, 3.61e-08, 4.64e-05, 1.461e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0010302417L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 54.842380652229473L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 54.842380652229473L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion152654Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[45U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    51'524'000'000ULL,  134'020'000'000ULL, 134'755'000'000ULL,
    137'629'000'000ULL, 148'906'000'000ULL, 151'989'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.64e-05, 3.15e-06, 1.15e-09, 9.7e-10, 1.26e-06, 3.97e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00003120912L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 65.065796836950227L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 65.065796836950227L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion153955Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[46U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    52'825'000'000ULL,  135'321'000'000ULL, 136'056'000'000ULL,
    138'930'000'000ULL, 150'207'000'000ULL, 153'290'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000269, 3.22e-05, 1.17e-08, 9.8e-09, 1.28e-05, 4.04e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0003180615L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 66.377589771789417L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 66.377589771789417L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion157243Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[47U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    56'113'000'000ULL,  138'609'000'000ULL, 139'344'000'000ULL,
    142'218'000'000ULL, 153'495'000'000ULL, 156'578'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0112, 0.0018, 0.00029, 7e-05, 0.00051, 0.000165,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.014035L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 73.562140363377267L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 73.562140363377267L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion16135Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[48U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    60'220'000'000ULL,  142'720'000'000ULL, 143'450'000'000ULL,
    146'320'000'000ULL, 157'600'000'000ULL, 160'690'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.8e-05, 9.6e-06, 8.6e-06, 4.7e-06, 5.9e-06, 1.87e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00008867L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 90.386560279688737L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 90.386560279688737L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion168733Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[49U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    67'603'000'000ULL,  150'099'000'000ULL, 150'834'000'000ULL,
    153'708'000'000ULL, 164'985'000'000ULL, 168'068'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00017, 2.8e-05, 2.4e-05, 1.3e-05, 1.7e-05, 5.3e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0002573L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 97.197852312475703L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 97.197852312475703L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion170805Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[50U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    69'675'000'000ULL,  152'171'000'000ULL, 152'906'000'000ULL,
    155'780'000'000ULL, 167'057'000'000ULL, 170'140'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.3e-05, 1.6e-06, 5.4e-10, 4.5e-10, 6.2e-07, 2e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00001542099L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 83.457975800516053L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 83.457975800516053L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion178312Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[51U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    77'182'000'000ULL,  159'678'000'000ULL, 160'413'000'000ULL,
    163'287'000'000ULL, 174'564'000'000ULL, 177'647'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.49e-05, 1.81e-06, 5.94e-10, 4.83e-10, 6.9e-07, 2.18e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000017619077L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 90.718676065891529L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 90.718676065891529L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion179756Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[52U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    78'626'000'000ULL,  161'122'000'000ULL, 161'857'000'000ULL,
    164'731'000'000ULL, 176'008'000'000ULL, 179'091'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000129, 2.12e-05, 1.65e-05, 8.3e-06, 1.17e-05, 3.74e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00019044L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 106.72937481621508L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 106.72937481621508L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion186286Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[53U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    85'156'000'000ULL,  167'652'000'000ULL, 168'387'000'000ULL,
    171'261'000'000ULL, 182'538'000'000ULL, 185'621'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.5e-06, 4.3e-07, 1.36e-10, 1.1e-10, 1.61e-07, 5.1e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000004142246L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 98.746764036225755L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 98.746764036225755L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion187921Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[54U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    86'791'000'000ULL,  169'287'000'000ULL, 170'022'000'000ULL,
    172'896'000'000ULL, 184'173'000'000ULL, 187'256'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000433, 5.31e-05, 1.67e-08, 1.35e-08, 1.99e-05, 6.28e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0005123102L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 100.36073172737923L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 100.36073172737923L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion195789Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[55U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    94'659'000'000ULL,  177'155'000'000ULL, 177'890'000'000ULL,
    180'764'000'000ULL, 192'041'000'000ULL, 195'124'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0016, 0.00027, 0.00013, 6e-05, 0.000117, 3.71e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0022141L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 118.76865877783298L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 118.76865877783298L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion197511Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[56U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    96'381'000'000ULL,  178'877'000'000ULL, 179'612'000'000ULL,
    182'486'000'000ULL, 193'763'000'000ULL, 196'846'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.89e-05, 2.33e-09, 7.1e-10, 5.6e-10, 8.6e-07, 2.71e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00002003460L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 101.93509459085782L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 101.93509459085782L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion197824Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[57U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    96'694'000'000ULL,  179'190'000'000ULL, 179'925'000'000ULL,
    182'799'000'000ULL, 194'076'000'000ULL, 197'159'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.75e-05, 3.39e-09, 1.03e-09, 8.2e-10, 1.25e-06, 4e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00002915524L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 102.26244997571619L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 102.26244997571619L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion198711Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[58U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    97'581'000'000ULL,  180'077'000'000ULL, 180'812'000'000ULL,
    183'686'000'000ULL, 194'963'000'000ULL, 198'046'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.35e-05, 1.66e-09, 5.04e-10, 3.99e-10, 6.13e-07, 1.93e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000014308563L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 103.12300716584888L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 103.12300716584888L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion216905Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[59U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    115'775'000'000ULL, 198'271'000'000ULL, 199'006'000'000ULL,
    201'880'000'000ULL, 213'157'000'000ULL, 216'240'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000186, 2.33e-08, 6.66e-09, 5.15e-09, 8.4e-06, 2.64e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00019707511L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 121.28638251811708L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 121.28638251811708L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion224567Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[60U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    123'437'000'000ULL, 205'933'000'000ULL, 206'668'000'000ULL,
    209'542'000'000ULL, 220'819'000'000ULL, 223'902'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    6.01e-05, 7.5e-09, 2.11e-09, 1.62e-09, 2.68e-06, 8.5e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00006364123L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 128.89435582907495L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 128.89435582907495L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion231196Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[61U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    130'066'000'000ULL, 212'562'000'000ULL, 213'297'000'000ULL,
    216'171'000'000ULL, 227'448'000'000ULL, 230'531'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.4e-05, 9e-06, 1e-06, 5.7e-11, 2.3e-06, 7.5e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000067050057L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 146.84491575819243L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 146.84491575819243L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion23864Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[62U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    137'510'000'000ULL, 220'010'000'000ULL, 220'740'000'000ULL,
    223'610'000'000ULL, 234'890'000'000ULL, 237'980'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    9.9e-06, 1.6e-06, 1.8e-07, 1.04e-11, 4.3e-07, 1.37e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0000122470104L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 154.05445279478167L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 154.05445279478167L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion240663Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[63U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    139'533'000'000ULL, 222'029'000'000ULL, 222'764'000'000ULL,
    225'638'000'000ULL, 236'915'000'000ULL, 239'998'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.35e-06, 6.8e-10, 1.8e-10, 1.36e-10, 2.36e-07, 7.5e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000005661996L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 144.93742206953166L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 144.93742206953166L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion243237Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[64U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    142'107'000'000ULL, 224'603'000'000ULL, 225'338'000'000ULL,
    228'212'000'000ULL, 239'489'000'000ULL, 242'572'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.9e-05, 4.7e-06, 5.4e-07, 3.05e-12, 1.26e-06, 4e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00003590000305L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 158.69651175549413L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 158.69651175549413L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion249614Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[65U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    148'484'000'000ULL, 230'980'000'000ULL, 231'715'000'000ULL,
    234'589'000'000ULL, 245'866'000'000ULL, 248'949'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.39e-05, 2.34e-06, 8.8e-06, 4.28e-06, 4.12e-06, 1.31e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00003475L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 201.05452748201438L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 201.05452748201438L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion253551Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[66U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    152'421'000'000ULL, 234'917'000'000ULL, 235'652'000'000ULL,
    238'526'000'000ULL, 249'803'000'000ULL, 252'886'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.35e-05, 6.8e-09, 1.75e-09, 1.3e-10, 2.34e-06, 7.41e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00005658968L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 157.77597401116245L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 157.77597401116245L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion256144Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[67U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    155'014'000'000ULL, 237'510'000'000ULL, 238'245'000'000ULL,
    241'119'000'000ULL, 252'396'000'000ULL, 255'479'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.47e-07, 1.86e-11, 4.8e-12, 3.5e-13, 6.4e-09, 2.03e-09,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 1.5545375E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 160.34775889066682L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 160.34775889066682L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion279209Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[68U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    178'079'000'000ULL, 260'575'000'000ULL, 261'310'000'000ULL,
    264'184'000'000ULL, 275'461'000'000ULL, 278'544'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    9.9e-06, 1.27e-09, 3.07e-10, 2.23e-11, 4.29e-07, 1.36e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0000104665993L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 183.38849531797783L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 183.38849531797783L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion282201Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[69U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    181'071'000'000ULL, 263'567'000'000ULL, 264'302'000'000ULL,
    267'176'000'000ULL, 278'453'000'000ULL, 281'536'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.42e-06, 5.5e-07, 6.3e-08, 3.51e-13, 1.47e-07, 4.74e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000004227400351L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 197.5571551866903L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 197.5571551866903L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion284896Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[70U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    183'766'000'000ULL, 266'262'000'000ULL, 266'997'000'000ULL,
    269'871'000'000ULL, 281'148'000'000ULL, 284'231'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.3e-06, 2.96e-10, 7e-12, 5.08e-12, 9.9e-08, 3.14e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00000243070808L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 189.04053623571284L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 189.04053623571284L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion29833Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[71U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    197'200'000'000ULL, 279'700'000'000ULL, 280'430'000'000ULL,
    283'300'000'000ULL, 294'580'000'000ULL, 297'670'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    6e-06, 9.8e-07, 4.1e-07, 1.43e-07, 3.8e-07, 1.21e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000008034L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 219.162642519293L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 219.162642519293L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion317119Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[72U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    215'989'000'000ULL, 298'485'000'000ULL, 299'220'000'000ULL,
    302'094'000'000ULL, 313'371'000'000ULL, 316'454'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.9e-06, 3.1e-07, 3.5e-11, 1.9e-13, 8.1e-08, 2.6e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00000231703519L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 231.55920868765915L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 231.55920868765915L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion321753Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[73U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    220'623'000'000ULL, 303'119'000'000ULL, 303'854'000'000ULL,
    306'728'000'000ULL, 318'005'000'000ULL, 321'088'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    7.8e-07, 1.02e-10, 2.22e-12, 1.56e-12, 3.31e-08, 1.05e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 8.2370578E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 225.82748015870425L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 225.82748015870425L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion354754Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[74U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    253'624'000'000ULL, 336'120'000'000ULL, 336'855'000'000ULL,
    339'729'000'000ULL, 351'006'000'000ULL, 354'089'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.8e-07, 5e-11, 1.02e-12, 7e-13, 1.6e-08, 5.1e-09,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 4.0115172E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 258.79599195636007L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 258.79599195636007L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion362394Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[75U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    261'264'000'000ULL, 343'760'000'000ULL, 344'495'000'000ULL,
    347'369'000'000ULL, 358'646'000'000ULL, 361'729'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    9.8e-07, 1.3e-10, 2.61e-12, 1.78e-12, 4.11e-08, 1.3e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00000103423439L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 266.40745706470852L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 266.40745706470852L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion36774Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[76U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    266'610'000'000ULL, 349'110'000'000ULL, 349'840'000'000ULL,
    352'720'000'000ULL, 363'990'000'000ULL, 367'080'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.4e-07, 2.2e-08, 3.6e-09, 2.2e-09, 6.8e-09, 2.1e-09,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 1.767E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 284.59099037917372L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 284.59099037917372L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion374881Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[77U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    273'751'000'000ULL, 356'247'000'000ULL, 356'982'000'000ULL,
    359'856'000'000ULL, 371'133'000'000ULL, 374'216'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.2e-07, 4.3e-11, 8.3e-13, 5.7e-13, 1.34e-08, 4.2e-09,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 3.3764440E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 278.87632501525275L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 278.87632501525275L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion406057Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[78U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    304'927'000'000ULL, 387'423'000'000ULL, 388'158'000'000ULL,
    391'032'000'000ULL, 402'309'000'000ULL, 405'392'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.12e-06, 1.48e-10, 2.75e-12, 1.83e-12, 4.59e-08, 1.46e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00000118065258L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 309.96592403758603L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 309.96592403758603L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion443408Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[79U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    342'278'000'000ULL, 424'774'000'000ULL, 425'509'000'000ULL,
    428'383'000'000ULL, 439'660'000'000ULL, 442'743'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4.3e-07, 7e-11, 9e-11, 3.2e-11, 4.9e-08, 1.6e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 4.95192E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 355.19254399505644L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 355.19254399505644L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion450915Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[80U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    349'785'000'000ULL, 432'281'000'000ULL, 433'016'000'000ULL,
    435'890'000'000ULL, 447'167'000'000ULL, 450'250'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.2e-06, 8.4e-07, 9.4e-11, 5e-13, 2.23e-07, 7.2e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0000063350945L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 365.29450388609041L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 365.29450388609041L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion452216Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[81U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    351'086'000'000ULL, 433'582'000'000ULL, 434'317'000'000ULL,
    437'191'000'000ULL, 448'468'000'000ULL, 451'551'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000185, 2.96e-05, 3.32e-09, 1.76e-11, 7.9e-06, 2.53e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0002250333376L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 366.48662343352987L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 366.48662343352987L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion462266Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[82U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    361'136'000'000ULL, 443'632'000'000ULL, 444'367'000'000ULL,
    447'241'000'000ULL, 458'518'000'000ULL, 461'601'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4.8e-08, 6.4e-12, 1.09e-13, 7e-14, 1.94e-09, 6.2e-10,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 5.0566579E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 366.11463329708346L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 366.11463329708346L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion515165Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[83U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    414'036'000'000ULL, 496'532'000'000ULL, 497'267'000'000ULL,
    500'141'000'000ULL, 511'418'000'000ULL, 514'501'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.61e-05, 4.17e-06, 4.62e-10, 2.44e-12, 1.11e-06, 3.56e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.00003173646444L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 429.40968926966082L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 429.40968926966082L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion525955Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[84U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    424'826'000'000ULL, 507'322'000'000ULL, 508'057'000'000ULL,
    510'931'000'000ULL, 522'208'000'000ULL, 525'291'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4.07e-05, 6.53e-06, 7.2e-10, 3.8e-12, 1.73e-06, 5.55e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.0000495157238L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 440.23500566456022L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 440.23500566456022L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion532123Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[85U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    430'994'000'000ULL, 513'490'000'000ULL, 514'225'000'000ULL,
    517'099'000'000ULL, 528'376'000'000ULL, 531'459'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    6.3e-08, 8.5e-12, 1.29e-13, 8.1e-14, 2.5e-09, 7.9e-10,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 6.6298710E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 435.87405388497001L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 435.87405388497001L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion551811Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[86U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    450'682'000'000ULL, 533'178'000'000ULL, 533'913'000'000ULL,
    536'787'000'000ULL, 548'064'000'000ULL, 551'147'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.3e-06, 8.4e-07, 9.3e-11, 4.9e-14, 2.2e-07, 7.2e-08,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 0.000006432093049L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 465.91217530310371L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 465.91217530310371L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion570669Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[87U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    469'540'000'000ULL, 552'036'000'000ULL, 552'771'000'000ULL,
    555'645'000'000ULL, 566'922'000'000ULL, 570'005'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.87e-07, 3.89e-12, 5.7e-13, 3.49e-14, 1.13e-08, 3.6e-09,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 3.019044949E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 474.3841211736808L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 474.3841211736808L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion637599Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[88U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    536'470'000'000ULL, 618'966'000'000ULL, 619'701'000'000ULL,
    622'575'000'000ULL, 633'852'000'000ULL, 636'935'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    8.9e-09, 1.32e-09, 1.8e-10, 8.6e-11, 3.7e-10, 1.2e-10,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 1.0976E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 552.8118795553936L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 552.8118795553936L,
              1.4134798368811606e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion64594Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[89U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    544'810'000'000ULL, 627'310'000'000ULL, 628'040'000'000ULL,
    630'920'000'000ULL, 642'190'000'000ULL, 645'280'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.08e-08, 1.6e-09, 2.2e-10, 1.04e-10, 4.6e-10, 1.5e-10,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 1.3334E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 561.24402879856007L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 561.24402879856007L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion76644Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[90U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    665'310'000'000ULL, 747'810'000'000ULL, 748'540'000'000ULL,
    751'420'000'000ULL, 762'690'000'000ULL, 765'780'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.53e-08, 2.2e-09, 2.7e-10, 1.26e-10, 6.3e-10, 2e-10,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 1.8726E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 681.13107016981735L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 681.13107016981735L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion77932Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[91U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    678'190'000'000ULL, 760'690'000'000ULL, 761'420'000'000ULL,
    764'300'000'000ULL, 775'570'000'000ULL, 778'660'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.7e-09, 4e-10, 4.8e-11, 2.2e-11, 1.1e-10, 3.5e-11,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 3.315E-9L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 694.21344193061839L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 694.21344193061839L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion80848Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[92U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    707'350'000'000ULL, 789'850'000'000ULL, 790'580'000'000ULL,
    793'460'000'000ULL, 804'730'000'000ULL, 807'820'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    9.4e-08, 1.39e-08, 1.7e-09, 7.6e-10, 3.8e-09, 1.24e-09,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 1.1540E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 723.36656325823219L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 723.36656325823219L,
              1.413549686074257e-07L);
}

TEST(GGEMSAc225Test, PreservesConversion825Kev) {
  auto const definition = BuildAc225Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[93U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    723'870'000'000ULL, 806'370'000'000ULL, 807'100'000'000ULL,
    809'980'000'000ULL, 821'250'000'000ULL, 824'340'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.16e-09, 3.1e-10, 3.7e-11, 1.68e-11, 8.8e-11, 2.8e-11,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(weight_sum, 2.6398E-9L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 739.5847352072127L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 739.5847352072127L,
              1.413549686074257e-07L);
}

} // namespace
