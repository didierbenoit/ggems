#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <string>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace {

// =============================================================================
// =============================================================================

struct HostAccessRecord {
  std::uint64_t identifier{0ULL};
  std::int32_t value{0};
  float weight{0.0F};

  auto operator==(HostAccessRecord const &) const -> bool = default;
};

// =============================================================================
// =============================================================================

struct NonTransferValue {
  std::string value;
};

// =============================================================================
// =============================================================================

static_assert(std::is_trivially_copyable_v<HostAccessRecord>);
static_assert(
    std::is_nothrow_move_constructible_v<ggems::ocl::GGEMSOpenCLSVMBuffer>);
static_assert(ggems::ocl::SVMHostTransferValue<HostAccessRecord>);
static_assert(!ggems::ocl::SVMHostTransferValue<NonTransferValue>);

// =============================================================================
// =============================================================================

template <std::size_t Size>
[[nodiscard]] auto
ReadHostAccessRecords(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer)
    -> std::array<HostAccessRecord, Size> {
  std::array<HostAccessRecord, Size> records{};
  ggems::ocl::ReadSVMToHost(buffer, std::span{records});
  return records;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeGeneratedHostAccessRecord(std::size_t index) noexcept
    -> HostAccessRecord {
  return HostAccessRecord{
      .identifier = static_cast<std::uint64_t>(100U + index),
      .value = static_cast<std::int32_t>(index) - 2,
      .weight = static_cast<float>(index) * 0.25F,
  };
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMHostAccessTest, WritesAndReadsObjectExactly) {
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

    auto buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(HostAccessRecord)});

    HostAccessRecord expected{
        .identifier = 0x0123456789ABCDEFULL,
        .value = -123'456,
        .weight = 0.375F,
    };

    ggems::ocl::WriteSVMFromHost(buffer, expected);

    auto actual = ggems::ocl::ReadSVMToHost<HostAccessRecord>(buffer);

    EXPECT_EQ(actual, expected);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMHostAccessTest,
     WritesAndReadsDynamicAndFixedExtentSpansExactly) {
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

    std::array<HostAccessRecord, 3U> expected{{
        {.identifier = 1ULL, .value = -1, .weight = 0.25F},
        {.identifier = 2ULL, .value = 0, .weight = 0.50F},
        {.identifier = 3ULL, .value = 1, .weight = 0.75F},
    }};

    auto buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{3U * sizeof(HostAccessRecord)});

    std::span<HostAccessRecord> dynamic_source{expected};
    ggems::ocl::WriteSVMFromHost(buffer, dynamic_source);

    std::array<HostAccessRecord, 3U> actual{};

    std::span<HostAccessRecord, 3U> fixed_destination{actual};
    ggems::ocl::ReadSVMToHost(buffer, fixed_destination);

    EXPECT_EQ(actual, expected);

    std::array<HostAccessRecord, 3U> replacement{{
        {.identifier = 4ULL, .value = -4, .weight = 1.25F},
        {.identifier = 5ULL, .value = 5, .weight = 1.50F},
        {.identifier = 6ULL, .value = 6, .weight = 1.75F},
    }};

    std::span<HostAccessRecord, 3U> fixed_source{replacement};
    ggems::ocl::WriteSVMFromHost(buffer, fixed_source);

    std::span<HostAccessRecord> dynamic_destination{actual};
    ggems::ocl::ReadSVMToHost(buffer, dynamic_destination);

    EXPECT_EQ(actual, replacement);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMHostAccessTest, FillsEveryArrayElementExactly) {
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

    constexpr std::size_t k_element_count{5U};

    HostAccessRecord expected{
        .identifier = 0xFEDCBA9876543210ULL,
        .value = -42,
        .weight = 1.25F,
    };

    auto buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{k_element_count * sizeof(HostAccessRecord)});

    ggems::ocl::FillSVMFromHost(buffer, k_element_count, expected);

    auto actual = ReadHostAccessRecords<k_element_count>(buffer);

    for (auto const &record : actual) {
      EXPECT_EQ(record, expected);
    }
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMHostAccessTest, GeneratesEveryElementFromItsIndex) {
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

    constexpr std::size_t k_element_count{5U};

    auto buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{k_element_count * sizeof(HostAccessRecord)});

    std::size_t invocation_count{0U};

    auto generator =
        [&invocation_count](std::size_t index) noexcept -> HostAccessRecord {
      ++invocation_count;
      return MakeGeneratedHostAccessRecord(index);
    };

    ggems::ocl::GenerateSVMFromHost<HostAccessRecord>(buffer, k_element_count,
                                                      generator);

    EXPECT_EQ(invocation_count, k_element_count);

    auto actual = ReadHostAccessRecords<k_element_count>(buffer);

    for (std::size_t index = 0U; index < k_element_count; ++index) {
      EXPECT_EQ(actual[index], MakeGeneratedHostAccessRecord(index));
    }
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMHostAccessTest, ZeroLengthOperationsAreNoOps) {
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

    std::array<HostAccessRecord, 2U> sentinels{{
        {.identifier = 7ULL, .value = -7, .weight = 0.125F},
        {.identifier = 8ULL, .value = -8, .weight = 0.875F},
    }};

    auto buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{sentinels.size() * sizeof(HostAccessRecord)});

    ggems::ocl::WriteSVMFromHost(buffer, std::span{sentinels});

    std::span<HostAccessRecord> empty_source{};
    std::span<HostAccessRecord> empty_destination{};

    ggems::ocl::WriteSVMFromHost(buffer, empty_source);
    ggems::ocl::ReadSVMToHost(buffer, empty_destination);
    ggems::ocl::FillSVMFromHost(buffer, 0U, HostAccessRecord{});

    std::size_t invocation_count{0U};

    auto generator =
        [&invocation_count](std::size_t) noexcept -> HostAccessRecord {
      ++invocation_count;
      return HostAccessRecord{};
    };

    ggems::ocl::GenerateSVMFromHost<HostAccessRecord>(buffer, 0U, generator);

    EXPECT_EQ(invocation_count, 0U);
    EXPECT_EQ(ReadHostAccessRecords<2U>(buffer), sentinels);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMHostAccessTest, RejectsInsufficientCapacityBeforeAnyAccess) {
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

    constexpr std::size_t k_buffer_element_count{2U};

    auto allocated_before = context.GetAllocatedVRAM().value;
    auto allocation_count_before = context.GetAllocationCountVRAM();

    {
      auto buffer = context.CreateSVMBuffer(ggems::units::Bytes{
          k_buffer_element_count * sizeof(HostAccessRecord)});

      EXPECT_EQ(context.GetAllocatedVRAM().value,
                allocated_before +
                    (k_buffer_element_count * sizeof(HostAccessRecord)));
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before + 1U);

      std::array<HostAccessRecord, k_buffer_element_count> sentinels{{
          {.identifier = 7ULL, .value = -7, .weight = 0.125F},
          {.identifier = 8ULL, .value = -8, .weight = 0.875F},
      }};

      ggems::ocl::WriteSVMFromHost(buffer, std::span{sentinels});

      std::array<HostAccessRecord, k_buffer_element_count + 1U>
          oversized_values{};

      EXPECT_THROW(
          ggems::ocl::WriteSVMFromHost(buffer, std::span{oversized_values}),
          ggems::core::GGEMSExceptionBase);

      EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer),
                sentinels);

      std::array<HostAccessRecord, k_buffer_element_count + 1U>
          oversized_destination{{
              {.identifier = 11ULL, .value = 11, .weight = 0.11F},
              {.identifier = 12ULL, .value = 12, .weight = 0.12F},
              {.identifier = 13ULL, .value = 13, .weight = 0.13F},
          }};
      auto destination_before = oversized_destination;

      EXPECT_THROW(
          ggems::ocl::ReadSVMToHost(buffer, std::span{oversized_destination}),
          ggems::core::GGEMSExceptionBase);

      EXPECT_EQ(oversized_destination, destination_before);
      EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer),
                sentinels);

      EXPECT_THROW(ggems::ocl::FillSVMFromHost(
                       buffer, k_buffer_element_count + 1U, HostAccessRecord{}),
                   ggems::core::GGEMSExceptionBase);

      EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer),
                sentinels);

      std::size_t invocation_count{0U};

      auto generator =
          [&invocation_count](std::size_t) noexcept -> HostAccessRecord {
        ++invocation_count;
        return HostAccessRecord{};
      };

      EXPECT_THROW(ggems::ocl::GenerateSVMFromHost<HostAccessRecord>(
                       buffer, k_buffer_element_count + 1U, generator),
                   ggems::core::GGEMSExceptionBase);

      EXPECT_EQ(invocation_count, 0U);
      EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer),
                sentinels);

      EXPECT_EQ(context.GetAllocatedVRAM().value,
                allocated_before +
                    (k_buffer_element_count * sizeof(HostAccessRecord)));
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before + 1U);
    }

    EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLSVMHostAccessTest, RejectsElementCountOverflowBeforeAnyAccess) {
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

    auto const allocated_before = context.GetAllocatedVRAM().value;
    auto const allocation_count_before = context.GetAllocationCountVRAM();

    {
      auto buffer = context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(HostAccessRecord)});

      HostAccessRecord sentinel{
          .identifier = 0x123456789ABCDEF0ULL,
          .value = -17,
          .weight = 0.625F,
      };
      ggems::ocl::WriteSVMFromHost(buffer, sentinel);

      auto const allocated_with_buffer = context.GetAllocatedVRAM().value;
      auto const allocation_count_with_buffer =
          context.GetAllocationCountVRAM();
      std::size_t invocation_count{0U};

      auto generator =
          [&invocation_count](std::size_t) noexcept -> HostAccessRecord {
        ++invocation_count;
        return HostAccessRecord{};
      };

      EXPECT_THROW(
          ggems::ocl::GenerateSVMFromHost<HostAccessRecord>(
              buffer, std::numeric_limits<std::size_t>::max(), generator),
          ggems::core::GGEMSInternal);

      EXPECT_EQ(invocation_count, 0U);
      EXPECT_EQ(ggems::ocl::ReadSVMToHost<HostAccessRecord>(buffer), sentinel);
      EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_with_buffer);
      EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_with_buffer);
    }

    EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_before);
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No available GGEMS-discovered device supports SVM.";
  }
}
