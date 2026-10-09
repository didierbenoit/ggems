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
 * \brief Validates device World navigation against independent exact ray
 * fixtures.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <limits>
#include <memory>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRecord.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRange.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/transport/GGEMSTransportCounters.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {
using namespace ggems;
using namespace ggems::core;

struct WorldCase {
  std::string_view name;
  std::array<std::int64_t, 3> half;
  std::array<std::int64_t, 3> start;
  std::array<float, 3> direction;
  std::array<std::int64_t, 3> end;
  std::uint32_t faces;
  std::uint32_t outcome;
};

/*! \brief Gives deterministic, readable names to CTest parameter cases. */
auto PrintTo(WorldCase const &value, std::ostream *output) -> void {
  *output << value.name;
}

// Exact rational goldens for the stored binary32 inputs. No float epsilon.
// W01 is 200 mm full size and +X; travel is exactly 100 mm = 10^11 pm.
constexpr std::array k_cases{
  WorldCase{.name = "W01",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {100000000000LL, 0LL, 0LL},
            .faces = 2U,
            .outcome = 0U},
  WorldCase{.name = "CenterMinusX",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {-0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {-100000000000LL, 0LL, 0LL},
            .faces = 1U,
            .outcome = 0U},
  WorldCase{.name = "CenterPlusY",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x0.0p+0F, 0x1.0000000000000p+0F, 0x0.0p+0F},
            .end = {0LL, 100000000000LL, 0LL},
            .faces = 8U,
            .outcome = 0U},
  WorldCase{.name = "CenterMinusY",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x0.0p+0F, -0x1.0000000000000p+0F, 0x0.0p+0F},
            .end = {0LL, -100000000000LL, 0LL},
            .faces = 4U,
            .outcome = 0U},
  WorldCase{.name = "CenterPlusZ",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x0.0p+0F, 0x0.0p+0F, 0x1.0000000000000p+0F},
            .end = {0LL, 0LL, 100000000000LL},
            .faces = 32U,
            .outcome = 0U},
  WorldCase{.name = "CenterMinusZ",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x0.0p+0F, 0x0.0p+0F, -0x1.0000000000000p+0F},
            .end = {0LL, 0LL, -100000000000LL},
            .faces = 16U,
            .outcome = 0U},
  WorldCase{.name = "OffCenterAsymmetric",
            .half = {100000000000LL, 200000000000LL, 300000000000LL},
            .start = {23000000001LL, -57LL, 813LL},
            .direction = {-0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {-100000000000LL, -57LL, 813LL},
            .faces = 1U,
            .outcome = 0U},
  WorldCase{
    .name = "DiagonalFace",
    .half = {100000000000LL, 200000000000LL, 300000000000LL},
    .start = {0LL, 0LL, 0LL},
    .direction = {0x1.6a09e60000000p-1F, 0x1.6a09e60000000p-1F, 0x0.0p+0F},
    .end = {100000000000LL, 100000000000LL, 0LL},
    .faces = 2U,
    .outcome = 0U},
  WorldCase{
    .name = "ExactEdge",
    .half = {100000000000LL, 100000000000LL, 200000000000LL},
    .start = {0LL, 0LL, 0LL},
    .direction = {0x1.6a09e60000000p-1F, -0x1.6a09e60000000p-1F, 0x0.0p+0F},
    .end = {100000000000LL, -100000000000LL, 0LL},
    .faces = 6U,
    .outcome = 0U},
  WorldCase{.name = "ExactCorner",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x1.279a740000000p-1F, 0x1.279a740000000p-1F,
                          -0x1.279a740000000p-1F},
            .end = {100000000000LL, 100000000000LL, -100000000000LL},
            .faces = 26U,
            .outcome = 0U},
  WorldCase{
    .name = "ObliqueUnequal",
    .half = {29LL, 37LL, 53LL},
    .start = {-7LL, 3LL, -11LL},
    .direction = {0x1.3333340000000p-1F, 0x1.99999a0000000p-1F, 0x0.0p+0F},
    .end = {19LL, 37LL, -11LL},
    .faces = 8U,
    .outcome = 0U},
  WorldCase{.name = "FaceOut0P",
            .half = {11LL, 23LL, 37LL},
            .start = {11LL, 0LL, 0LL},
            .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {11LL, 0LL, 0LL},
            .faces = 2U,
            .outcome = 0U},
  WorldCase{.name = "FaceIn0P",
            .half = {11LL, 23LL, 37LL},
            .start = {11LL, 0LL, 0LL},
            .direction = {-0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {-11LL, 0LL, 0LL},
            .faces = 1U,
            .outcome = 0U},
  WorldCase{.name = "FaceOut0M",
            .half = {11LL, 23LL, 37LL},
            .start = {-11LL, 0LL, 0LL},
            .direction = {-0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {-11LL, 0LL, 0LL},
            .faces = 1U,
            .outcome = 0U},
  WorldCase{.name = "FaceIn0M",
            .half = {11LL, 23LL, 37LL},
            .start = {-11LL, 0LL, 0LL},
            .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {11LL, 0LL, 0LL},
            .faces = 2U,
            .outcome = 0U},
  WorldCase{.name = "FaceOut1P",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, 23LL, 0LL},
            .direction = {0x0.0p+0F, 0x1.0000000000000p+0F, 0x0.0p+0F},
            .end = {0LL, 23LL, 0LL},
            .faces = 8U,
            .outcome = 0U},
  WorldCase{.name = "FaceIn1P",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, 23LL, 0LL},
            .direction = {0x0.0p+0F, -0x1.0000000000000p+0F, 0x0.0p+0F},
            .end = {0LL, -23LL, 0LL},
            .faces = 4U,
            .outcome = 0U},
  WorldCase{.name = "FaceOut1M",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, -23LL, 0LL},
            .direction = {0x0.0p+0F, -0x1.0000000000000p+0F, 0x0.0p+0F},
            .end = {0LL, -23LL, 0LL},
            .faces = 4U,
            .outcome = 0U},
  WorldCase{.name = "FaceIn1M",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, -23LL, 0LL},
            .direction = {0x0.0p+0F, 0x1.0000000000000p+0F, 0x0.0p+0F},
            .end = {0LL, 23LL, 0LL},
            .faces = 8U,
            .outcome = 0U},
  WorldCase{.name = "FaceOut2P",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, 0LL, 37LL},
            .direction = {0x0.0p+0F, 0x0.0p+0F, 0x1.0000000000000p+0F},
            .end = {0LL, 0LL, 37LL},
            .faces = 32U,
            .outcome = 0U},
  WorldCase{.name = "FaceIn2P",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, 0LL, 37LL},
            .direction = {0x0.0p+0F, 0x0.0p+0F, -0x1.0000000000000p+0F},
            .end = {0LL, 0LL, -37LL},
            .faces = 16U,
            .outcome = 0U},
  WorldCase{.name = "FaceOut2M",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, 0LL, -37LL},
            .direction = {0x0.0p+0F, 0x0.0p+0F, -0x1.0000000000000p+0F},
            .end = {0LL, 0LL, -37LL},
            .faces = 16U,
            .outcome = 0U},
  WorldCase{.name = "FaceIn2M",
            .half = {11LL, 23LL, 37LL},
            .start = {0LL, 0LL, -37LL},
            .direction = {0x0.0p+0F, 0x0.0p+0F, 0x1.0000000000000p+0F},
            .end = {0LL, 0LL, 37LL},
            .faces = 32U,
            .outcome = 0U},
  WorldCase{.name = "FaceCoincident",
            .half = {11LL, 23LL, 37LL},
            .start = {11LL, 0LL, 0LL},
            .direction = {0x0.0p+0F, 0x1.0000000000000p+0F, 0x0.0p+0F},
            .end = {11LL, 23LL, 0LL},
            .faces = 8U,
            .outcome = 0U},
  WorldCase{.name = "EdgeCoincident",
            .half = {11LL, 23LL, 37LL},
            .start = {-11LL, 23LL, 0LL},
            .direction = {-0x0.0p+0F, 0x0.0p+0F, 0x1.0000000000000p+0F},
            .end = {-11LL, 23LL, 37LL},
            .faces = 32U,
            .outcome = 0U},
  WorldCase{.name = "CornerIn",
            .half = {11LL, 11LL, 11LL},
            .start = {11LL, 11LL, 11LL},
            .direction = {-0x1.279a740000000p-1F, -0x1.279a740000000p-1F,
                          -0x1.279a740000000p-1F},
            .end = {-11LL, -11LL, -11LL},
            .faces = 21U,
            .outcome = 0U},
  WorldCase{
    .name = "CornerMixedOut",
    .half = {11LL, 11LL, 11LL},
    .start = {11LL, 11LL, 11LL},
    .direction = {-0x1.6a09e60000000p-1F, 0x1.6a09e60000000p-1F, 0x0.0p+0F},
    .end = {11LL, 11LL, 11LL},
    .faces = 8U,
    .outcome = 0U},
  WorldCase{.name = "OnePmInside",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {99999999999LL, 0LL, 0LL},
            .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {100000000000LL, 0LL, 0LL},
            .faces = 2U,
            .outcome = 0U},
  WorldCase{.name = "OnePmOutside",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {100000000001LL, 0LL, 0LL},
            .direction = {-0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {100000000001LL, 0LL, 0LL},
            .faces = 0U,
            .outcome = 1U},
  WorldCase{.name = "OutsideInt64Min",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {std::numeric_limits<std::int64_t>::min(), 0LL, 0LL},
            .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {std::numeric_limits<std::int64_t>::min(), 0LL, 0LL},
            .faces = 0U,
            .outcome = 1U},
  WorldCase{.name = "SmallestWorld",
            .half = {1LL, 1LL, 1LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {1LL, 0LL, 0LL},
            .faces = 2U,
            .outcome = 0U},
  WorldCase{
    .name = "NegativeEndpointHalf",
    .half = {1LL, 5LL, 5LL},
    .start = {0LL, -1LL, 0LL},
    .direction = {0x1.c9f25c0000000p-1F, 0x1.c9f25c0000000p-2F, 0x0.0p+0F},
    .end = {1LL, -1LL, 0LL},
    .faces = 2U,
    .outcome = 0U},
  WorldCase{
    .name = "PositiveEndpointHalf",
    .half = {1LL, 5LL, 5LL},
    .start = {0LL, 1LL, 0LL},
    .direction = {-0x1.c9f25c0000000p-1F, -0x1.c9f25c0000000p-2F, 0x0.0p+0F},
    .end = {-1LL, 1LL, 0LL},
    .faces = 1U,
    .outcome = 0U},
  WorldCase{
    .name = "PositiveHalfAway",
    .half = {1LL, 5LL, 5LL},
    .start = {0LL, 0LL, 0LL},
    .direction = {0x1.c9f25c0000000p-1F, 0x1.c9f25c0000000p-2F, 0x0.0p+0F},
    .end = {1LL, 1LL, 0LL},
    .faces = 2U,
    .outcome = 0U},
  WorldCase{
    .name = "NegativeHalfAway",
    .half = {1LL, 5LL, 5LL},
    .start = {0LL, 0LL, 0LL},
    .direction = {-0x1.c9f25c0000000p-1F, -0x1.c9f25c0000000p-2F, 0x0.0p+0F},
    .end = {-1LL, -1LL, 0LL},
    .faces = 1U,
    .outcome = 0U},
  WorldCase{
    .name = "BeyondDoubleLattice",
    .half = {9007199254740993LL, 9007199254740993LL, 9007199254740993LL},
    .start = {9007199254740992LL, -9007199254740986LL, 19LL},
    .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
    .end = {9007199254740993LL, -9007199254740986LL, 19LL},
    .faces = 2U,
    .outcome = 0U},
  WorldCase{.name = "MaximumWorldAxis",
            .half = {4611686018427387903LL, 4611686018427387903LL,
                     4611686018427387903LL},
            .start = {-4611686018427387903LL, 1LL, 3LL},
            .direction = {0x1.0000000000000p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {4611686018427387903LL, 1LL, 3LL},
            .faces = 2U,
            .outcome = 0U},
  WorldCase{.name = "MaximumWorldDiagonal",
            .half = {4611686018427387903LL, 4611686018427387903LL,
                     4611686018427387903LL},
            .start = {-4611686018427387903LL, -4611686018427387903LL,
                      -4611686018427387903LL},
            .direction = {0x1.279a740000000p-1F, 0x1.279a740000000p-1F,
                          0x1.279a740000000p-1F},
            .end = {4611686018427387903LL, 4611686018427387903LL,
                    4611686018427387903LL},
            .faces = 42U,
            .outcome = 0U},
  WorldCase{
    .name = "NearTieOnePm",
    .half = {4611686018427387903LL, 4611686018427387903LL,
             4611686018427387903LL},
    .start = {0LL, 1LL, 0LL},
    .direction = {0x1.6a09e60000000p-1F, 0x1.6a09e60000000p-1F, 0x0.0p+0F},
    .end = {4611686018427387902LL, 4611686018427387903LL, 0LL},
    .faces = 8U,
    .outcome = 0U},
  WorldCase{
    .name = "NearTieSmall",
    .half = {13LL, 13LL, 13LL},
    .start = {0LL, 1LL, 0LL},
    .direction = {0x1.6a09e60000000p-1F, 0x1.6a09e60000000p-1F, 0x0.0p+0F},
    .end = {12LL, 13LL, 0LL},
    .faces = 8U,
    .outcome = 0U},
  WorldCase{
    .name = "RoundedButNotCrossed",
    .half = {100LL, 100LL, 100LL},
    .start = {99LL, 99LL, 0LL},
    .direction = {0x1.c9f25c0000000p-1F, 0x1.c9f25c0000000p-2F, 0x0.0p+0F},
    .end = {100LL, 100LL, 0LL},
    .faces = 2U,
    .outcome = 0U},
  WorldCase{
    .name = "TinyComponent",
    .half = {100000000000LL, 100000000000LL, 100000000000LL},
    .start = {0LL, 0LL, 0LL},
    .direction = {0x1.0000000000000p-126F, 0x0.0p+0F, 0x1.0000000000000p+0F},
    .end = {0LL, 0LL, 100000000000LL},
    .faces = 32U,
    .outcome = 0U},
  WorldCase{
    .name = "SubnormalComponent",
    .half = {100000000000LL, 100000000000LL, 100000000000LL},
    .start = {0LL, 0LL, 0LL},
    .direction = {0x1.0000000000000p-149F, 0x0.0p+0F, 0x1.0000000000000p+0F},
    .end = {0LL, 0LL, 100000000000LL},
    .faces = 32U,
    .outcome = 0U},
  WorldCase{.name = "ZeroDirection",
            .half = {100000000000LL, 100000000000LL, 100000000000LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x0.0p+0F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {0LL, 0LL, 0LL},
            .faces = 0U,
            .outcome = 2U},
  // The unit-direction parameter (2^62-1) * 2^126 pm fits no integer, but
  // the endpoint is representable: the exact witness commits it.
  WorldCase{.name = "HugeParameterStillExits",
            .half = {4611686018427387903LL, 4611686018427387903LL,
                     4611686018427387903LL},
            .start = {0LL, 0LL, 0LL},
            .direction = {0x1.0000000000000p-126F, 0x0.0p+0F, 0x0.0p+0F},
            .end = {4611686018427387903LL, 0LL, 0LL},
            .faces = 2U,
            .outcome = 0U}};

class GGEMSWorldTransportTest : public testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ocl::GGEMSOpenCL::GetInstance();
    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"all"});
      opencl.Initialize();
    }
    ASSERT_FALSE(opencl.GetContext().empty());
  }
};

