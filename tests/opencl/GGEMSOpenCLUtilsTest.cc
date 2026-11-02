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
 * \brief Checks OpenCL query and error-reporting boundaries.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <string>
#include <string_view>
#include <unordered_set>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace {

auto ExpectContains(std::string_view value, std::string_view expected) -> void {
  EXPECT_NE(value.find(expected), std::string_view::npos);
}

} // namespace

// =============================================================================
// =============================================================================

// Known and unknown OpenCL status codes retain useful diagnostic names and
// descriptions.
TEST(GGEMSOpenCLUtilsTest, ReportsRepresentativeErrorCodes) {
  struct ErrorNameCase {
    cl_int error_code;
    std::string_view expected_name;
  };

  constexpr std::array test_cases{
    ErrorNameCase{.error_code = CL_SUCCESS, .expected_name = "CL_SUCCESS"},
    ErrorNameCase{
      .error_code = CL_INVALID_VALUE,
      .expected_name = "CL_INVALID_VALUE",
    },
    ErrorNameCase{
      .error_code = CL_INVALID_EVENT,
      .expected_name = "CL_INVALID_EVENT",
    },
    ErrorNameCase{.error_code = 123'456, .expected_name = "CL_UNKNOWN_ERROR"},
  };

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.expected_name);
    EXPECT_EQ(ggems::ocl::GetErrorCodeName(test_case.error_code),
              test_case.expected_name);
  }

  EXPECT_EQ(ggems::ocl::GetErrorDescription(CL_INVALID_EVENT),
            "Invalid event object.");
  EXPECT_EQ(ggems::ocl::GetErrorDescription(123'456), "Unknown OpenCL error.");
}

// =============================================================================
// =============================================================================

// Combined error diagnostics include both the OpenCL code name and its meaning.
TEST(GGEMSOpenCLUtilsTest, BuildsLongErrorDescription) {
  auto const description = ggems::ocl::GetLongErrorString(CL_INVALID_EVENT);
  ExpectContains(description, "CL_INVALID_EVENT");
  ExpectContains(description, "Invalid event object.");
}

// =============================================================================
// =============================================================================

// OpenCL failures retain their typed GGEMS exception and caller-provided
// context.
TEST(GGEMSOpenCLUtilsTest,
     CheckCLErrorAcceptsSuccessAndPreservesFailureContext) {
  EXPECT_NO_THROW(ggems::ocl::CheckCLError(CL_SUCCESS, "OpenCL success"));

  EXPECT_THROW(
    ggems::ocl::CheckCLError(CL_INVALID_VALUE, "OpenCL default failure"),
    ggems::core::GGEMSFatal);

  try {
    ggems::ocl::CheckCLError<ggems::core::GGEMSRecoverable>(CL_INVALID_VALUE,
                                                            "OpenCL probe");
    FAIL() << "Expected GGEMSRecoverable.";
  } catch (ggems::core::GGEMSRecoverable const &exception) {
    auto const diagnostic = std::string_view{exception.what()};
    ExpectContains(diagnostic, "OpenCL probe");
    ExpectContains(diagnostic, "code -30");
    ExpectContains(diagnostic, "CL_INVALID_VALUE");
  }
}

// =============================================================================
// =============================================================================

// Capability lookup rejects missing names, case changes, and empty extension
// strings.
TEST(GGEMSOpenCLUtilsTest, FindsOnlyExactExtensions) {
  std::unordered_set<std::string> const extensions{
    "cl_khr_fp64",
    "cl_khr_device_uuid",
  };

  EXPECT_TRUE(ggems::ocl::HasExtension(extensions, "cl_khr_fp64"));
  EXPECT_FALSE(ggems::ocl::HasExtension(extensions, "cl_khr_fp16"));
  EXPECT_FALSE(ggems::ocl::HasExtension(extensions, "CL_KHR_FP64"));
  EXPECT_FALSE(ggems::ocl::HasExtension(extensions, ""));
}

// =============================================================================
// =============================================================================

// Native scalar, array, and extension queries preserve their driver-reported
// values.
TEST(GGEMSOpenCLUtilsTest, ReadsRepresentativeNativeInformation) {
  auto const &entry = ggems::test::GetSelectedOpenCLDevice();
  SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

  auto const &device = entry.device.get();
  auto const &native = device.GetDeviceNative();

  auto const name = ggems::ocl::GetInfo<CL_DEVICE_NAME>(native);
  EXPECT_EQ(name, device.GetName());
  EXPECT_EQ(name.find('\0'), std::string::npos);

  EXPECT_EQ(ggems::ocl::GetInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(native),
            device.GetMaxWorkItemSizes());
  EXPECT_EQ(ggems::ocl::ExtractExtensions<CL_DEVICE_EXTENSIONS>(native),
            device.GetDeviceExtensions());
}

// =============================================================================
// =============================================================================

// The fallback string query removes the terminal NUL from advertised SPIR
// versions.
TEST(GGEMSOpenCLUtilsTest,
     ReadsFallbackStringWithoutTerminalNullWhenSpirIsAdvertised) {
  auto const &entry = ggems::test::GetSelectedOpenCLDevice();
  SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

  auto const &device = entry.device.get();
  if (!ggems::ocl::HasExtension(device.GetDeviceExtensions(), "cl_khr_spir")) {
    GTEST_SKIP() << "The selected device does not advertise cl_khr_spir.";
  }

  auto const versions =
    ggems::ocl::GetInfo<CL_DEVICE_SPIR_VERSIONS>(device.GetDeviceNative());
  EXPECT_EQ(versions.find('\0'), std::string::npos);
}
