#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"

namespace {

// =============================================================================
// =============================================================================

struct HostAccessRecord {
  std::uint64_t identifier{0ULL};
  std::int32_t value{0};
  float weight{0.0f};

  bool operator==(HostAccessRecord const &) const = default;
};

static_assert(std::is_trivially_copyable_v<HostAccessRecord>);
static_assert(std::is_nothrow_move_constructible_v<
              ggems::ocl::GGEMSOpenCLSVMBuffer>);

// =============================================================================
// =============================================================================

struct NonTransferValue {
  ~NonTransferValue() {}
};

// =============================================================================
// =============================================================================

struct NothrowHostAccessGenerator {
  [[nodiscard]] HostAccessRecord operator()(std::size_t) noexcept;
};

// =============================================================================
// =============================================================================

struct ThrowingHostAccessGenerator {
  [[nodiscard]] HostAccessRecord operator()(std::size_t);
};

// =============================================================================
// =============================================================================

struct IncompatibleHostAccessGenerator {
  [[nodiscard]] std::uint64_t operator()(std::size_t) noexcept;
};

// =============================================================================
// =============================================================================

template <typename T>
concept CanUseSVMObjectWriteOverload =
    requires(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer, T const &value) {
      ggems::ocl::WriteSVMFromHost<T>(buffer, value);
    };

// =============================================================================
// =============================================================================

template <typename T>
concept CanWriteSVMSpan = requires(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer,
                                   std::span<T const> values) {
  ggems::ocl::WriteSVMFromHost(buffer, values);
};

// =============================================================================
// =============================================================================

template <typename T>
concept CanReadSVMObject = requires(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer) {
  { ggems::ocl::ReadSVMToHost<T>(buffer) } -> std::same_as<T>;
};

// =============================================================================
// =============================================================================

template <typename T>
concept CanReadSVMSpan = requires(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer,
                                  std::span<T> destination) {
  ggems::ocl::ReadSVMToHost(buffer, destination);
};

// =============================================================================
// =============================================================================

template <typename T>
concept CanFillSVM =
    requires(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer, T const &value) {
      ggems::ocl::FillSVMFromHost(buffer, 1U, value);
    };

// =============================================================================
// =============================================================================

template <typename Generator>
concept CanGenerateHostAccessRecords =
    requires(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer, Generator &generator) {
      ggems::ocl::GenerateSVMFromHost<HostAccessRecord>(buffer, 1U, generator);
    };

// =============================================================================
// =============================================================================

using HostAccessRecordSpan = std::span<HostAccessRecord const>;

static_assert(ggems::ocl::SVMHostTransferValue<HostAccessRecord>);
static_assert(!ggems::ocl::SVMHostTransferValue<NonTransferValue>);

static_assert(CanUseSVMObjectWriteOverload<HostAccessRecord>);
static_assert(std::is_trivially_copyable_v<HostAccessRecordSpan>);
static_assert(!CanUseSVMObjectWriteOverload<HostAccessRecordSpan>);
static_assert(!CanUseSVMObjectWriteOverload<NonTransferValue>);

static_assert(CanWriteSVMSpan<HostAccessRecord>);
static_assert(!CanWriteSVMSpan<NonTransferValue>);

static_assert(CanReadSVMObject<HostAccessRecord>);
static_assert(!CanReadSVMObject<NonTransferValue>);

static_assert(CanReadSVMSpan<HostAccessRecord>);
static_assert(!CanReadSVMSpan<HostAccessRecord const>);
static_assert(!CanReadSVMSpan<NonTransferValue>);

static_assert(CanFillSVM<HostAccessRecord>);
static_assert(!CanFillSVM<NonTransferValue>);

static_assert(CanGenerateHostAccessRecords<NothrowHostAccessGenerator>);
static_assert(!CanGenerateHostAccessRecords<ThrowingHostAccessGenerator>);
static_assert(!CanGenerateHostAccessRecords<IncompatibleHostAccessGenerator>);

// =============================================================================
// =============================================================================

template <std::size_t Size>
[[nodiscard]] std::array<HostAccessRecord, Size>
ReadHostAccessRecords(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer) {
  std::array<HostAccessRecord, Size> records{};
  ggems::ocl::ReadSVMToHost(buffer, std::span{records});
  return records;
}

// =============================================================================
// =============================================================================

[[nodiscard]] HostAccessRecord
MakeGeneratedHostAccessRecord(std::size_t index) noexcept {
  return HostAccessRecord{
      .identifier = static_cast<std::uint64_t>(100U + index),
      .value = static_cast<std::int32_t>(index) - 2,
      .weight = static_cast<float>(index) * 0.25F,
  };
}

// =============================================================================
// =============================================================================

class GGEMSOpenCLSVMHostAccessTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
    ASSERT_TRUE(opencl.GetContext().front().GetSVMSupport().HasAny());
  }

  static ggems::ocl::GGEMSOpenCLContext &GetContext() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    return opencl.GetContext().front();
  }
};
} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSOpenCLSVMHostAccessTest, WritesAndReadsObjectExactly) {
  auto buffer = GetContext().CreateSVMBuffer(
      ggems::units::Bytes{sizeof(HostAccessRecord)});

  HostAccessRecord expected{
      .identifier = 0x0123456789ABCDEFULL,
      .value = -123'456,
      .weight = 0.375F,
  };

  ggems::ocl::WriteSVMFromHost(buffer, expected);

  auto actual = ggems::ocl::ReadSVMToHost<HostAccessRecord>(buffer);

  EXPECT_EQ(actual, expected);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSOpenCLSVMHostAccessTest,
       WritesAndReadsDynamicAndFixedExtentSpansExactly) {
  std::array<HostAccessRecord, 3U> expected{{
      {.identifier = 1ULL, .value = -1, .weight = 0.25F},
      {.identifier = 2ULL, .value = 0, .weight = 0.50F},
      {.identifier = 3ULL, .value = 1, .weight = 0.75F},
  }};

  auto buffer = GetContext().CreateSVMBuffer(
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

// =============================================================================
// =============================================================================

TEST_F(GGEMSOpenCLSVMHostAccessTest, FillsEveryArrayElementExactly) {
  constexpr std::size_t k_element_count{5U};

  HostAccessRecord expected{
      .identifier = 0xFEDCBA9876543210ULL,
      .value = -42,
      .weight = 1.25F,
  };

  auto buffer = GetContext().CreateSVMBuffer(
      ggems::units::Bytes{k_element_count * sizeof(HostAccessRecord)});

  ggems::ocl::FillSVMFromHost(buffer, k_element_count, expected);

  auto actual = ReadHostAccessRecords<k_element_count>(buffer);

  for (auto const &record : actual) {
    EXPECT_EQ(record, expected);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSOpenCLSVMHostAccessTest, GeneratesEveryElementFromItsIndex) {
  constexpr std::size_t k_element_count{5U};

  auto buffer = GetContext().CreateSVMBuffer(
      ggems::units::Bytes{k_element_count * sizeof(HostAccessRecord)});

  std::size_t invocation_count{0U};

  auto generator = [&invocation_count](std::size_t index) noexcept {
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

// =============================================================================
// =============================================================================

TEST_F(GGEMSOpenCLSVMHostAccessTest, ZeroLengthOperationsAreNoOps) {
  std::array<HostAccessRecord, 2U> sentinels{{
      {.identifier = 7ULL, .value = -7, .weight = 0.125F},
      {.identifier = 8ULL, .value = -8, .weight = 0.875F},
  }};

  auto buffer = GetContext().CreateSVMBuffer(
      ggems::units::Bytes{sentinels.size() * sizeof(HostAccessRecord)});

  ggems::ocl::WriteSVMFromHost(buffer, std::span{sentinels});

  std::span<HostAccessRecord> empty_source{};
  std::span<HostAccessRecord> empty_destination{};

  ggems::ocl::WriteSVMFromHost(buffer, empty_source);
  ggems::ocl::ReadSVMToHost(buffer, empty_destination);
  ggems::ocl::FillSVMFromHost(buffer, 0U, HostAccessRecord{});

  std::size_t invocation_count{0U};

  auto generator = [&invocation_count](std::size_t) noexcept {
    ++invocation_count;
    return HostAccessRecord{};
  };

  ggems::ocl::GenerateSVMFromHost<HostAccessRecord>(buffer, 0U, generator);

  EXPECT_EQ(invocation_count, 0U);
  EXPECT_EQ(ReadHostAccessRecords<2U>(buffer), sentinels);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSOpenCLSVMHostAccessTest,
       RejectsInsufficientCapacityBeforeAnyAccess) {
  constexpr std::size_t k_buffer_element_count{2U};

  auto &context = GetContext();
  auto allocated_before = context.GetAllocatedVRAM().value;
  auto allocation_count_before = context.GetAllocationCountVRAM();

  {
    auto buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{k_buffer_element_count * sizeof(HostAccessRecord)});

    EXPECT_EQ(context.GetAllocatedVRAM().value,
              allocated_before +
                  k_buffer_element_count * sizeof(HostAccessRecord));
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

    EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer), sentinels);

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
    EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer), sentinels);

    EXPECT_THROW(ggems::ocl::FillSVMFromHost(
                     buffer, k_buffer_element_count + 1U, HostAccessRecord{}),
                 ggems::core::GGEMSExceptionBase);

    EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer), sentinels);

    std::size_t invocation_count{0U};

    auto generator = [&invocation_count](std::size_t) noexcept {
      ++invocation_count;
      return HostAccessRecord{};
    };

    EXPECT_THROW(ggems::ocl::GenerateSVMFromHost<HostAccessRecord>(
                     buffer, k_buffer_element_count + 1U, generator),
                 ggems::core::GGEMSExceptionBase);

    EXPECT_EQ(invocation_count, 0U);
    EXPECT_EQ(ReadHostAccessRecords<k_buffer_element_count>(buffer), sentinels);

    EXPECT_EQ(context.GetAllocatedVRAM().value,
              allocated_before +
                  k_buffer_element_count * sizeof(HostAccessRecord));
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before + 1U);
  }

  EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_before);
  EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
}