class GGEMSWorldRayTest : public GGEMSWorldTransportTest,
                          public testing::WithParamInterface<WorldCase> {};
} // namespace

TEST_P(GGEMSWorldRayTest, ActualStreamKernel) {
  auto const &test = GetParam();
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    auto buffer = [&context](auto const &value) -> ocl::GGEMSOpenCLSVMBuffer {
      auto result = context.CreateSVMBuffer(units::Bytes{sizeof(value)});
      ocl::WriteSVMFromHost(result, value);
      return result;
    };
    for (auto const *engine : {"philox", "pcg32", "jkiss"}) {
      SCOPED_TRACE(engine);
      random::GGEMSRandom random{};
      random.SetEngine(engine);
      random.SetSeed(91231ULL);
      auto rng = context.CreateSVMBuffer(units::Bytes{random.GetStateSize()});
      std::vector<std::byte> rng_before(random.GetStateSize());
      random.InitializeStates(0ULL, std::span<std::byte>{rng_before});
      ocl::WriteSVMFromHost(rng, std::span<std::byte const>{rng_before});

      geometry::GGEMSWorldRecord const world{.half_extent_x_pm = test.half[0],
                                             .half_extent_y_pm = test.half[1],
                                             .half_extent_z_pm = test.half[2]};
      auto world_buffer = buffer(world);
      sources::GGEMSSourceRecord source{};
      source.emitted_particle_type =
        particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
      source.position_x_pm = test.start[0];
      source.position_y_pm = test.start[1];
      source.position_z_pm = test.start[2];
      source.axis_z_x = test.direction[0];
      source.axis_z_y = test.direction[1];
      source.axis_z_z = test.direction[2];
      source.energy_micro_eV = 123456789ULL;
      source.time_start_ps = 17ULL;
      source.time_stop_ps = 17ULL;
      auto source_buffer = buffer(source);
      auto ranges = buffer(sources::GGEMSSourceRunRange{
        .projection_primary_begin = 0ULL, .primary_count = 1ULL});
      auto populations = buffer(sources::GGEMSSourcePopulationRecord{});
      auto emissions = buffer(sources::GGEMSSourceEmissionRecord{});
      auto emission_ranges = buffer(sources::GGEMSSourceEmissionRange{});
      sources::GGEMSEnergyDistributionRecord energy{};
      energy.distribution_type = sources::ToKernelEnergyDistributionType(
        sources::GGEMSEnergyDistributionType::Mono);
      auto energies = buffer(energy);
      auto values = buffer(std::uint64_t{0});
      auto tickets = buffer(std::uint64_t{0});
      auto counters_buffer = buffer(transport::GGEMSTransportCounters{});
      observer::GGEMSObserverConfigRecord capture{};
      capture.enabled = 1U;
      capture.capture_first_primary_count_per_source = 1U;
      auto capture_buffer = buffer(capture);
      auto observed_counts = buffer(observer::GGEMSObserverCounters{});
      auto observations = context.CreateSVMBuffer(
        units::Bytes{2 * sizeof(observer::GGEMSObserverRecord)});
      ocl::FillSVMFromHost(observations, 2U, observer::GGEMSObserverRecord{});

      std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
      auto const &program = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
        context, root / "transport", "particle_stream_transport",
        std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                    root.generic_string(), random.GetKernelBuildDefinition()));
      ocl::GGEMSOpenCLKernel kernel{
        context, program.CreateKernel("particle_stream_transport"),
        "particle_stream_transport"};
      kernel.SetArgSVMPointer(0U, rng.GetData());
      kernel.SetArgSVMPointer(1U, counters_buffer.GetData());
      kernel.SetArgSVMPointer(2U, source_buffer.GetData());
      kernel.SetArgSVMPointer(3U, ranges.GetData());
      kernel.SetArg(4U, cl_uint{1});
      kernel.SetArg(5U, cl_uint{1});
      kernel.SetArg(6U, cl_ulong{0});
      kernel.SetArg(7U, cl_ulong{0});
      kernel.SetArgSVMPointer(8U, capture_buffer.GetData());
      kernel.SetArgSVMPointer(9U, observed_counts.GetData());
      kernel.SetArgSVMPointer(10U, observations.GetData());
      kernel.SetArg(11U, cl_uint{2});
      kernel.SetArg(12U, cl_ulong{0});
      kernel.SetArg(13U, cl_uint{1});
      kernel.SetArgSVMPointer(14U, energies.GetData());
      kernel.SetArgSVMPointer(15U, values.GetData());
      kernel.SetArgSVMPointer(16U, tickets.GetData());
      kernel.SetArgSVMPointer(17U, populations.GetData());
      kernel.SetArgSVMPointer(18U, emissions.GetData());
      kernel.SetArgSVMPointer(19U, emission_ranges.GetData());
      kernel.SetArgSVMPointer(20U, world_buffer.GetData());
      kernel.Run({64U}, {64U});

      std::vector<std::byte> rng_after(rng_before.size());
      ocl::ReadSVMToHost(rng, std::span<std::byte>{rng_after});
      EXPECT_EQ(rng_after, rng_before);
      auto const counts =
        ocl::ReadSVMToHost<transport::GGEMSTransportCounters>(counters_buffer);
      EXPECT_EQ(counts.consumed_primary_count, 1U);
      EXPECT_EQ(counts.completed_history_count, test.outcome == 0U ? 1U : 0U);
      EXPECT_EQ(counts.terminal_particle_count, test.outcome == 0U ? 1U : 0U);
      EXPECT_EQ(counts.outside_world_count, test.outcome == 1U ? 1U : 0U);
      EXPECT_EQ(counts.unresolved_geometry_count, test.outcome == 2U ? 1U : 0U);
      EXPECT_EQ(counts.escaped_world_count, test.outcome == 0U ? 1U : 0U);
      EXPECT_EQ(counts.overflow_count, 0U);
      EXPECT_EQ(counts.created_secondary_count, 0U);
      EXPECT_EQ(counts.total_fake_step_count, 0U);
      EXPECT_EQ(counts.aionino_to_gamma_count, 0U);
      EXPECT_EQ(counts.gamma_to_electron_count, 0U);
      EXPECT_EQ(counts.electron_to_electron_count, 0U);
      auto const capture_counts =
        ocl::ReadSVMToHost<observer::GGEMSObserverCounters>(observed_counts);
      ASSERT_EQ(capture_counts.record_count, test.outcome == 0U ? 2U : 1U);
      EXPECT_EQ(capture_counts.overflow_count, 0U);
      std::array<observer::GGEMSObserverRecord, 2> records{};
      ocl::ReadSVMToHost(observations,
                         std::span<observer::GGEMSObserverRecord>{records});
      auto const &birth = records[0];
      EXPECT_EQ(birth.position_x_pm, test.start[0]);
      EXPECT_EQ(birth.position_y_pm, test.start[1]);
      EXPECT_EQ(birth.position_z_pm, test.start[2]);
      EXPECT_EQ(birth.status, particles::ToKernelParticleStatus(
                                particles::GGEMSParticleStatus::Alive));
      if (test.outcome != 0U) {
        continue;
      }
      auto const &end = records[1];
      EXPECT_EQ(end.position_x_pm, test.end[0]);
      EXPECT_EQ(end.position_y_pm, test.end[1]);
      EXPECT_EQ(end.position_z_pm, test.end[2]);
      EXPECT_EQ(end.status, particles::ToKernelParticleStatus(
                              particles::GGEMSParticleStatus::EscapedWorld));
      EXPECT_EQ(end.energy_micro_eV, source.energy_micro_eV);
      EXPECT_EQ(end.deposited_energy_micro_eV, 0ULL);
      EXPECT_EQ(end.time_ps, 17ULL);
      EXPECT_EQ(end.generation, 0U);
      EXPECT_EQ(end.direction_x, test.direction[0]);
      EXPECT_EQ(end.direction_y, test.direction[1]);
      EXPECT_EQ(end.direction_z, test.direction[2]);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(World, GGEMSWorldRayTest, testing::ValuesIn(k_cases),
                         [](testing::TestParamInfo<WorldCase> const &info)
                           -> std::string {
                           return std::string{info.param.name};
                         });

