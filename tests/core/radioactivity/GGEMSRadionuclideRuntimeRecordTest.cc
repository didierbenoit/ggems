#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmissionRecord.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideGroupRange.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"

namespace {

using EmissionRecord =
    ggems::core::radioactivity::GGEMSRadionuclideEmissionRecord;
using GroupRange = ggems::core::radioactivity::GGEMSRadionuclideGroupRange;
using PopulationRecord = ggems::core::sources::GGEMSSourcePopulationRecord;

struct PopulationAlignmentProbe {
  std::uint8_t prefix;
  PopulationRecord record;
};

struct EmissionAlignmentProbe {
  std::uint8_t prefix;
  EmissionRecord record;
};

struct GroupAlignmentProbe {
  std::uint8_t prefix;
  GroupRange range;
};

class GGEMSRadionuclideRuntimeRecordKernelTest : public ::testing::Test {
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

TEST(GGEMSRadionuclideRuntimeRecord, HostLayoutsAndSentinelsAreExact) {
  EXPECT_TRUE(std::is_standard_layout_v<PopulationRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<PopulationRecord>);
  EXPECT_EQ(sizeof(PopulationRecord), 16U);
  EXPECT_EQ(alignof(PopulationRecord), 4U);
  EXPECT_EQ(offsetof(PopulationRecord, population_mode), 0U);
  EXPECT_EQ(offsetof(PopulationRecord, first_emission_index), 4U);
  EXPECT_EQ(offsetof(PopulationRecord, emission_count), 8U);
  EXPECT_EQ(offsetof(PopulationRecord, scaled_decay), 12U);
  EXPECT_EQ(offsetof(PopulationAlignmentProbe, record), 4U);

  EXPECT_TRUE(std::is_standard_layout_v<EmissionRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<EmissionRecord>);
  EXPECT_EQ(sizeof(EmissionRecord), 16U);
  EXPECT_EQ(alignof(EmissionRecord), 8U);
  EXPECT_EQ(offsetof(EmissionRecord, particle_type), 0U);
  EXPECT_EQ(offsetof(EmissionRecord, energy_distribution_record_index), 4U);
  EXPECT_EQ(offsetof(EmissionRecord, mono_energy_milli_eV), 8U);
  EXPECT_EQ(offsetof(EmissionAlignmentProbe, record), 8U);

  EXPECT_TRUE(std::is_standard_layout_v<GroupRange>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GroupRange>);
  EXPECT_EQ(sizeof(GroupRange), 16U);
  EXPECT_EQ(alignof(GroupRange), 8U);
  EXPECT_EQ(offsetof(GroupRange, source_local_primary_begin), 0U);
  EXPECT_EQ(offsetof(GroupRange, primary_count), 8U);
  EXPECT_EQ(offsetof(GroupAlignmentProbe, range), 8U);

  PopulationRecord const population{};
  EXPECT_EQ(population.population_mode, 0U);
  EXPECT_EQ(population.first_emission_index, 0U);
  EXPECT_EQ(population.emission_count, 0U);
  EXPECT_FLOAT_EQ(population.scaled_decay, 0.0F);

  EmissionRecord const emission{};
  EXPECT_EQ(emission.particle_type, 0U);
  EXPECT_EQ(emission.energy_distribution_record_index, 0U);
  EXPECT_EQ(emission.mono_energy_milli_eV, 0ULL);

  GroupRange const range{};
  EXPECT_EQ(range.source_local_primary_begin, 0ULL);
  EXPECT_EQ(range.primary_count, 0ULL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRadionuclideRuntimeRecordKernelTest,
       HostAndOpenCLLayoutsStridesAndValuesMatch) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = opencl.GetContext().front();

  std::array<PopulationRecord, 2U> populations{{
      {.population_mode = 7U,
       .first_emission_index = 11U,
       .emission_count = 13U,
       .scaled_decay = 0.25F},
      {},
  }};
  std::array<EmissionRecord, 2U> emissions{{
      {.particle_type = 17U,
       .energy_distribution_record_index = 19U,
       .mono_energy_milli_eV = 23ULL},
      {},
  }};
  std::array<GroupRange, 2U> groups{{
      {.source_local_primary_begin = 29ULL, .primary_count = 31ULL},
      {},
  }};
  std::array<std::uint64_t, 18U> layout{};

  auto layout_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{layout.size() * sizeof(std::uint64_t)});
  auto population_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{populations.size() * sizeof(PopulationRecord)});
  auto emission_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{emissions.size() * sizeof(EmissionRecord)});
  auto group_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{groups.size() * sizeof(GroupRange)});

  ggems::ocl::WriteSVMFromHost(layout_buffer, std::span{layout});
  ggems::ocl::WriteSVMFromHost(population_buffer, std::span{populations});
  ggems::ocl::WriteSVMFromHost(emission_buffer, std::span{emissions});
  ggems::ocl::WriteSVMFromHost(group_buffer, std::span{groups});

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";
  std::string const build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "radionuclide_runtime_record_abi_probe",
      build_options);
  cl::Kernel raw_kernel =
      program.CreateKernel("radionuclide_runtime_record_abi_probe");
  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "radionuclide_runtime_record_abi_probe"};

  kernel.SetArgSVMPointer(0U, layout_buffer.GetData());
  kernel.SetArgSVMPointer(1U, population_buffer.GetData());
  kernel.SetArgSVMPointer(2U, emission_buffer.GetData());
  kernel.SetArgSVMPointer(3U, group_buffer.GetData());
  kernel.Run({1U}, {1U});

  ggems::ocl::ReadSVMToHost(layout_buffer, std::span{layout});
  ggems::ocl::ReadSVMToHost(population_buffer, std::span{populations});
  ggems::ocl::ReadSVMToHost(emission_buffer, std::span{emissions});
  ggems::ocl::ReadSVMToHost(group_buffer, std::span{groups});

  std::array<std::uint64_t, 18U> const expected_layout{{
      sizeof(PopulationRecord),
      offsetof(PopulationRecord, population_mode),
      offsetof(PopulationRecord, first_emission_index),
      offsetof(PopulationRecord, emission_count),
      offsetof(PopulationRecord, scaled_decay),
      sizeof(PopulationRecord),
      offsetof(PopulationAlignmentProbe, record),
      sizeof(EmissionRecord),
      offsetof(EmissionRecord, particle_type),
      offsetof(EmissionRecord, energy_distribution_record_index),
      offsetof(EmissionRecord, mono_energy_milli_eV),
      sizeof(EmissionRecord),
      offsetof(EmissionAlignmentProbe, record),
      sizeof(GroupRange),
      offsetof(GroupRange, source_local_primary_begin),
      offsetof(GroupRange, primary_count),
      sizeof(GroupRange),
      offsetof(GroupAlignmentProbe, range),
  }};

  EXPECT_EQ(layout, expected_layout);

  EXPECT_EQ(populations[0U].population_mode, 7U);
  EXPECT_EQ(populations[0U].first_emission_index, 11U);
  EXPECT_EQ(populations[0U].emission_count, 13U);
  EXPECT_FLOAT_EQ(populations[0U].scaled_decay, 0.25F);
  EXPECT_EQ(populations[1U].population_mode, 37U);
  EXPECT_EQ(populations[1U].first_emission_index, 41U);
  EXPECT_EQ(populations[1U].emission_count, 43U);
  EXPECT_FLOAT_EQ(populations[1U].scaled_decay, 1.5F);

  EXPECT_EQ(emissions[0U].particle_type, 17U);
  EXPECT_EQ(emissions[0U].energy_distribution_record_index, 19U);
  EXPECT_EQ(emissions[0U].mono_energy_milli_eV, 23ULL);
  EXPECT_EQ(emissions[1U].particle_type, 47U);
  EXPECT_EQ(emissions[1U].energy_distribution_record_index, 53U);
  EXPECT_EQ(emissions[1U].mono_energy_milli_eV, 59ULL);

  EXPECT_EQ(groups[0U].source_local_primary_begin, 29ULL);
  EXPECT_EQ(groups[0U].primary_count, 31ULL);
  EXPECT_EQ(groups[1U].source_local_primary_begin, 61ULL);
  EXPECT_EQ(groups[1U].primary_count, 67ULL);
}
