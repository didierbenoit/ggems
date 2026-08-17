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
 * \brief Unit tests for GGEMS OpenCL SVM buffers.
 *
 * Validates automatic allocation, ownership transfer through move operations, allocation accounting, and rejection of invalid or unsupported requests.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstddef>
#include <utility>
#include <limits>
#include <cstdint>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMMemoryKind.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

/// \cond

using namespace ggems::units;

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMBufferTest, AutoAllocationTracksOwnershipAndAccounting) {
  std::size_t compatible_device_count{0U};

  for (auto const &inventory : ggems::test::GetOpenCLDeviceInventory()) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(inventory));

    auto const &device = inventory.device.get();
    if (device.GetAvailable() == CL_FALSE) {
      continue;
    }

    ggems::ocl::GGEMSOpenCLContext context{device};
    auto const &support = context.GetSVMSupport();
    if (!support.HasAny()) {
      continue;
    }
    ++compatible_device_count;

    constexpr auto k_buffer_size{64_B};
    auto const allocated_before = context.GetAllocatedVRAM();
    auto const allocation_count_before = context.GetAllocationCountVRAM();

    {
      auto buffer = context.CreateSVMBuffer(k_buffer_size,
                                            ggems::ocl::SVMMemoryKind::Auto);

      EXPECT_NE(buffer.GetData(), nullptr);
      EXPECT_EQ(buffer.GetSize(), k_buffer_size);
      EXPECT_NE(buffer.GetKind(), ggems::ocl::SVMMemoryKind::Auto);
      EXPECT_NE(buffer.GetKind(), ggems::ocl::SVMMemoryKind::None);
      EXPECT_TRUE(support.Supports(buffer.GetKind()));
      EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before + k_buffer_size);
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before + 1U);
    }

    EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMBufferTest,
     MoveConstructionTransfersOwnershipWithoutChangingAccounting) {
  std::size_t compatible_device_count{0U};

  for (auto const &inventory : ggems::test::GetOpenCLDeviceInventory()) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(inventory));

    auto const &device = inventory.device.get();
    if (device.GetAvailable() == CL_FALSE) {
      continue;
    }

    ggems::ocl::GGEMSOpenCLContext context{device};
    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }
    ++compatible_device_count;

    constexpr auto k_buffer_size{64_B};
    auto const allocated_before = context.GetAllocatedVRAM();
    auto const allocation_count_before = context.GetAllocationCountVRAM();

    {
      auto source = context.CreateSVMBuffer(k_buffer_size);
      auto *const source_data = source.GetData();
      auto const source_size = source.GetSize();
      auto const source_flags = source.GetFlags();
      auto const source_kind = source.GetKind();
      auto const allocated_with_buffer = context.GetAllocatedVRAM();
      auto const allocation_count_with_buffer =
          context.GetAllocationCountVRAM();

      ggems::ocl::GGEMSOpenCLSVMBuffer destination{std::move(source)};

      EXPECT_EQ(context.GetAllocatedVRAM(), allocated_with_buffer);
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_with_buffer);
      EXPECT_EQ(destination.GetData(), source_data);
      EXPECT_EQ(destination.GetSize(), source_size);
      EXPECT_EQ(destination.GetFlags(), source_flags);
      EXPECT_EQ(destination.GetKind(), source_kind);
      EXPECT_EQ(source.GetData(), nullptr);
      EXPECT_EQ(source.GetSize(), 0_B);
      EXPECT_EQ(source.GetFlags(), 0U);
      EXPECT_EQ(source.GetKind(), ggems::ocl::SVMMemoryKind::None);
    }

    EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMBufferTest,
     MoveAssignmentReleasesPreviousAllocationAndTransfersOwnership) {
  std::size_t compatible_device_count{0U};

  for (auto const &inventory : ggems::test::GetOpenCLDeviceInventory()) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(inventory));

    auto const &device = inventory.device.get();
    if (device.GetAvailable() == CL_FALSE) {
      continue;
    }

    ggems::ocl::GGEMSOpenCLContext context{device};
    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }
    ++compatible_device_count;

    constexpr auto k_source_size{64_B};
    constexpr auto k_destination_size{128_B};
    auto const allocated_before = context.GetAllocatedVRAM();
    auto const allocation_count_before = context.GetAllocationCountVRAM();

    {
      auto source = context.CreateSVMBuffer(k_source_size);
      auto destination = context.CreateSVMBuffer(k_destination_size);

      EXPECT_EQ(context.GetAllocatedVRAM(),
                allocated_before + k_source_size + k_destination_size);
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before + 2U);

      auto *const source_data = source.GetData();
      auto const source_size = source.GetSize();
      auto const source_flags = source.GetFlags();
      auto const source_kind = source.GetKind();

      destination = std::move(source);

      EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before + k_source_size);
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before + 1U);
      EXPECT_EQ(destination.GetData(), source_data);
      EXPECT_EQ(destination.GetSize(), source_size);
      EXPECT_EQ(destination.GetFlags(), source_flags);
      EXPECT_EQ(destination.GetKind(), source_kind);
      EXPECT_EQ(source.GetData(), nullptr);
      EXPECT_EQ(source.GetSize(), 0_B);
      EXPECT_EQ(source.GetFlags(), 0U);
      EXPECT_EQ(source.GetKind(), ggems::ocl::SVMMemoryKind::None);
    }

    EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMBufferTest,
     RejectsInvalidAndUnsupportedAllocationsWithoutChangingAccounting) {
  constexpr std::array k_concrete_kinds{
      ggems::ocl::SVMMemoryKind::CoarseGrainBuffer,
      ggems::ocl::SVMMemoryKind::FineGrainBuffer,
      ggems::ocl::SVMMemoryKind::FineGrainBufferAtomics,
      ggems::ocl::SVMMemoryKind::FineGrainSystem,
  };

  std::size_t compatible_device_count{0U};

  for (auto const &inventory : ggems::test::GetOpenCLDeviceInventory()) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(inventory));

    auto const &device = inventory.device.get();
    if (device.GetAvailable() == CL_FALSE) {
      continue;
    }

    ggems::ocl::GGEMSOpenCLContext context{device};
    auto const &support = context.GetSVMSupport();
    if (!support.HasAny()) {
      continue;
    }
    ++compatible_device_count;

    auto const allocated_before = context.GetAllocatedVRAM();
    auto const allocation_count_before = context.GetAllocationCountVRAM();

    EXPECT_THROW((void)context.CreateSVMBuffer(0_B), ggems::core::GGEMSFatal);
    EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);

    EXPECT_THROW(
        (void)context.CreateSVMBuffer(64_B, ggems::ocl::SVMMemoryKind::None),
        ggems::core::GGEMSFatal);
    EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);

    constexpr auto k_oversized_alignment = ggems::units::Bytes{
        static_cast<std::uint64_t>(std::numeric_limits<cl_uint>::max()) + 1ULL};

    EXPECT_THROW((void)context.CreateSVMBuffer(64_B,
                                               ggems::ocl::SVMMemoryKind::Auto,
                                               k_oversized_alignment),
                 ggems::core::GGEMSFatal);
    EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);

    for (auto const kind : k_concrete_kinds) {
      if (support.Supports(kind)) {
        continue;
      }

      EXPECT_THROW((void)context.CreateSVMBuffer(64_B, kind),
                   ggems::core::GGEMSFatal);
      EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
    }
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}
/// \endcond