TEST_F(GGEMSWorldTransportTest,
       RunOwnsWorldAndRejectsOutsideWithoutPublication) {
  using namespace units;
  auto source = std::make_shared<sources::GGEMSSource>();
  source->SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
    .SetDirection(1, 0, 0);
  source->SetPrimaryCount(1ULL);
  auto rng = std::make_shared<random::GGEMSRandom>();
  auto observer = std::make_shared<observer::GGEMSTransportObserver>();
  observer->Enable().SetRecordCapacity(2U).CaptureFirstPrimaries(1U);
  GGEMSRun run{};
  run.SetRandom(rng);
  run.AddSource(source);
  run.SetObserver(observer);
  run.SetWorld(
    geometry::GGEMSWorld{200_mm, 200_mm, 200_mm,
                         materials::builtins::BuildBuiltInMaterial("Vacuum")});
  ASSERT_NO_THROW(run.Initialize());
  EXPECT_THROW(
    run.SetWorld(geometry::GGEMSWorld{
      2_mm, 2_mm, 2_mm, materials::builtins::BuildBuiltInMaterial("Vacuum")}),
    GGEMSRecoverable);
  ASSERT_NO_THROW(run.Run());
  auto const successful = observer->GetRecords();
  ASSERT_EQ(successful.size(), 2U);
  EXPECT_EQ(successful.back().position_x_pm, 100'000'000'000LL);
  // One picometer outside the +X face: rejected by the static origin
  // preflight before any launch, so nothing new is published.
  source->SetPositionPicoMeter(100'000'000'001LL, 0LL, 0LL);
  EXPECT_THROW(run.Run(), GGEMSRecoverable);
  EXPECT_EQ(observer->GetRecords().size(), successful.size());
  EXPECT_EQ(observer->GetRecords().back().position_x_pm,
            successful.back().position_x_pm);
}

TEST_F(GGEMSWorldTransportTest,
       RunRejectsDeviceOutsideBirthWithoutPublishingAndRecovers) {
  using namespace units;
  // Point origin inside a 2 pm World; a later rectangular emission generates
  // births outside the World on the device, exercising the counter-to-error
  // path of GGEMSRun rather than the static origin preflight.
  auto source = std::make_shared<sources::GGEMSSource>();
  source->SetAnalytic()
    .SetPrimaryCount(1ULL)
    .SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
    .SetDirection(0, 0, 1);
  auto rng = std::make_shared<random::GGEMSRandom>();
  rng->SetSeed(41ULL);
  auto observer = std::make_shared<observer::GGEMSTransportObserver>();
  observer->Enable().SetRecordCapacity(2U).CaptureFirstPrimaries(1U);
  GGEMSRun run{};
  run.SetRandom(rng);
  run.AddSource(source);
  run.SetObserver(observer);
  run.SetWorld(geometry::GGEMSWorld{
    2_pm, 2_pm, 2_pm, materials::builtins::BuildBuiltInMaterial("Vacuum")});
  ASSERT_NO_THROW(run.Initialize());
  ASSERT_NO_THROW(run.Run());
  auto const before = observer->GetRecords();
  ASSERT_EQ(before.size(), 2U);
  EXPECT_EQ(before.back().position_z_pm, 1LL);

  source->SetRectangleEmissionPicoMeter(200'000'000'000ULL, 200'000'000'000ULL);
  EXPECT_THROW(run.Run(), GGEMSRecoverable);
  auto const after = observer->GetRecords();
  ASSERT_EQ(after.size(), before.size());
  EXPECT_EQ(std::memcmp(after.data(), before.data(),
                        before.size() * sizeof(observer::GGEMSObserverRecord)),
            0);

  source->SetPointEmission();
  ASSERT_NO_THROW(run.Run());
  ASSERT_EQ(observer->GetRecords().size(), 2U);
  EXPECT_EQ(observer->GetRecords().back().position_z_pm, 1LL);
  EXPECT_EQ(observer->GetRecords().back().status,
            particles::ToKernelParticleStatus(
              particles::GGEMSParticleStatus::EscapedWorld));
}

TEST_F(GGEMSWorldTransportTest, AioninoRequiresExplicitWorld) {
  auto source = std::make_shared<sources::GGEMSSource>();
  source->SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
    .SetPrimaryCount(1ULL);
  GGEMSRun run{};
  run.SetRandom(std::make_shared<random::GGEMSRandom>());
  run.AddSource(source);
  ASSERT_NO_THROW(run.Initialize());
  EXPECT_THROW(run.Run(), GGEMSRecoverable);
  EXPECT_FALSE(run.GetLastSourceRunSnapshot().has_value());
}

TEST_F(GGEMSWorldTransportTest, ExactCrossingMasksThroughTheSharedHelper) {
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
    auto const &program = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "tests", "world_navigation_probe",
      std::format("-I{}", root.generic_string()));
    ocl::GGEMSOpenCLKernel kernel{
      context, program.CreateKernel("world_navigation_probe"),
      "world_navigation_probe"};
    for (auto const &test : k_cases) {
      SCOPED_TRACE(test.name);
      geometry::GGEMSWorldRecord const world{.half_extent_x_pm = test.half[0],
                                             .half_extent_y_pm = test.half[1],
                                             .half_extent_z_pm = test.half[2]};
      auto world_buffer = context.CreateSVMBuffer(units::Bytes{sizeof(world)});
      ocl::WriteSVMFromHost(world_buffer, world);
      auto positions =
        context.CreateSVMBuffer(units::Bytes{sizeof(test.start)});
      ocl::WriteSVMFromHost(positions,
                            std::span<std::int64_t const>{test.start});
      auto directions =
        context.CreateSVMBuffer(units::Bytes{sizeof(test.direction)});
      ocl::WriteSVMFromHost(directions, std::span<float const>{test.direction});
      auto statuses = context.CreateSVMBuffer(units::Bytes{sizeof(cl_uint)});
      auto endpoints =
        context.CreateSVMBuffer(units::Bytes{sizeof(test.start)});
      auto faces = context.CreateSVMBuffer(units::Bytes{sizeof(cl_uint)});
      kernel.SetArgSVMPointer(0U, world_buffer.GetData());
      kernel.SetArgSVMPointer(1U, positions.GetData());
      kernel.SetArgSVMPointer(2U, directions.GetData());
      kernel.SetArg(3U, cl_uint{1});
      kernel.SetArgSVMPointer(4U, statuses.GetData());
      kernel.SetArgSVMPointer(5U, endpoints.GetData());
      kernel.SetArgSVMPointer(6U, faces.GetData());
      kernel.Run({64U}, {64U});
      EXPECT_EQ(ocl::ReadSVMToHost<cl_uint>(statuses), test.outcome);
      EXPECT_EQ(ocl::ReadSVMToHost<cl_uint>(faces), test.faces);
      std::array<std::int64_t, 3> endpoint{};
      ocl::ReadSVMToHost(endpoints, std::span<std::int64_t>{endpoint});
      EXPECT_EQ(endpoint, test.outcome == 0U ? test.end : test.start);
    }
  }
}

