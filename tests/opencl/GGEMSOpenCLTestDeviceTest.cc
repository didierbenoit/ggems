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
 * \brief Checks single-device launch syntax without OpenCL discovery.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

// Device type indices and overall indices remain distinct before OpenCL
// discovery.
TEST(GGEMSOpenCLTestDeviceTest, ParsesSingleDeviceRequestsWithoutOpenCL) {
  struct Case {
    std::string_view selector;
    cl_device_type type;
    std::size_t index;
  };
  constexpr std::array cases{
    Case{.selector = "cpu", .type = CL_DEVICE_TYPE_CPU, .index = 0U},
    Case{.selector = "gpu", .type = CL_DEVICE_TYPE_GPU, .index = 0U},
    Case{.selector = "cpu:2", .type = CL_DEVICE_TYPE_CPU, .index = 2U},
    Case{.selector = "gpu:1", .type = CL_DEVICE_TYPE_GPU, .index = 1U},
    Case{.selector = "0", .type = 0, .index = 0U},
    Case{.selector = "12", .type = 0, .index = 12U},
  };

  for (auto const &entry : cases) {
    SCOPED_TRACE(entry.selector);
    auto const request = ggems::test::ParseOpenCLDeviceRequest(entry.selector);

    EXPECT_EQ(request.type, entry.type);
    EXPECT_EQ(request.index, entry.index);
  }
}

// Malformed, negative, overflowing, and multiple-device requests fail
// explicitly.
TEST(GGEMSOpenCLTestDeviceTest, RejectsAmbiguousOrMalformedRequests) {
  constexpr std::array selectors{
    "",       "all",     "igpu", "GPU", "cpu:",
    "gpu:-1", "gpu:1:2", "-1",   "+1",  "0;1",
    "0-1",    "1x",      " 1",   "1 ",  "18446744073709551616",
  };

  for (auto const *selector : selectors) {
    SCOPED_TRACE(selector);
    EXPECT_THROW((void)ggems::test::ParseOpenCLDeviceRequest(selector),
                 std::invalid_argument);
  }
}
