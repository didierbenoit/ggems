#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"

namespace {

// =============================================================================
// =============================================================================

using EnergyRecord = ggems::core::sources::GGEMSEnergyDistributionRecord;

// =============================================================================
// =============================================================================

struct AlignmentProbe {
  std::uint8_t prefix;
  EnergyRecord record;
};

// =============================================================================
// =============================================================================

class GGEMSEnergyDistributionRecordKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyDistributionRecordKernelTest,
       HostAndKernelLayoutStrideAndValuesMatch) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = opencl.GetContext().front();

  std::array<EnergyRecord, 2U> records{{
      {
          .regular_bin_width_micro_eV = 11ULL,
          .table_offset = 12ULL,
          .distribution_type = 13U,
          .table_count = 14U,
      },
      {},
  }};

  std::array<std::uint64_t, 7U> layout{};
  std::array<std::uint64_t, 4U> host_values{};

  auto layout_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{layout.size() * sizeof(std::uint64_t)});
  auto records_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{records.size() * sizeof(EnergyRecord)});
  auto values_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{host_values.size() * sizeof(std::uint64_t)});

  ggems::ocl::WriteSVMFromHost(layout_buffer, std::span{layout});
  ggems::ocl::WriteSVMFromHost(records_buffer, std::span{records});
  ggems::ocl::WriteSVMFromHost(values_buffer, std::span{host_values});

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";
  std::string const build_options =
      std::format("-I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "energy_distribution_record_abi_probe",
      build_options);
  cl::Kernel raw_kernel =
      program.CreateKernel("energy_distribution_record_abi_probe");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "energy_distribution_record_abi_probe"};

  kernel.SetArgSVMPointer(0U, layout_buffer.GetData());
  kernel.SetArgSVMPointer(1U, records_buffer.GetData());
  kernel.SetArgSVMPointer(2U, values_buffer.GetData());
  kernel.Run({1U}, {1U});

  ggems::ocl::ReadSVMToHost(layout_buffer, std::span{layout});
  ggems::ocl::ReadSVMToHost(records_buffer, std::span{records});
  ggems::ocl::ReadSVMToHost(values_buffer, std::span{host_values});

  std::array<std::uint64_t, 7U> const expected_layout{{
      sizeof(EnergyRecord),
      offsetof(EnergyRecord, regular_bin_width_micro_eV),
      offsetof(EnergyRecord, table_offset),
      offsetof(EnergyRecord, distribution_type),
      offsetof(EnergyRecord, table_count),
      sizeof(EnergyRecord),
      offsetof(AlignmentProbe, record),
  }};

  EXPECT_EQ(layout, expected_layout);
  EXPECT_EQ(host_values[0U], 11ULL);
  EXPECT_EQ(host_values[1U], 12ULL);
  EXPECT_EQ(host_values[2U], 13ULL);
  EXPECT_EQ(host_values[3U], 14ULL);

  EXPECT_EQ(records[1U].regular_bin_width_micro_eV, 101ULL);
  EXPECT_EQ(records[1U].table_offset, 102ULL);
  EXPECT_EQ(records[1U].distribution_type, 3U);
  EXPECT_EQ(records[1U].table_count, 103U);
}