TEST_F(GGEMSWorldTransportTest, DeviceLayoutsAndArrayStrides) {
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    std::array<geometry::GGEMSWorldRecord, 2> const worlds{
      {{.half_extent_x_pm = 11, .half_extent_y_pm = 23, .half_extent_z_pm = 37},
       {.half_extent_x_pm = 41,
        .half_extent_y_pm = 59,
        .half_extent_z_pm = 83}}};
    auto world_buffer = context.CreateSVMBuffer(units::Bytes{sizeof(worlds)});
    ocl::WriteSVMFromHost(world_buffer,
                          std::span<geometry::GGEMSWorldRecord const>{worlds});
    std::array<transport::GGEMSTransportCounters, 2> counts{};
    auto counter_buffer = context.CreateSVMBuffer(units::Bytes{sizeof(counts)});
    ocl::WriteSVMFromHost(
      counter_buffer,
      std::span<transport::GGEMSTransportCounters const>{counts});
    std::array<observer::GGEMSObserverRecord, 2> observed{};
    auto observer_buffer =
      context.CreateSVMBuffer(units::Bytes{sizeof(observed)});
    ocl::WriteSVMFromHost(
      observer_buffer,
      std::span<observer::GGEMSObserverRecord const>{observed});
    std::array<std::uint64_t, 17> layout{};
    auto output = context.CreateSVMBuffer(units::Bytes{sizeof(layout)});
    std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
    auto const &program = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "tests", "world_abi_probe",
      std::format("-I{}", root.generic_string()));
    ocl::GGEMSOpenCLKernel kernel{
      context, program.CreateKernel("world_abi_probe"), "world_abi_probe"};
    kernel.SetArgSVMPointer(0U, world_buffer.GetData());
    kernel.SetArgSVMPointer(1U, counter_buffer.GetData());
    kernel.SetArgSVMPointer(2U, observer_buffer.GetData());
    kernel.SetArgSVMPointer(3U, output.GetData());
    kernel.Run({1U}, {1U});
    ocl::ReadSVMToHost(output, std::span<std::uint64_t>{layout});
    std::array<std::uint64_t, 17> const expected{
      sizeof(geometry::GGEMSWorldRecord),
      offsetof(geometry::GGEMSWorldRecord, half_extent_y_pm),
      offsetof(geometry::GGEMSWorldRecord, half_extent_z_pm),
      alignof(geometry::GGEMSWorldRecord),
      sizeof(geometry::GGEMSWorldRecord),
      sizeof(transport::GGEMSTransportCounters),
      offsetof(transport::GGEMSTransportCounters, outside_world_count),
      offsetof(transport::GGEMSTransportCounters, unresolved_geometry_count),
      offsetof(transport::GGEMSTransportCounters, escaped_world_count),
      alignof(transport::GGEMSTransportCounters),
      sizeof(transport::GGEMSTransportCounters),
      sizeof(observer::GGEMSObserverRecord),
      alignof(observer::GGEMSObserverRecord),
      sizeof(observer::GGEMSObserverRecord),
      41ULL,
      59ULL,
      83ULL};
    EXPECT_EQ(layout, expected);
    ocl::ReadSVMToHost(counter_buffer,
                       std::span<transport::GGEMSTransportCounters>{counts});
    EXPECT_EQ(counts[1].outside_world_count, 7U);
    EXPECT_EQ(counts[1].unresolved_geometry_count, 11U);
    EXPECT_EQ(counts[1].escaped_world_count, 13U);
    ocl::ReadSVMToHost(observer_buffer,
                       std::span<observer::GGEMSObserverRecord>{observed});
    EXPECT_EQ(observed[1].energy_micro_eV, 9223372036854775806ULL);
  }
}

