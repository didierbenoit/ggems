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
 * \brief Unit tests for OpenCL string conversion helpers.
 *
 * Validates formatting of versions, structured name-version values, scalars, flags, UUID/LUID byte arrays, and representative kernel argument qualifiers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLStrings.hh"

/// \cond

namespace {

auto SetName(cl_name_version &name_version, std::string_view name) -> void {
  std::ranges::copy(name, name_version.name);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLStringsTest, FormatsVersionsAndStructuredNameVersions) {
  EXPECT_EQ(ggems::ocl::ClVersionToString(CL_MAKE_VERSION(3, 1, 7)), "3.1.7");

  cl_name_version name_version{};
  name_version.version = CL_MAKE_VERSION(2, 0, 4);
  SetName(name_version, "opencl_feature");

  auto const formatted = ggems::ocl::ClNameVersionToString({name_version});
  EXPECT_NE(formatted.find("opencl_feature"), std::string::npos);
  EXPECT_NE(formatted.find("2.0.4"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLStringsTest, FormatsRepresentativeScalarAndFallbackValues) {
  EXPECT_EQ(ggems::ocl::ClBoolToString(CL_TRUE), "Yes");
  EXPECT_EQ(ggems::ocl::ClBoolToString(CL_FALSE), "No");
  EXPECT_EQ(ggems::ocl::UIntToString(42U), "42");
  EXPECT_EQ(ggems::ocl::UIntToString(std::numeric_limits<cl_uint>::max()),
            "N/A");

  auto const unknown_vendor = ggems::ocl::VendorIdToString(0x1234U);
  EXPECT_NE(unknown_vendor.find("Unknown"), std::string::npos);
  EXPECT_NE(unknown_vendor.find("1234"), std::string::npos);

  EXPECT_EQ(ggems::ocl::CacheTypeToString(CL_NONE), "None");
  EXPECT_NE(ggems::ocl::CacheTypeToString(
                static_cast<cl_device_mem_cache_type>(0x7fffU))
                .find("Unknown"),
            std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLStringsTest, FormatsZeroSingleAndCombinedFlags) {
  EXPECT_EQ(ggems::ocl::DeviceTypeToString(CL_DEVICE_TYPE_CPU), "CPU");
  EXPECT_EQ(
      ggems::ocl::DeviceTypeToString(CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU),
      "CPU | GPU");
  EXPECT_NE(ggems::ocl::DeviceTypeToString(0).find("Unknown"),
            std::string::npos);

  EXPECT_EQ(ggems::ocl::QueuePropertiesToString(0), "None");
  EXPECT_EQ(ggems::ocl::QueuePropertiesToString(CL_QUEUE_PROFILING_ENABLE),
            "Profiling enabled");
  EXPECT_EQ(
      ggems::ocl::QueuePropertiesToString(
          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE),
      "Out-of-order execution, Profiling enabled");

  EXPECT_EQ(ggems::ocl::SVMCapabilitiesToString(0), "None");
  EXPECT_EQ(
      ggems::ocl::SVMCapabilitiesToString(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER |
                                          CL_DEVICE_SVM_FINE_GRAIN_BUFFER),
      "Coarse-grain buffer, Fine-grain buffer");

  EXPECT_EQ(ggems::ocl::FPConfigToString(0), "None");
  EXPECT_EQ(ggems::ocl::FPConfigToString(CL_FP_DENORM | CL_FP_INF_NAN),
            "Denormals, Inf/NaN");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLStringsTest, FormatsUuidAndLuidBytes) {
  std::array<cl_uchar, CL_UUID_SIZE_KHR> uuid{};
  for (std::size_t index = 0; index < uuid.size(); ++index) {
    uuid[index] = static_cast<cl_uchar>(index);
  }
  EXPECT_EQ(ggems::ocl::UUIDToString(uuid),
            "00010203-0405-0607-0809-0a0b0c0d0e0f");

  std::array<cl_uchar, CL_LUID_SIZE_KHR> luid{};
  for (std::size_t index = 0; index < luid.size(); ++index) {
    luid[index] = static_cast<cl_uchar>(index);
  }
  EXPECT_EQ(ggems::ocl::LUIDToString(luid), "0001020304050607");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLStringsTest, FormatsRepresentativeKernelArgumentQualifiers) {
  EXPECT_EQ(
      ggems::ocl::ArgAddressQualifierToString(CL_KERNEL_ARG_ADDRESS_GLOBAL),
      "CL_KERNEL_ARG_ADDRESS_GLOBAL");
  EXPECT_EQ(
      ggems::ocl::ArgAddressQualifierToString(CL_KERNEL_ARG_ADDRESS_PRIVATE),
      "CL_KERNEL_ARG_ADDRESS_PRIVATE");
  EXPECT_EQ(
      ggems::ocl::ArgAccessQualifierToString(CL_KERNEL_ARG_ACCESS_READ_ONLY),
      "CL_KERNEL_ARG_ACCESS_READ_ONLY");
  EXPECT_EQ(ggems::ocl::ArgAccessQualifierToString(CL_KERNEL_ARG_ACCESS_NONE),
            "CL_KERNEL_ARG_ACCESS_NONE");
  EXPECT_EQ(ggems::ocl::ArgTypeQualifierToString(CL_KERNEL_ARG_TYPE_CONST |
                                                 CL_KERNEL_ARG_TYPE_RESTRICT),
            "CL_KERNEL_ARG_TYPE_CONST CL_KERNEL_ARG_TYPE_RESTRICT");
  EXPECT_EQ(ggems::ocl::ArgTypeQualifierToString(CL_KERNEL_ARG_TYPE_NONE),
            "None");
}
/// \endcond
