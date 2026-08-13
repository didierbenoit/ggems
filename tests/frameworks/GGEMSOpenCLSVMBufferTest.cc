#include <array>
#include <cstddef>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMMemoryKind.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

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