TEST_F(GGEMSWorldTransportTest, OtherSpeciesKeepDiagnosticMovementWithWorld) {
  using namespace units;
  for (auto type : {particles::GGEMSParticleType::Gamma,
                    particles::GGEMSParticleType::Electron,
                    particles::GGEMSParticleType::Positron,
                    particles::GGEMSParticleType::Proton,
                    particles::GGEMSParticleType::Neutron,
                    particles::GGEMSParticleType::Alpha}) {
    SCOPED_TRACE(particles::ToKernelParticleType(type));
    auto source = std::make_shared<sources::GGEMSSource>();
    source->SetEmittedParticleType(type).SetDirection(1, 0, 0).SetPrimaryCount(
      1ULL);
    auto observer = std::make_shared<observer::GGEMSTransportObserver>();
    observer->Enable().SetRecordCapacity(2U).CaptureFirstPrimaries(1U);
    GGEMSRun run{};
    run.SetRandom(std::make_shared<random::GGEMSRandom>());
    run.AddSource(source);
    run.SetObserver(observer);
    run.SetWorld(geometry::GGEMSWorld{
      200_mm, 200_mm, 200_mm,
      materials::builtins::BuildBuiltInMaterial("Vacuum")});
    ASSERT_NO_THROW(run.Initialize());
    ASSERT_NO_THROW(run.Run());
    auto const records = observer->GetRecords();
    ASSERT_EQ(records.size(), 2U);
    auto const &terminal = records.back();
    EXPECT_EQ(terminal.position_x_pm, 1'000'000'000'000LL);
    EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                                 particles::GGEMSParticleStatus::Killed));
    EXPECT_EQ(terminal.particle_type, particles::ToKernelParticleType(type));
  }
}

TEST_F(GGEMSWorldTransportTest, DisabledAioninoDoesNotRequireWorld) {
  auto disabled = std::make_shared<sources::GGEMSSource>();
  disabled->SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
    .SetPrimaryCount(0ULL);
  auto active = std::make_shared<sources::GGEMSSource>();
  active->SetEmittedParticleType(particles::GGEMSParticleType::Gamma)
    .SetPrimaryCount(1ULL);
  GGEMSRun run{};
  run.SetRandom(std::make_shared<random::GGEMSRandom>());
  run.AddSource(disabled);
  run.AddSource(active);
  ASSERT_NO_THROW(run.Initialize());
  ASSERT_NO_THROW(run.Run());
  EXPECT_TRUE(run.GetLastSourceRunSnapshot().has_value());
  disabled->SetPrimaryCount(1ULL);
  EXPECT_THROW(run.Run(), GGEMSRecoverable);
}
