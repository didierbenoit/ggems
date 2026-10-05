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
 * \brief Tests the independently selected Eu-152 source contracts.
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
using ggems::core::radioactivity::builtins::BuildEu152Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildEu152Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Eu-152");
  // LNHB evaluated 4939 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 426729600.0L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 130U);
  // Independent selected LNHB values; see data/Eu-152/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_minus_126_4_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[0U].GetYieldPerDecay(), 0.000203L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_175_4_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[1U].GetYieldPerDecay(), 0.01826L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_213_5_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[2U].GetYieldPerDecay(), 0.00101L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_268_6_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[3U].GetYieldPerDecay(), 0.000536L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_384_8_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[4U].GetYieldPerDecay(), 0.0244L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_500_3_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[5U].GetYieldPerDecay(), 0.000267L, 1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_504_1_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[6U].GetYieldPerDecay(), 0.000048L, 1.0e-16L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_536_5_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[7U].GetYieldPerDecay(), 0.00037L, 1.0e-16L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_695_6_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[8U].GetYieldPerDecay(), 0.138L, 1.0e-16L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_709_7_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[9U].GetYieldPerDecay(), 0.00245L, 1.0e-16L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_888_2_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[10U].GetYieldPerDecay(), 0.00303L, 1.0e-16L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1063_4_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[11U].GetYieldPerDecay(), 0.00904L, 1.0e-16L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1474_5_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[12U].GetYieldPerDecay(), 0.0817L, 1.0e-16L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_plus_485_8_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_NEAR(emissions[13U].GetYieldPerDecay(), 0.000024L, 1.0e-16L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_plus_730_5_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_NEAR(emissions[14U].GetYieldPerDecay(), 0.00025L, 1.0e-16L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[15U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(emissions[15U].GetYieldPerDecay(), 1.5918799L, 1.0e-16L);
  EXPECT_EQ(emissions[15U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[16U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(emissions[16U].GetYieldPerDecay(), 0.873513L, 1.0e-16L);
  EXPECT_EQ(emissions[16U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_121_7817_keV
  EXPECT_EQ(emissions[17U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[17U].GetYieldPerDecay(), 0.32998L, 1.0e-16L);
  EXPECT_EQ(emissions[17U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_125_69_keV
  EXPECT_EQ(emissions[18U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[18U].GetYieldPerDecay(), 0.0001977L, 1.0e-16L);
  EXPECT_EQ(emissions[18U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_148_01_keV
  EXPECT_EQ(emissions[19U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[19U].GetYieldPerDecay(), 0.0002022L, 1.0e-16L);
  EXPECT_EQ(emissions[19U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_192_6_keV
  EXPECT_EQ(emissions[20U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[20U].GetYieldPerDecay(), 0.0000034250L, 1.0e-16L);
  EXPECT_EQ(emissions[20U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_207_6_keV
  EXPECT_EQ(emissions[21U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[21U].GetYieldPerDecay(), 0.0000022727L, 1.0e-16L);
  EXPECT_EQ(emissions[21U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_209_41_keV
  EXPECT_EQ(emissions[22U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[22U].GetYieldPerDecay(), 0.0000022187L, 1.0e-16L);
  EXPECT_EQ(emissions[22U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_212_568_keV
  EXPECT_EQ(emissions[23U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[23U].GetYieldPerDecay(), 0.000033634L, 1.0e-16L);
  EXPECT_EQ(emissions[23U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_239_42_keV
  EXPECT_EQ(emissions[24U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[24U].GetYieldPerDecay(), 0.000002119L, 1.0e-16L);
  EXPECT_EQ(emissions[24U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_244_6974_keV
  EXPECT_EQ(emissions[25U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[25U].GetYieldPerDecay(), 0.008170L, 1.0e-16L);
  EXPECT_EQ(emissions[25U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_251_633_keV
  EXPECT_EQ(emissions[26U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[26U].GetYieldPerDecay(), 0.0000156384L, 1.0e-16L);
  EXPECT_EQ(emissions[26U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_269_86_keV
  EXPECT_EQ(emissions[27U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[27U].GetYieldPerDecay(), 0.000004739L, 1.0e-16L);
  EXPECT_EQ(emissions[27U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_271_131_keV
  EXPECT_EQ(emissions[28U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[28U].GetYieldPerDecay(), 0.000064566L, 1.0e-16L);
  EXPECT_EQ(emissions[28U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_275_449_keV
  EXPECT_EQ(emissions[29U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[29U].GetYieldPerDecay(), 0.000033787L, 1.0e-16L);
  EXPECT_EQ(emissions[29U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_285_98_keV
  EXPECT_EQ(emissions[30U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[30U].GetYieldPerDecay(), 0.000006582L, 1.0e-16L);
  EXPECT_EQ(emissions[30U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_295_9387_keV
  EXPECT_EQ(emissions[31U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[31U].GetYieldPerDecay(), 0.000067958L, 1.0e-16L);
  EXPECT_EQ(emissions[31U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_315_174_keV
  EXPECT_EQ(emissions[32U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[32U].GetYieldPerDecay(), 0.000025749L, 1.0e-16L);
  EXPECT_EQ(emissions[32U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_316_2_keV
  EXPECT_EQ(emissions[33U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[33U].GetYieldPerDecay(), 0.000001494L, 1.0e-16L);
  EXPECT_EQ(emissions[33U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_324_83_keV
  EXPECT_EQ(emissions[34U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[34U].GetYieldPerDecay(), 0.000046838L, 1.0e-16L);
  EXPECT_EQ(emissions[34U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_329_425_keV
  EXPECT_EQ(emissions[35U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[35U].GetYieldPerDecay(), 0.000015136L, 1.0e-16L);
  EXPECT_EQ(emissions[35U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_330_54_keV
  EXPECT_EQ(emissions[36U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[36U].GetYieldPerDecay(), 6.925E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[36U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_340_4_keV
  EXPECT_EQ(emissions[37U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[37U].GetYieldPerDecay(), 0.000011898L, 1.0e-16L);
  EXPECT_EQ(emissions[37U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_344_2785_keV
  EXPECT_EQ(emissions[38U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[38U].GetYieldPerDecay(), 0.0106091L, 1.0e-16L);
  EXPECT_EQ(emissions[38U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_351_66_keV
  EXPECT_EQ(emissions[39U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[39U].GetYieldPerDecay(), 0.000005239L, 1.0e-16L);
  EXPECT_EQ(emissions[39U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_357_26_keV
  EXPECT_EQ(emissions[40U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[40U].GetYieldPerDecay(), 3.8372E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[40U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_367_7891_keV
  EXPECT_EQ(emissions[41U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[41U].GetYieldPerDecay(), 0.000083852L, 1.0e-16L);
  EXPECT_EQ(emissions[41U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_385_69_keV
  EXPECT_EQ(emissions[42U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[42U].GetYieldPerDecay(), 0.0000017417L, 1.0e-16L);
  EXPECT_EQ(emissions[42U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_387_9_keV
  EXPECT_EQ(emissions[43U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[43U].GetYieldPerDecay(), 0.0000112L, 1.0e-16L);
  EXPECT_EQ(emissions[43U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_411_1165_keV
  EXPECT_EQ(emissions[44U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[44U].GetYieldPerDecay(), 0.00053328L, 1.0e-16L);
  EXPECT_EQ(emissions[44U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_416_048_keV
  EXPECT_EQ(emissions[45U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[45U].GetYieldPerDecay(), 0.0000072606L, 1.0e-16L);
  EXPECT_EQ(emissions[45U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_423_45_keV
  EXPECT_EQ(emissions[46U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[46U].GetYieldPerDecay(), 8.621E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[46U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_440_86_keV
  EXPECT_EQ(emissions[47U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[47U].GetYieldPerDecay(), 0.0000026143L, 1.0e-16L);
  EXPECT_EQ(emissions[47U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_443_965_keV_sm152_5_2
  EXPECT_EQ(emissions[48U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[48U].GetYieldPerDecay(), 0.000057082L, 1.0e-16L);
  EXPECT_EQ(emissions[48U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_443_965_keV_sm152_13_9
  EXPECT_EQ(emissions[49U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[49U].GetYieldPerDecay(), 0.000168206L, 1.0e-16L);
  EXPECT_EQ(emissions[49U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_482_31_keV_gd152_13_7
  EXPECT_EQ(emissions[50U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[50U].GetYieldPerDecay(), 7.1775E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[50U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_482_31_keV_sm152_11_5
  EXPECT_EQ(emissions[51U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[51U].GetYieldPerDecay(), 0.0000053496L, 1.0e-16L);
  EXPECT_EQ(emissions[51U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_488_6792_keV
  EXPECT_EQ(emissions[52U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[52U].GetYieldPerDecay(), 0.000057904L, 1.0e-16L);
  EXPECT_EQ(emissions[52U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_493_508_keV_gd152_6_2
  EXPECT_EQ(emissions[53U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[53U].GetYieldPerDecay(), 0.0000013089L, 1.0e-16L);
  EXPECT_EQ(emissions[53U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_493_508_keV_sm152_14_9
  EXPECT_EQ(emissions[54U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[54U].GetYieldPerDecay(), 0.0000012394L, 1.0e-16L);
  EXPECT_EQ(emissions[54U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_496_39_keV_gd152_13_6
  EXPECT_EQ(emissions[55U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[55U].GetYieldPerDecay(), 0.0000034L, 1.0e-16L);
  EXPECT_EQ(emissions[55U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_496_39_keV_sm152_17_10
  EXPECT_EQ(emissions[56U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[56U].GetYieldPerDecay(), 2.1703E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[56U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_503_474_keV
  EXPECT_EQ(emissions[57U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[57U].GetYieldPerDecay(), 0.000021192L, 1.0e-16L);
  EXPECT_EQ(emissions[57U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_520_227_keV
  EXPECT_EQ(emissions[58U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[58U].GetYieldPerDecay(), 0.0000097339L, 1.0e-16L);
  EXPECT_EQ(emissions[58U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_523_13_keV
  EXPECT_EQ(emissions[59U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[59U].GetYieldPerDecay(), 0.0000017582L, 1.0e-16L);
  EXPECT_EQ(emissions[59U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_526_881_keV
  EXPECT_EQ(emissions[60U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[60U].GetYieldPerDecay(), 0.0000108L, 1.0e-16L);
  EXPECT_EQ(emissions[60U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_534_245_keV
  EXPECT_EQ(emissions[61U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[61U].GetYieldPerDecay(), 0.0000015114L, 1.0e-16L);
  EXPECT_EQ(emissions[61U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_538_29_keV
  EXPECT_EQ(emissions[62U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[62U].GetYieldPerDecay(), 6.061E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[62U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_556_56_keV
  EXPECT_EQ(emissions[63U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[63U].GetYieldPerDecay(), 5.9953E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[63U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_557_91_keV
  EXPECT_EQ(emissions[64U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[64U].GetYieldPerDecay(), 4.650E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[64U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_561_2_keV
  EXPECT_EQ(emissions[65U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[65U].GetYieldPerDecay(), 1.0282E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[65U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_562_93_keV
  EXPECT_EQ(emissions[66U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[66U].GetYieldPerDecay(), 0.000003622L, 1.0e-16L);
  EXPECT_EQ(emissions[66U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_563_99_keV
  EXPECT_EQ(emissions[67U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[67U].GetYieldPerDecay(), 0.0000149734L, 1.0e-16L);
  EXPECT_EQ(emissions[67U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_566_442_keV
  EXPECT_EQ(emissions[68U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[68U].GetYieldPerDecay(), 0.0000180483L, 1.0e-16L);
  EXPECT_EQ(emissions[68U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_586_265_keV
  EXPECT_EQ(emissions[69U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[69U].GetYieldPerDecay(), 0.000093L, 1.0e-16L);
  EXPECT_EQ(emissions[69U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_616_05_keV
  EXPECT_EQ(emissions[70U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[70U].GetYieldPerDecay(), 6.9682E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[70U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_644_37_keV
  EXPECT_EQ(emissions[71U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[71U].GetYieldPerDecay(), 1.5480E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[71U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_656_489_keV
  EXPECT_EQ(emissions[72U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[72U].GetYieldPerDecay(), 0.0000714L, 1.0e-16L);
  EXPECT_EQ(emissions[72U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_664_78_keV
  EXPECT_EQ(emissions[73U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[73U].GetYieldPerDecay(), 6.242E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[73U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_671_155_keV
  EXPECT_EQ(emissions[74U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[74U].GetYieldPerDecay(), 0.0000020493L, 1.0e-16L);
  EXPECT_EQ(emissions[74U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_674_675_keV
  EXPECT_EQ(emissions[75U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[75U].GetYieldPerDecay(), 0.0000037757L, 1.0e-16L);
  EXPECT_EQ(emissions[75U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_674_677_keV
  EXPECT_EQ(emissions[76U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[76U].GetYieldPerDecay(), 0.0000012958L, 1.0e-16L);
  EXPECT_EQ(emissions[76U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_678_623_keV
  EXPECT_EQ(emissions[77U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[77U].GetYieldPerDecay(), 0.000032241L, 1.0e-16L);
  EXPECT_EQ(emissions[77U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_686_61_keV
  EXPECT_EQ(emissions[78U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[78U].GetYieldPerDecay(), 0.0000015817L, 1.0e-16L);
  EXPECT_EQ(emissions[78U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_688_67_keV
  EXPECT_EQ(emissions[79U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[79U].GetYieldPerDecay(), 0.000302L, 1.0e-16L);
  EXPECT_EQ(emissions[79U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_703_25_keV_gd152_5_1
  EXPECT_EQ(emissions[80U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[80U].GetYieldPerDecay(), 1.0814E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[80U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_703_25_keV_gd152_10_2
  EXPECT_EQ(emissions[81U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[81U].GetYieldPerDecay(), 2.1093E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[81U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_712_843_keV
  EXPECT_EQ(emissions[82U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[82U].GetYieldPerDecay(), 0.0000021354L, 1.0e-16L);
  EXPECT_EQ(emissions[82U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_719_349_keV_sm152_9_2
  EXPECT_EQ(emissions[83U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[83U].GetYieldPerDecay(), 0.000014057L, 1.0e-16L);
  EXPECT_EQ(emissions[83U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_719_349_keV_sm152_13_5
  EXPECT_EQ(emissions[84U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[84U].GetYieldPerDecay(), 0.0000011654L, 1.0e-16L);
  EXPECT_EQ(emissions[84U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_727_99_keV
  EXPECT_EQ(emissions[85U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[85U].GetYieldPerDecay(), 2.0524E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[85U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_764_9_keV
  EXPECT_EQ(emissions[86U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[86U].GetYieldPerDecay(), 0.0000099842L, 1.0e-16L);
  EXPECT_EQ(emissions[86U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_768_944_keV
  EXPECT_EQ(emissions[87U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[87U].GetYieldPerDecay(), 0.00000153625L, 1.0e-16L);
  EXPECT_EQ(emissions[87U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_778_9045_keV
  EXPECT_EQ(emissions[88U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[88U].GetYieldPerDecay(), 0.000242328L, 1.0e-16L);
  EXPECT_EQ(emissions[88U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_794_81_keV
  EXPECT_EQ(emissions[89U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[89U].GetYieldPerDecay(), 0.0000020118L, 1.0e-16L);
  EXPECT_EQ(emissions[89U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_805_7_keV
  EXPECT_EQ(emissions[90U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[90U].GetYieldPerDecay(), 2.0278E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[90U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_810_451_keV
  EXPECT_EQ(emissions[91U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[91U].GetYieldPerDecay(), 0.0000124413L, 1.0e-16L);
  EXPECT_EQ(emissions[91U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_839_36_keV
  EXPECT_EQ(emissions[92U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[92U].GetYieldPerDecay(), 2.2472E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[92U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_841_574_keV
  EXPECT_EQ(emissions[93U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[93U].GetYieldPerDecay(), 0.00000229105L, 1.0e-16L);
  EXPECT_EQ(emissions[93U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_867_38_keV
  EXPECT_EQ(emissions[94U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[94U].GetYieldPerDecay(), 0.000145682L, 1.0e-16L);
  EXPECT_EQ(emissions[94U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_901_181_keV
  EXPECT_EQ(emissions[95U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[95U].GetYieldPerDecay(), 0.0000025865L, 1.0e-16L);
  EXPECT_EQ(emissions[95U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_919_337_keV
  EXPECT_EQ(emissions[96U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[96U].GetYieldPerDecay(), 0.0000050217L, 1.0e-16L);
  EXPECT_EQ(emissions[96U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_926_317_keV
  EXPECT_EQ(emissions[97U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[97U].GetYieldPerDecay(), 0.0000080677L, 1.0e-16L);
  EXPECT_EQ(emissions[97U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_930_58_keV
  EXPECT_EQ(emissions[98U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[98U].GetYieldPerDecay(), 0.0000023407L, 1.0e-16L);
  EXPECT_EQ(emissions[98U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_937_05_keV
  EXPECT_EQ(emissions[99U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[99U].GetYieldPerDecay(), 1.1762E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[99U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_958_63_keV
  EXPECT_EQ(emissions[100U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[100U].GetYieldPerDecay(), 6.5E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[100U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_963_39_keV
  EXPECT_EQ(emissions[101U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[101U].GetYieldPerDecay(), 0.00000155199L, 1.0e-16L);
  EXPECT_EQ(emissions[101U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_964_079_keV
  EXPECT_EQ(emissions[102U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[102U].GetYieldPerDecay(), 0.00039398L, 1.0e-16L);
  EXPECT_EQ(emissions[102U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_974_09_keV
  EXPECT_EQ(emissions[103U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[103U].GetYieldPerDecay(), 6.6E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[103U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_990_19_keV
  EXPECT_EQ(emissions[104U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[104U].GetYieldPerDecay(), 8.9543E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[104U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1005_272_keV
  EXPECT_EQ(emissions[105U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[105U].GetYieldPerDecay(), 0.0000172201L, 1.0e-16L);
  EXPECT_EQ(emissions[105U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1085_837_keV
  EXPECT_EQ(emissions[106U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[106U].GetYieldPerDecay(), 0.000214006L, 1.0e-16L);
  EXPECT_EQ(emissions[106U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1089_737_keV
  EXPECT_EQ(emissions[107U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[107U].GetYieldPerDecay(), 0.0000346L, 1.0e-16L);
  EXPECT_EQ(emissions[107U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_1109_174_keV
  EXPECT_EQ(emissions[108U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[108U].GetYieldPerDecay(), 0.0000041645L, 1.0e-16L);
  EXPECT_EQ(emissions[108U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1112_076_keV
  EXPECT_EQ(emissions[109U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[109U].GetYieldPerDecay(), 0.000268359L, 1.0e-16L);
  EXPECT_EQ(emissions[109U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1170_93_keV
  EXPECT_EQ(emissions[110U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[110U].GetYieldPerDecay(), 8.5229E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[110U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1206_11_keV
  EXPECT_EQ(emissions[111U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[111U].GetYieldPerDecay(), 2.5454E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[111U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1212_948_keV
  EXPECT_EQ(emissions[112U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[112U].GetYieldPerDecay(), 0.0000099339L, 1.0e-16L);
  EXPECT_EQ(emissions[112U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1249_938_keV
  EXPECT_EQ(emissions[113U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[113U].GetYieldPerDecay(), 0.0000030308L, 1.0e-16L);
  EXPECT_EQ(emissions[113U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1261_343_keV
  EXPECT_EQ(emissions[114U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[114U].GetYieldPerDecay(), 9.0634E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[114U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1292_778_keV
  EXPECT_EQ(emissions[115U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[115U].GetYieldPerDecay(), 0.00000157634L, 1.0e-16L);
  EXPECT_EQ(emissions[115U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1299_142_keV
  EXPECT_EQ(emissions[116U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[116U].GetYieldPerDecay(), 0.0000114591L, 1.0e-16L);
  EXPECT_EQ(emissions[116U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1314_7_keV
  EXPECT_EQ(emissions[117U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[117U].GetYieldPerDecay(), 3.350E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[117U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1348_1_keV
  EXPECT_EQ(emissions[118U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[118U].GetYieldPerDecay(), 2.6769E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[118U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1363_77_keV
  EXPECT_EQ(emissions[119U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[119U].GetYieldPerDecay(), 5.0697E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[119U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1390_36_keV
  EXPECT_EQ(emissions[120U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[120U].GetYieldPerDecay(), 7.729E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[120U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1408_013_keV
  EXPECT_EQ(emissions[121U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[121U].GetYieldPerDecay(), 0.000120492L, 1.0e-16L);
  EXPECT_EQ(emissions[121U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1457_643_keV
  EXPECT_EQ(emissions[122U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[122U].GetYieldPerDecay(), 0.00000285574L, 1.0e-16L);
  EXPECT_EQ(emissions[122U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1528_103_keV
  EXPECT_EQ(emissions[123U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[123U].GetYieldPerDecay(), 0.000001124L, 1.0e-16L);
  EXPECT_EQ(emissions[123U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_1605_61_keV
  EXPECT_EQ(emissions[124U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[124U].GetYieldPerDecay(), 7.29E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[124U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_1608_36_keV
  EXPECT_EQ(emissions[125U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[125U].GetYieldPerDecay(), 2.12E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[125U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_1635_2_keV
  EXPECT_EQ(emissions[126U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[126U].GetYieldPerDecay(), 1.5E-9L, 1.0e-16L);
  EXPECT_EQ(emissions[126U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_1643_6_keV
  EXPECT_EQ(emissions[127U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[127U].GetYieldPerDecay(), 4.2E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[127U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_1647_41_keV
  EXPECT_EQ(emissions[128U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[128U].GetYieldPerDecay(), 5.12E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[128U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_1769_09_keV
  EXPECT_EQ(emissions[129U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[129U].GetYieldPerDecay(), 6.44E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[129U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  EXPECT_NEAR(definition.GetTotalYieldPerDecay(), 3.097554520545L, 1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus1264Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 126.4 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'604'000ULL);
  ASSERT_EQ(energies.size(), 253U);
  EXPECT_EQ(distribution.GetTableCount(), 253U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'802'000ULL, 188'000ULL);
  EXPECT_EQ(energies.back() + 249'802'000ULL, 126'400'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'604'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 32.97801164399796L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 32.97801164399796L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 63U) {
      EXPECT_NEAR(cumulative, 0.55369292135530043L, 1.0e-12L);
    }
    if (index + 1U == 126U) {
      EXPECT_NEAR(cumulative, 0.85840048708127046L, 1.0e-12L);
    }
    if (index + 1U == 189U) {
      EXPECT_NEAR(cumulative, 0.98081806335127759L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus1754Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 175.4 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'714'000ULL);
  ASSERT_EQ(energies.size(), 351U);
  EXPECT_EQ(distribution.GetTableCount(), 351U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'857'000ULL, 386'000ULL);
  EXPECT_EQ(energies.back() + 249'857'000ULL, 175'400'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'714'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 46.77055132795343L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 46.77055132795343L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 87U) {
      EXPECT_NEAR(cumulative, 0.53982802172006642L, 1.0e-12L);
    }
    if (index + 1U == 175U) {
      EXPECT_NEAR(cumulative, 0.85142198392847069L, 1.0e-12L);
    }
    if (index + 1U == 263U) {
      EXPECT_NEAR(cumulative, 0.97985083982254402L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus2135Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 213.5 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'998'000ULL);
  ASSERT_EQ(energies.size(), 427U);
  EXPECT_EQ(distribution.GetTableCount(), 427U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'999'000ULL, 854'000ULL);
  EXPECT_EQ(energies.back() + 249'999'000ULL, 213'500'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'998'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 57.85987188390199L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 57.85987188390199L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 106U) {
      EXPECT_NEAR(cumulative, 0.53144709212059538L, 1.0e-12L);
    }
    if (index + 1U == 213U) {
      EXPECT_NEAR(cumulative, 0.84598397127206405L, 1.0e-12L);
    }
    if (index + 1U == 320U) {
      EXPECT_NEAR(cumulative, 0.97876306877054097L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus2686Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 268.6 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'256'000ULL);
  ASSERT_EQ(energies.size(), 538U);
  EXPECT_EQ(distribution.GetTableCount(), 538U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'628'000ULL, 272'000ULL);
  EXPECT_EQ(energies.back() + 249'628'000ULL, 268'600'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'256'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 74.43711836046289L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 74.43711836046289L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 134U) {
      EXPECT_NEAR(cumulative, 0.52018010629990471L, 1.0e-12L);
    }
    if (index + 1U == 269U) {
      EXPECT_NEAR(cumulative, 0.83904628218248097L, 1.0e-12L);
    }
    if (index + 1U == 403U) {
      EXPECT_NEAR(cumulative, 0.97709022643639211L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus3848Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 384.8 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'740'000ULL);
  ASSERT_EQ(energies.size(), 770U);
  EXPECT_EQ(distribution.GetTableCount(), 770U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'870'000ULL, 200'000ULL);
  EXPECT_EQ(energies.back() + 249'870'000ULL, 384'800'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'740'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 111.31749788213098L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 111.31749788213098L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 192U) {
      EXPECT_NEAR(cumulative, 0.49562375913898829L, 1.0e-12L);
    }
    if (index + 1U == 385U) {
      EXPECT_NEAR(cumulative, 0.82314986001897818L, 1.0e-12L);
    }
    if (index + 1U == 577U) {
      EXPECT_NEAR(cumulative, 0.97396836859236122L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus5003Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 500.3 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'800'000ULL);
  ASSERT_EQ(energies.size(), 1001U);
  EXPECT_EQ(distribution.GetTableCount(), 1001U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'900'000ULL, 200'000ULL);
  EXPECT_EQ(energies.back() + 249'900'000ULL, 500'300'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'800'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 150.26035000279865L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 150.26035000279865L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 250U) {
      EXPECT_NEAR(cumulative, 0.47364082531316675L, 1.0e-12L);
    }
    if (index + 1U == 500U) {
      EXPECT_NEAR(cumulative, 0.80804940133452815L, 1.0e-12L);
    }
    if (index + 1U == 750U) {
      EXPECT_NEAR(cumulative, 0.97096601845308247L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus5041Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 504.1 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'602'000ULL);
  ASSERT_EQ(energies.size(), 1009U);
  EXPECT_EQ(distribution.GetTableCount(), 1009U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'801'000ULL, 1'582'000ULL);
  EXPECT_EQ(energies.back() + 249'801'000ULL, 504'100'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'602'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 164.46726434988997L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 164.46726434988997L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 252U) {
      EXPECT_NEAR(cumulative, 0.43631781121072067L, 1.0e-12L);
    }
    if (index + 1U == 504U) {
      EXPECT_NEAR(cumulative, 0.75946981723702499L, 1.0e-12L);
    }
    if (index + 1U == 756U) {
      EXPECT_NEAR(cumulative, 0.95474975268821902L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus5365Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 536.5 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'998'000ULL);
  ASSERT_EQ(energies.size(), 1073U);
  EXPECT_EQ(distribution.GetTableCount(), 1073U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'999'000ULL, 2'146'000ULL);
  EXPECT_EQ(energies.back() + 249'999'000ULL, 536'500'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'998'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 162.88126347111057L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 162.88126347111057L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 268U) {
      EXPECT_NEAR(cumulative, 0.46701277494900177L, 1.0e-12L);
    }
    if (index + 1U == 536U) {
      EXPECT_NEAR(cumulative, 0.80374336325744766L, 1.0e-12L);
    }
    if (index + 1U == 804U) {
      EXPECT_NEAR(cumulative, 0.97009429516992783L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus6956Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 695.6 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'712'000ULL);
  ASSERT_EQ(energies.size(), 1392U);
  EXPECT_EQ(distribution.GetTableCount(), 1392U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'856'000ULL, 896'000ULL);
  EXPECT_EQ(energies.back() + 249'856'000ULL, 695'600'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'712'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 220.39466781256291L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 220.39466781256291L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 348U) {
      EXPECT_NEAR(cumulative, 0.44045911350890323L, 1.0e-12L);
    }
    if (index + 1U == 696U) {
      EXPECT_NEAR(cumulative, 0.78648507410066188L, 1.0e-12L);
    }
    if (index + 1U == 1044U) {
      EXPECT_NEAR(cumulative, 0.96668773838010220L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus7097Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 709.7 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'788'000ULL);
  ASSERT_EQ(energies.size(), 1420U);
  EXPECT_EQ(distribution.GetTableCount(), 1420U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'894'000ULL, 1'040'000ULL);
  EXPECT_EQ(energies.back() + 249'894'000ULL, 709'700'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'788'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 225.63743948491972L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 225.63743948491972L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 355U) {
      EXPECT_NEAR(cumulative, 0.43824426929828391L, 1.0e-12L);
    }
    if (index + 1U == 710U) {
      EXPECT_NEAR(cumulative, 0.78501260879424254L, 1.0e-12L);
    }
    if (index + 1U == 1065U) {
      EXPECT_NEAR(cumulative, 0.96638327839848567L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus8882Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 888.2 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'830'000ULL);
  ASSERT_EQ(energies.size(), 1777U);
  EXPECT_EQ(distribution.GetTableCount(), 1777U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'915'000ULL, 2'090'000ULL);
  EXPECT_EQ(energies.back() + 249'915'000ULL, 888'200'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'830'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 293.7613540303877L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 293.7613540303877L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 444U) {
      EXPECT_NEAR(cumulative, 0.41214446007006154L, 1.0e-12L);
    }
    if (index + 1U == 888U) {
      EXPECT_NEAR(cumulative, 0.76731943249102808L, 1.0e-12L);
    }
    if (index + 1U == 1332U) {
      EXPECT_NEAR(cumulative, 0.96259302730903089L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus10634Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 1063.4 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'952'000ULL);
  ASSERT_EQ(energies.size(), 2127U);
  EXPECT_EQ(distribution.GetTableCount(), 2127U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'976'000ULL, 2'096'000ULL);
  EXPECT_EQ(energies.back() + 249'976'000ULL, 1'063'400'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'952'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 363.31968229657264L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 363.31968229657264L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 531U) {
      EXPECT_NEAR(cumulative, 0.38987131043727856L, 1.0e-12L);
    }
    if (index + 1U == 1063U) {
      EXPECT_NEAR(cumulative, 0.75238083588942931L, 1.0e-12L);
    }
    if (index + 1U == 1595U) {
      EXPECT_NEAR(cumulative, 0.95956120517595393L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaMinus14745Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 1474.5 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'998'000ULL);
  ASSERT_EQ(energies.size(), 2949U);
  EXPECT_EQ(distribution.GetTableCount(), 2949U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'999'000ULL, 5'898'000ULL);
  EXPECT_EQ(energies.back() + 249'999'000ULL, 1'474'500'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'998'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 532.8097774684103L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 532.8097774684103L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 737U) {
      EXPECT_NEAR(cumulative, 0.36338707848476168L, 1.0e-12L);
    }
    if (index + 1U == 1474U) {
      EXPECT_NEAR(cumulative, 0.71745768576296182L, 1.0e-12L);
    }
    if (index + 1U == 2211U) {
      EXPECT_NEAR(cumulative, 0.94587675713231195L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaPlus4858Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 485.8 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(),
            1'999'176'000ULL);
  ASSERT_EQ(energies.size(), 243U);
  EXPECT_EQ(distribution.GetTableCount(), 243U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 999'588'000ULL, 232'000ULL);
  EXPECT_EQ(energies.back() + 999'588'000ULL, 485'800'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 1'999'176'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 229.9698437773362L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 229.9698437773362L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 60U) {
      EXPECT_NEAR(cumulative, 0.12048232461487976L, 1.0e-12L);
    }
    if (index + 1U == 121U) {
      EXPECT_NEAR(cumulative, 0.55902986348849402L, 1.0e-12L);
    }
    if (index + 1U == 182U) {
      EXPECT_NEAR(cumulative, 0.91967374155605269L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesBetaPlus7305Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 730.5 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'998'000ULL);
  ASSERT_EQ(energies.size(), 1461U);
  EXPECT_EQ(distribution.GetTableCount(), 1461U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'999'000ULL, 2'922'000ULL);
  EXPECT_EQ(energies.back() + 249'999'000ULL, 730'500'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'998'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 337.02205449865863L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 337.02205449865863L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 365U) {
      EXPECT_NEAR(cumulative, 0.14546126565590457L, 1.0e-12L);
    }
    if (index + 1U == 730U) {
      EXPECT_NEAR(cumulative, 0.58485549171172963L, 1.0e-12L);
    }
    if (index + 1U == 1095U) {
      EXPECT_NEAR(cumulative, 0.92519702885513331L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesNuclearGamma) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[15U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB selected photon law; see reference README; each line
  // retains its energy and absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 125U> expected_energies{
    {
      121'781'700'000ULL,   125'690'000'000ULL,   148'010'000'000ULL,
      192'600'000'000ULL,   207'600'000'000ULL,   209'410'000'000ULL,
      212'568'000'000ULL,   237'310'000'000ULL,   239'420'000'000ULL,
      244'697'400'000ULL,   251'633'000'000ULL,   269'860'000'000ULL,
      271'131'000'000ULL,   275'449'000'000ULL,   285'980'000'000ULL,
      295'938'700'000ULL,   315'174'000'000ULL,   316'200'000'000ULL,
      320'030'000'000ULL,   324'830'000'000ULL,   329'425'000'000ULL,
      330'540'000'000ULL,   340'400'000'000ULL,   344'278'500'000ULL,
      351'660'000'000ULL,   357'260'000'000ULL,   367'789'100'000ULL,
      379'370'000'000ULL,   385'690'000'000ULL,   387'900'000'000ULL,
      391'320'000'000ULL,   406'740'000'000ULL,   411'116'500'000ULL,
      416'048'000'000ULL,   423'450'000'000ULL,   440'860'000'000ULL,
      443'965'000'000ULL,   482'310'000'000ULL,   488'679'200'000ULL,
      493'508'000'000ULL,   496'390'000'000ULL,   503'474'000'000ULL,
      520'227'000'000ULL,   523'130'000'000ULL,   526'881'000'000ULL,
      534'245'000'000ULL,   535'400'000'000ULL,   538'290'000'000ULL,
      556'560'000'000ULL,   557'910'000'000ULL,   561'200'000'000ULL,
      562'930'000'000ULL,   563'990'000'000ULL,   566'442'000'000ULL,
      571'830'000'000ULL,   586'265'000'000ULL,   595'610'000'000ULL,
      616'050'000'000ULL,   644'370'000'000ULL,   656'489'000'000ULL,
      664'780'000'000ULL,   671'155'000'000ULL,   674'675'000'000ULL,
      674'677'000'000ULL,   678'623'000'000ULL,   683'320'000'000ULL,
      686'610'000'000ULL,   688'670'000'000ULL,   696'870'000'000ULL,
      703'250'000'000ULL,   712'843'000'000ULL,   719'349'000'000ULL,
      727'990'000'000ULL,   735'400'000'000ULL,   756'120'000'000ULL,
      764'900'000'000ULL,   768'944'000'000ULL,   778'904'500'000ULL,
      794'810'000'000ULL,   805'700'000'000ULL,   810'451'000'000ULL,
      839'360'000'000ULL,   841'574'000'000ULL,   867'380'000'000ULL,
      896'580'000'000ULL,   901'181'000'000ULL,   906'010'000'000ULL,
      919'337'000'000ULL,   926'317'000'000ULL,   930'580'000'000ULL,
      937'050'000'000ULL,   958'630'000'000ULL,   963'390'000'000ULL,
      964'079'000'000ULL,   974'090'000'000ULL,   990'190'000'000ULL,
      1'001'100'000'000ULL, 1'005'272'000'000ULL, 1'084'000'000'000ULL,
      1'085'837'000'000ULL, 1'089'737'000'000ULL, 1'109'174'000'000ULL,
      1'112'076'000'000ULL, 1'139'000'000'000ULL, 1'170'930'000'000ULL,
      1'206'110'000'000ULL, 1'212'948'000'000ULL, 1'249'938'000'000ULL,
      1'261'343'000'000ULL, 1'292'778'000'000ULL, 1'299'142'000'000ULL,
      1'314'700'000'000ULL, 1'348'100'000'000ULL, 1'363'770'000'000ULL,
      1'390'360'000'000ULL, 1'408'013'000'000ULL, 1'457'643'000'000ULL,
      1'528'103'000'000ULL, 1'605'610'000'000ULL, 1'608'360'000'000ULL,
      1'635'200'000'000ULL, 1'643'600'000'000ULL, 1'647'410'000'000ULL,
      1'674'300'000'000ULL, 1'769'090'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 125U> expected_weights{
    {
      0.2841,   0.00019,   0.00035,   0.000068,  0.000059, 0.000055, 0.000196,
      0.000025, 0.00008,   0.0755,    0.000671,  0.00006,  0.00078,  0.000323,
      0.0001,   0.00442,   0.000496,  0.000031,  0.000017, 0.000738, 0.00129,
      0.00006,  0.00031,   0.2659,    0.00014,   0.00004,  0.00862,  0.0000083,
      0.00005,  0.0000296, 0.0000125, 0.0000083, 0.02238,  0.00109,  0.000032,
      0.000133, 0.0312,    0.0002929, 0.004139,  0.000368, 0.000091, 0.001533,
      0.000536, 0.000113,  0.000129,  0.000368,  0.00006,  0.000042, 0.000177,
      0.000044, 0.0000108, 0.00038,   0.00457,   0.00131,  0.000048, 0.00462,
      0.000031, 0.000092,  0.000063,  0.001437,  0.0001,   0.000194, 0.0017,
      0.000171, 0.0047,    0.000031,  0.0002,    0.00841,  0.000029, 0.000053,
      0.000961, 0.00327,   0.000106,  0.000058,  0.000054, 0.0019,   0.00088,
      0.1297,   0.000263,  0.000125,  0.00317,   0.00016,  0.00163,  0.04243,
      0.000669, 0.00084,   0.00016,   0.00429,   0.00273,  0.000729, 0.000027,
      0.00021,  0.001341,  0.145,     0.000138,  0.000315, 0.000046, 0.00665,
      0.00244,  0.1013,    0.0173,    0.00186,   0.1341,   0.000013, 0.000365,
      0.000135, 0.01416,   0.00186,   0.000336,  0.00104,  0.01633,  0.000048,
      0.000175, 0.000256,  0.000048,  0.2085,    0.00498,  0.00281,  0.000081,
      0.000053, 0.0000015, 0.000015,  0.000064,  0.00006,  0.000092,
    },
  };

  ASSERT_EQ(energies.size(), 125U);
  EXPECT_EQ(distribution.GetTableCount(), 125U);
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
  EXPECT_NEAR(weight_sum, 1.5918799L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 711.90725285670106L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 711.90725285670106L,
              0.000047943081473170221L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesAtomicXray) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[16U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB selected photon law; see reference README; each line
  // retains its energy and absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 10U> expected_energies{
    {
      6'395'000'000ULL,
      6'732'550'000ULL,
      39'522'900'000ULL,
      40'118'600'000ULL,
      42'309'300'000ULL,
      42'996'700'000ULL,
      45'477'700'000ULL,
      46'697'700'000ULL,
      48'768'700'000ULL,
      50'093'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 10U> expected_weights{
    {
      0.13,
      0.00177,
      0.208,
      0.377,
      0.00243,
      0.00437,
      0.1178,
      0.0304,
      0.00138,
      0.000363,
    },
  };

  ASSERT_EQ(energies.size(), 10U);
  EXPECT_EQ(distribution.GetTableCount(), 10U);
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
  EXPECT_NEAR(weight_sum, 0.873513L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 35.880195139053454L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 35.880195139053454L,
              1.0184233466386795e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1217817Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[17U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      74'947'600'000ULL,
      114'526'870'000ULL,
      120'407'900'000ULL,
      121'620'700'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.192,
      0.1074,
      0.0249,
      0.00568,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.32998L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 92.063419976968301L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 92.063419976968301L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion12569Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[18U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      78'860'000'000ULL,
      118'440'000'000ULL,
      124'320'000'000ULL,
      125'530'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000117,
      0.000063,
      0.0000144,
      0.0000033,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0001977L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 95.562959028831563L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 95.562959028831563L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion14801Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[19U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      101'176'000'000ULL,
      140'755'000'000ULL,
      146'636'000'000ULL,
      147'849'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000151,
      0.00004,
      0.0000091,
      0.0000021,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0002022L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 111.53633283877349L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 111.53633283877349L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1926Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[20U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      142'360'000'000ULL,
      184'750'000'000ULL,
      191'100'000'000ULL,
      192'430'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000029,
      4.14E-7,
      8.98E-8,
      2.12E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000034250L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 149.07176525547445L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 149.07176525547445L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion2076Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[21U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      160'770'000'000ULL,
      200'350'000'000ULL,
      206'230'000'000ULL,
      207'440'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000193,
      2.68E-7,
      5.75E-8,
      1.72E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000022727L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 166.94068420821050L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 166.94068420821050L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion20941Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[22U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      159'170'000'000ULL,
      201'560'000'000ULL,
      207'910'000'000ULL,
      209'240'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000188,
      2.67E-7,
      5.8E-8,
      1.37E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000022187L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 165.85454906026051L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 165.85454906026051L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion212568Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[23U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      165'734'000'000ULL,
      205'313'000'000ULL,
      211'194'000'000ULL,
      212'407'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000244,
      0.00000713,
      0.00000162,
      4.84E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000033634L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 176.98550746268657L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 176.98550746268657L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion23942Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[24U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      192'590'000'000ULL,
      232'170'000'000ULL,
      238'050'000'000ULL,
      239'260'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000018,
      2.5E-7,
      5.3E-8,
      1.6E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000002119L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 198.74908447380840L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 198.74908447380840L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion2446974Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[25U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      197'863'400'000ULL,
      237'442'700'000ULL,
      243'323'700'000ULL,
      244'536'500'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00611,
      0.001593,
      0.000359,
      0.000108,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.008170L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 208.19519527539780L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 208.19519527539780L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion251633Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[26U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      204'799'000'000ULL,
      244'378'000'000ULL,
      250'259'000'000ULL,
      251'472'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000133,
      0.00000183,
      3.91E-7,
      1.174E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000156384L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 210.91751853130755L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 210.91751853130755L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion26986Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[27U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      223'030'000'000ULL,
      262'610'000'000ULL,
      268'490'000'000ULL,
      269'700'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000036,
      8.8E-7,
      2E-7,
      5.9E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000004739L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 232.87932053175775L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 232.87932053175775L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion271131Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[28U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      220'892'000'000ULL,
      263'281'000'000ULL,
      269'628'000'000ULL,
      270'957'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000484,
      0.0000126,
      0.00000289,
      6.76E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000064566L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 231.86979605365053L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 231.86979605365053L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion275449Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[29U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      228'615'000'000ULL,
      268'194'000'000ULL,
      274'075'000'000ULL,
      275'288'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000287,
      0.00000397,
      8.6E-7,
      2.57E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000033787L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 234.77770136443011L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 234.77770136443011L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion28598Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[30U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      239'146'000'000ULL,
      278'725'000'000ULL,
      284'606'000'000ULL,
      285'819'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000506,
      0.00000118,
      2.63E-7,
      7.9E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000006582L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 248.61825265876633L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 248.61825265876633L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion2959387Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[31U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      249'104'800'000ULL,
      288'684'100'000ULL,
      294'565'100'000ULL,
      295'777'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000579,
      0.00000787,
      0.000001684,
      5.04E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000067958L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 255.16100351687807L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 255.16100351687807L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion315174Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[32U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      264'935'000'000ULL,
      307'324'000'000ULL,
      313'671'000'000ULL,
      315'000'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000198,
      0.00000465,
      0.000001052,
      2.47E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000025749L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 275.06141955027380L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 275.06141955027380L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion3162Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[33U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      269'370'000'000ULL,
      308'950'000'000ULL,
      314'830'000'000ULL,
      316'040'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000117,
      2.5E-7,
      5.7E-8,
      1.7E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000001494L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 278.25862784471218L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 278.25862784471218L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion32483Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[34U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      274'591'000'000ULL,
      316'980'000'000ULL,
      323'327'000'000ULL,
      324'656'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000384,
      0.00000662,
      0.00000147,
      3.48E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000046838L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 282.48373068875699L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 282.48373068875699L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion329425Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[35U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      282'591'000'000ULL,
      322'170'000'000ULL,
      328'051'000'000ULL,
      329'264'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000129,
      0.00000175,
      3.74E-7,
      1.12E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000015136L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 288.63570573467230L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 288.63570573467230L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion33054Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[36U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      283'710'000'000ULL,
      323'290'000'000ULL,
      329'170'000'000ULL,
      330'380'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      5.9E-7,
      8E-8,
      1.73E-8,
      5.2E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 6.925E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 289.76854440433213L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 289.76854440433213L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion3404Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[37U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      293'570'000'000ULL,
      333'150'000'000ULL,
      339'030'000'000ULL,
      340'240'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000094,
      0.00000196,
      4.37E-7,
      1.01E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000011898L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 302.15602202050765L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 302.15602202050765L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion3442785Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[38U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      294'039'800'000ULL,
      336'429'300'000ULL,
      342'775'800'000ULL,
      344'105'100'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00827,
      0.00183,
      0.000412,
      0.0000971,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0106091L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 303.70257607242839L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 303.70257607242839L,
              4.6726944094896317e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion35166Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[39U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      301'421'000'000ULL,
      343'810'000'000ULL,
      350'157'000'000ULL,
      351'486'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000041,
      8.9E-7,
      2.02E-7,
      4.7E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000005239L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 310.95028745943882L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 310.95028745943882L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion35726Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[40U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      310'430'000'000ULL,
      350'010'000'000ULL,
      355'890'000'000ULL,
      357'100'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      3.28E-7,
      4.4E-8,
      9.5E-9,
      2.22E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 3.8372E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 316.36400760971542L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 316.36400760971542L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion3677891Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[41U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      317'550'500'000ULL,
      359'940'000'000ULL,
      366'286'500'000ULL,
      367'615'800'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000715,
      0.00000974,
      0.00000211,
      5.02E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000083852L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 324.00042928731575L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 324.00042928731575L,
              4.6726944094896317e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion38569Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[42U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      338'860'000'000ULL,
      378'440'000'000ULL,
      384'320'000'000ULL,
      385'530'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000145,
      2.3E-7,
      5E-8,
      1.17E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000017417L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 345.70528851122467L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 345.70528851122467L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion3879Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[43U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 337'660'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion4111165Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[44U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      360'878'000'000ULL,
      403'267'500'000ULL,
      409'614'000'000ULL,
      410'943'300'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000425,
      0.0000848,
      0.000019,
      0.00000448,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.00053328L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 369.77558877887789L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 369.77558877887789L,
              4.6726944094896317e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion416048Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[45U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      369'215'000'000ULL,
      408'794'000'000ULL,
      414'675'000'000ULL,
      415'888'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000621,
      8.31E-7,
      1.78E-7,
      4.16E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000072606L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 375.12685656832769L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 375.12685656832769L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion42345Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[46U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      376'616'000'000ULL,
      416'195'000'000ULL,
      422'076'000'000ULL,
      423'289'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      7.2E-7,
      1.12E-7,
      2.44E-8,
      5.7E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 8.621E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 383.35316285813711L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 383.35316285813711L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion44086Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[47U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      390'620'000'000ULL,
      433'010'000'000ULL,
      439'360'000'000ULL,
      440'690'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000021,
      4.03E-7,
      9E-8,
      2.13E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000026143L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 399.24038059901312L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 399.24038059901312L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion443965KevSm15252) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[48U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      397'131'800'000ULL,
      436'711'100'000ULL,
      442'592'100'000ULL,
      443'804'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000464,
      0.0000084,
      0.00000185,
      4.32E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000057082L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 404.78273119021758L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 404.78273119021758L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion443965KevSm152139) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[49U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      397'131'800'000ULL,
      436'711'100'000ULL,
      442'592'100'000ULL,
      443'804'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0001456,
      0.0000178,
      0.00000389,
      9.16E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000168206L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 402.62568646421650L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 402.62568646421650L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion48231KevGd152137) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[50U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      432'071'000'000ULL,
      474'460'000'000ULL,
      480'807'000'000ULL,
      482'136'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      6.12E-8,
      8.26E-9,
      1.78E-9,
      5.35E-10,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 7.1775E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 438.53102082897945L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 438.53102082897945L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion48231KevSm152115) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[51U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      435'476'000'000ULL,
      475'055'000'000ULL,
      480'936'000'000ULL,
      482'149'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000449,
      6.78E-7,
      1.47E-7,
      3.46E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000053496L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 442.04323265290863L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 442.04323265290863L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion4886792Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[52U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      441'845'800'000ULL,
      481'425'100'000ULL,
      487'306'100'000ULL,
      488'518'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000476,
      0.00000811,
      0.00000178,
      4.14E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000057904L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 449.12042904807958L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 449.12042904807958L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion493508KevGd15262) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[53U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      443'270'000'000ULL,
      485'659'000'000ULL,
      492'006'000'000ULL,
      493'335'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000106,
      1.93E-7,
      4.3E-8,
      1.29E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000013089L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 451.61484185193674L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 451.61484185193674L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion493508KevSm152149) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[54U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      446'674'000'000ULL,
      486'253'000'000ULL,
      492'134'000'000ULL,
      493'347'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000106,
      1.42E-7,
      3.03E-8,
      7.1E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000012394L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 452.58737284169760L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 452.58737284169760L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion49639KevGd152136) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[55U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 446'151'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion49639KevSm1521710) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[56U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      449'556'000'000ULL,
      489'135'000'000ULL,
      495'016'000'000ULL,
      496'229'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      1.86E-7,
      2.46E-8,
      5.2E-9,
      1.23E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 2.1703E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 455.39594466202829L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 455.39594466202829L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion503474Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[57U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      453'236'000'000ULL,
      495'625'000'000ULL,
      501'972'000'000ULL,
      503'301'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000172,
      0.0000031,
      6.87E-7,
      2.05E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000021192L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 461.50095172706682L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 461.50095172706682L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion520227Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[58U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      469'989'000'000ULL,
      512'378'000'000ULL,
      518'725'000'000ULL,
      520'054'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000815,
      0.000001233,
      2.7E-7,
      8.09E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000097339L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 477.12638742949897L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 477.12638742949897L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion52313Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[59U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      476'300'000'000ULL,
      515'880'000'000ULL,
      521'760'000'000ULL,
      522'970'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000148,
      2.19E-7,
      4.8E-8,
      1.12E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000017582L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 482.76843590035263L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 482.76843590035263L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion526881Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[60U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 476'643'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion534245Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[61U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      484'007'000'000ULL,
      526'396'000'000ULL,
      532'743'000'000ULL,
      534'072'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000129,
      1.73E-7,
      3.72E-8,
      1.12E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000015114L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 490.42952494376075L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 490.42952494376075L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion53829Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[62U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      491'460'000'000ULL,
      531'040'000'000ULL,
      536'920'000'000ULL,
      538'130'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      5.1E-7,
      7.6E-8,
      1.63E-8,
      3.8E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 6.061E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 497.93817851839630L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 497.93817851839630L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion55656Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[63U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      509'726'000'000ULL,
      549'305'000'000ULL,
      555'186'000'000ULL,
      556'399'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      5.13E-7,
      6.85E-8,
      1.46E-8,
      3.43E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 5.9953E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 515.62222852901439L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 515.62222852901439L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion55791Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[64U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      507'670'000'000ULL,
      550'060'000'000ULL,
      556'410'000'000ULL,
      557'740'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      3.8E-7,
      6.6E-8,
      1.46E-8,
      4.4E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 4.650E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 515.69075698924731L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 515.69075698924731L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion5612Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[65U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      514'400'000'000ULL,
      553'900'000'000ULL,
      559'800'000'000ULL,
      561'000'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      8.5E-8,
      1.4E-8,
      3.1E-9,
      7.2E-10,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 1.0282E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 521.47344874538028L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 521.47344874538028L,
              4.3499631977081299e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion56293Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[66U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      516'096'000'000ULL,
      555'675'000'000ULL,
      561'556'000'000ULL,
      562'769'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000003,
      4.9E-7,
      1.07E-7,
      2.5E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000003622L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 523.11553478741027L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 523.11553478741027L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion56399Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[67U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      517'157'000'000ULL,
      556'736'000'000ULL,
      562'617'000'000ULL,
      563'830'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000128,
      0.00000172,
      3.67E-7,
      8.64E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000149734L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 523.08699901158054L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 523.08699901158054L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion566442Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[68U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      519'608'000'000ULL,
      559'187'000'000ULL,
      565'068'000'000ULL,
      566'281'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000153,
      0.00000217,
      4.68E-7,
      1.103E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000180483L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 525.83073244017442L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 525.83073244017442L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion586265Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[69U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 536'026'900'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion61605Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[70U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      569'216'000'000ULL,
      608'795'000'000ULL,
      614'676'000'000ULL,
      615'889'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      5.8E-7,
      9.2E-8,
      2.01E-8,
      4.72E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 6.9682E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 576.06900444878161L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 576.06900444878161L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion64437Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[71U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      597'540'000'000ULL,
      636'630'000'000ULL,
      637'060'000'000ULL,
      637'650'000'000ULL,
      643'000'000'000ULL,
      644'210'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      1.32E-7,
      1.6E-8,
      1E-9,
      1E-9,
      3.77E-9,
      1.03E-9,
    },
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
  EXPECT_NEAR(weight_sum, 1.5480E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 603.51237919896641L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 603.51237919896641L,
              6.5297236835956573e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion656489Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[72U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 609'656'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion66478Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[73U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      617'950'000'000ULL,
      657'530'000'000ULL,
      663'410'000'000ULL,
      664'620'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      5.2E-7,
      8.2E-8,
      1.8E-8,
      4.2E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 6.242E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 624.77450176225569L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 624.77450176225569L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion671155Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[74U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      624'323'000'000ULL,
      663'902'000'000ULL,
      669'783'000'000ULL,
      670'996'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000175,
      2.37E-7,
      5.04E-8,
      1.19E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000020493L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 630.28933762748256L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 630.28933762748256L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion674675Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[75U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      627'842'800'000ULL,
      667'422'100'000ULL,
      673'303'100'000ULL,
      674'515'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000323,
      4.32E-7,
      9.21E-8,
      2.16E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000037757L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 633.74721247715655L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 633.74721247715655L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion674677Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[76U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      624'438'000'000ULL,
      666'827'000'000ULL,
      673'174'000'000ULL,
      674'503'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000108,
      1.68E-7,
      3.68E-8,
      1.1E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000012958L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 631.74279379533879L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 631.74279379533879L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion678623Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[77U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      628'386'000'000ULL,
      670'775'000'000ULL,
      677'122'000'000ULL,
      678'451'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000268,
      0.00000423,
      9.31E-7,
      2.8E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000032241L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 635.78951992804193L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 635.78951992804193L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion68661Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[78U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      639'780'000'000ULL,
      679'360'000'000ULL,
      685'240'000'000ULL,
      686'450'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000134,
      1.91E-7,
      4.1E-8,
      9.7E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000015817L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 646.02412910159954L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 646.02412910159954L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion68867Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[79U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 641'838'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion70325KevGd15251) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[80U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      653'010'000'000ULL,
      695'400'000'000ULL,
      701'750'000'000ULL,
      703'080'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      9E-8,
      1.4E-8,
      3.2E-9,
      9.4E-10,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 1.0814E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 660.37539485851674L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 660.37539485851674L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion70325KevGd152102) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[81U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      653'010'000'000ULL,
      695'400'000'000ULL,
      701'750'000'000ULL,
      703'080'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      1.75E-7,
      2.8E-8,
      6.1E-9,
      1.83E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 2.1093E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 660.48101929550088L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 660.48101929550088L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion712843Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[82U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      662'606'000'000ULL,
      704'995'000'000ULL,
      711'342'000'000ULL,
      712'671'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000183,
      2.41E-7,
      5.2E-8,
      1.24E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000021354L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 668.86750931909712L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 668.86750931909712L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion719349KevSm15292) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[83U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      672'516'800'000ULL,
      712'096'100'000ULL,
      717'977'100'000ULL,
      719'189'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000118,
      0.00000178,
      3.86E-7,
      9.1E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000014057L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 679.07908796329231L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 679.07908796329231L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion719349KevSm152135) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[84U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      672'516'800'000ULL,
      712'096'100'000ULL,
      717'977'100'000ULL,
      719'189'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000001,
      1.31E-7,
      2.79E-8,
      6.5E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000011654L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 678.31447103140553L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 678.31447103140553L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion72799Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[85U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      681'160'000'000ULL,
      720'250'000'000ULL,
      720'680'000'000ULL,
      721'270'000'000ULL,
      726'620'000'000ULL,
      727'830'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      1.76E-7,
      2.1E-8,
      1E-9,
      1E-9,
      4.9E-9,
      1.34E-9,
    },
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
  EXPECT_NEAR(weight_sum, 2.0524E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 686.93768368739037L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 686.93768368739037L,
              6.5297236835956573e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion7649Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[86U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      714'663'000'000ULL,
      756'526'000'000ULL,
      756'972'000'000ULL,
      757'659'000'000ULL,
      763'399'000'000ULL,
      764'728'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00000836,
      0.00000103,
      1.6E-7,
      8E-8,
      2.77E-7,
      7.72E-8,
    },
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
  EXPECT_NEAR(weight_sum, 0.0000099842L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 721.74347715390317L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 721.74347715390317L,
              7.0039997047185898e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion768944Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[87U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      722'112'000'000ULL,
      761'209'000'000ULL,
      761'634'000'000ULL,
      762'230'000'000ULL,
      767'572'000'000ULL,
      768'785'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00000132,
      1.5E-7,
      1E-8,
      1E-8,
      3.63E-8,
      9.95E-9,
    },
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
  EXPECT_NEAR(weight_sum, 0.00000153625L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 727.82431528071603L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 727.82431528071603L,
              6.5301427787542343e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion7789045Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[88U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      728'667'500'000ULL,
      771'057'000'000ULL,
      777'403'500'000ULL,
      778'732'800'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000208,
      0.0000271,
      0.00000584,
      0.000001388,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000242328L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 734.86927745204846L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 734.86927745204846L,
              4.6726944094896317e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion79481Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[89U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      744'571'000'000ULL,
      786'960'000'000ULL,
      793'307'000'000ULL,
      794'636'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000171,
      2.38E-7,
      5.15E-8,
      1.23E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000020118L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 751.13938925340491L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 751.13938925340491L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion8057Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[90U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      758'870'000'000ULL,
      798'450'000'000ULL,
      804'330'000'000ULL,
      805'540'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      1.75E-7,
      2.2E-8,
      4.68E-9,
      1.1E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 2.0278E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 764.46645823059473L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 764.46645823059473L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion810451Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[91U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      763'619'000'000ULL,
      803'198'000'000ULL,
      809'079'000'000ULL,
      810'292'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00001046,
      0.000001563,
      3.39E-7,
      7.93E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000124413L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 770.12749074453634L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 770.12749074453634L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion83936Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[92U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      792'526'000'000ULL,
      832'105'000'000ULL,
      837'986'000'000ULL,
      839'199'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      1.92E-7,
      2.59E-8,
      5.52E-9,
      1.3E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 2.2472E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 798.47433659665361L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 798.47433659665361L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion841574Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[93U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      794'742'000'000ULL,
      834'321'000'000ULL,
      840'202'000'000ULL,
      841'415'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000196,
      2.62E-7,
      5.59E-8,
      1.315E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.00000229105L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 800.64525918247092L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 800.64525918247092L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion86738Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[94U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      820'548'800'000ULL,
      860'128'100'000ULL,
      866'009'100'000ULL,
      867'221'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000123,
      0.0000179,
      0.00000387,
      9.12E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000145682L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 826.91174599332793L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 826.91174599332793L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion901181Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[95U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      854'350'000'000ULL,
      893'929'000'000ULL,
      899'810'000'000ULL,
      901'023'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000218,
      3.21E-7,
      6.92E-8,
      1.63E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000025865L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 860.77237034602745L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 860.77237034602745L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion919337Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[96U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      872'505'800'000ULL,
      912'085'100'000ULL,
      917'966'100'000ULL,
      919'178'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000429,
      5.79E-7,
      1.236E-7,
      2.91E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000050217L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 878.45866357010574L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 878.45866357010574L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion926317Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[97U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      879'486'000'000ULL,
      919'065'000'000ULL,
      924'946'000'000ULL,
      926'159'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000683,
      9.77E-7,
      2.11E-7,
      4.97E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000080677L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 885.75549330044498L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 885.75549330044498L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion93058Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[98U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      880'340'000'000ULL,
      922'730'000'000ULL,
      929'080'000'000ULL,
      930'410'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000197,
      2.92E-7,
      6.36E-8,
      1.51E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000023407L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 887.27544708847781L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 887.27544708847781L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion93705Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[99U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      886'814'000'000ULL,
      929'203'000'000ULL,
      935'550'000'000ULL,
      936'879'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      1E-7,
      1.39E-8,
      3E-9,
      7.2E-10,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 1.1762E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 893.37293470498215L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 893.37293470498215L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion95863Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[100U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 911'800'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion96339Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[101U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      916'559'000'000ULL,
      955'656'000'000ULL,
      956'081'000'000ULL,
      956'677'000'000ULL,
      962'019'000'000ULL,
      963'232'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000001341,
      1.53E-7,
      6E-9,
      7E-9,
      3.53E-8,
      9.69E-9,
    },
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
  EXPECT_NEAR(weight_sum, 0.00000155199L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 922.07243653631789L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 922.07243653631789L,
              6.5301427787542343e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion964079Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[102U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      917'248'000'000ULL,
      956'827'000'000ULL,
      962'708'000'000ULL,
      963'921'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000334,
      0.0000474,
      0.00001019,
      0.00000239,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.00039398L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 923.46869767500888L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 923.46869767500888L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion97409Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[103U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 923'851'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion99019Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[104U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      939'951'000'000ULL,
      981'814'000'000ULL,
      982'260'000'000ULL,
      982'947'000'000ULL,
      988'687'000'000ULL,
      990'016'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      7.56E-7,
      9.3E-8,
      1.1E-8,
      5E-9,
      2.38E-8,
      6.63E-9,
    },
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
  EXPECT_NEAR(weight_sum, 8.9543E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 946.72482458706990L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 946.72482458706990L,
              7.0039997047185898e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1005272Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[105U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      958'442'000'000ULL,
      998'021'000'000ULL,
      1'003'902'000'000ULL,
      1'005'115'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000146,
      0.00000207,
      4.45E-7,
      1.051E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000172201L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 964.65936007920976L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 964.65936007920976L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1085837Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[106U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'039'007'000'000ULL,
      1'078'586'000'000ULL,
      1'084'467'000'000ULL,
      1'085'680'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000182,
      0.0000253,
      0.00000543,
      0.000001276,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000214006L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1045.1178158089026L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1045.1178158089026L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1089737Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[107U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'039'502'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1109174Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[108U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'058'939'000'000ULL,
      1'101'328'000'000ULL,
      1'107'675'000'000ULL,
      1'109'004'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000353,
      5E-7,
      1.086E-7,
      2.59E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000041645L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1065.6106083803578L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1065.6106083803578L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1112076Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[109U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'065'245'800'000ULL,
      1'104'825'100'000ULL,
      1'110'706'100'000ULL,
      1'111'918'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000228,
      0.0000319,
      0.00000685,
      0.000001609,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000268359L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1071.3908510059286L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1071.3908510059286L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion117093Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[110U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'124'100'000'000ULL,
      1'163'680'000'000ULL,
      1'169'560'000'000ULL,
      1'170'770'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      7.3E-7,
      9.67E-8,
      2.07E-8,
      4.89E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 8.5229E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1129.9625870302362L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1129.9625870302362L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion120611Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[111U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'155'870'000'000ULL,
      1'198'260'000'000ULL,
      1'204'610'000'000ULL,
      1'205'940'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      2.16E-7,
      3.04E-8,
      6.57E-9,
      1.57E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 2.5454E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1162.4995580262434L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1162.4995580262434L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1212948Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[112U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'166'119'000'000ULL,
      1'205'698'000'000ULL,
      1'211'579'000'000ULL,
      1'212'792'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000085,
      0.000001136,
      2.41E-7,
      5.69E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000099339L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1172.0153043416986L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1172.0153043416986L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1249938Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[113U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      1'203'110'000'000ULL,
      1'242'207'000'000ULL,
      1'242'632'000'000ULL,
      1'243'228'000'000ULL,
      1'248'570'000'000ULL,
      1'249'783'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000026,
      3.1E-7,
      2E-8,
      1E-8,
      7.35E-8,
      1.73E-8,
    },
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
  EXPECT_NEAR(weight_sum, 0.0000030308L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1208.8710013527781L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1208.8710013527781L,
              6.5301427787542343e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1261343Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[114U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'211'110'000'000ULL,
      1'253'499'000'000ULL,
      1'259'846'000'000ULL,
      1'261'175'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      7.73E-7,
      1.052E-7,
      2.27E-8,
      5.44E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 9.0634E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1217.5512732528632L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1217.5512732528632L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1292778Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[115U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'245'950'000'000ULL,
      1'285'529'000'000ULL,
      1'291'410'000'000ULL,
      1'292'623'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.00000135,
      1.79E-7,
      3.83E-8,
      9.04E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.00000157634L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1251.8165534846543L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1251.8165534846543L,
              4.3567618525028229e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1299142Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[116U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'248'909'000'000ULL,
      1'291'298'000'000ULL,
      1'297'645'000'000ULL,
      1'298'974'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000098,
      0.000001311,
      2.81E-7,
      6.71E-8,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.0000114591L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1255.2468586887277L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1255.2468586887277L,
              4.6726664698123932e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion13147Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[117U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'264'460'000'000ULL,
      1'306'850'000'000ULL,
      1'313'200'000'000ULL,
      1'314'530'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      2.88E-8,
      3.71E-9,
      8E-10,
      1.9E-10,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 3.350E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1270.6024537313433L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1270.6024537313433L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion13481Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[118U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'297'860'000'000ULL,
      1'340'250'000'000ULL,
      1'346'600'000'000ULL,
      1'347'930'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      2.28E-7,
      3.13E-8,
      6.77E-9,
      1.62E-9,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 2.6769E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1304.3521745302402L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1304.3521745302402L,
              4.6731321310997009e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion136377Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[119U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      1'316'940'000'000ULL,
      1'356'030'000'000ULL,
      1'356'460'000'000ULL,
      1'357'050'000'000ULL,
      1'362'400'000'000ULL,
      1'363'610'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      4.35E-7,
      5.4E-8,
      2E-9,
      1E-9,
      1.21E-8,
      2.87E-9,
    },
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
  EXPECT_NEAR(weight_sum, 5.0697E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1322.6879119080024L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1322.6879119080024L,
              6.5297236835956573e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion139036Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[120U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'343'530'000'000ULL,
      1'382'620'000'000ULL,
      1'388'990'000'000ULL,
      1'390'200'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      6.7E-8,
      8E-9,
      1.85E-9,
      4.4E-10,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 7.729E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1348.9298680294993L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1348.9298680294993L,
              4.3564824557304382e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1408013Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[121U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'361'178'800'000ULL,
      1'400'758'100'000ULL,
      1'406'639'100'000ULL,
      1'407'851'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0001043,
      0.00001282,
      0.00000273,
      6.42E-7,
    },
  };

  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  EXPECT_NEAR(weight_sum, 0.000120492L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1366.6686032666069L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1366.6686032666069L,
              4.3567711657285690e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1457643Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[122U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      1'410'817'000'000ULL,
      1'449'914'000'000ULL,
      1'450'339'000'000ULL,
      1'450'935'000'000ULL,
      1'456'277'000'000ULL,
      1'457'490'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00000249,
      2.7E-7,
      1E-8,
      1E-8,
      6.13E-8,
      1.444E-8,
    },
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
  EXPECT_NEAR(weight_sum, 0.00000285574L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1416.0041830488770L, 1.0e-9L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1416.0041830488770L,
              6.5301427787542343e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion1528103Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[123U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'481'277'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion160561Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[124U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'555'370'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion160836Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[125U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'561'530'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion16352Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[126U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'588'400'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion16436Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[127U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'593'360'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion164741Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[128U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'600'580'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEu152Test, PreservesConversion176909Kev) {
  auto const definition = BuildEu152Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[129U].GetEnergyDistribution();
  // Selected LNHB absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'722'260'000'000ULL);
}

} // namespace
