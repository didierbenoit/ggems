#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace {

using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;

constexpr std::size_t k_layout_value_count{24U};
constexpr std::size_t k_record_count{2U};

// =============================================================================
// =============================================================================

class GGEMSObserverRecordKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialise();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static auto GetContext() -> ggems::ocl::GGEMSOpenCLContext & {
    return ggems::ocl::GGEMSOpenCL::GetInstance().GetContext().front();
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSObserverRecordKernelTest, HostAndKernelLayoutsMatch) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";

  std::string const build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto layout_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{k_layout_value_count * sizeof(std::uint64_t)});

  auto records_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{k_record_count * sizeof(ObserverRecord)});

  auto *layout = static_cast<std::uint64_t *>(layout_buffer.GetData());
  auto *records = static_cast<ObserverRecord *>(records_buffer.GetData());

  layout_buffer.Map(CL_MAP_WRITE);
  std::fill_n(layout, k_layout_value_count, 0ULL);
  layout_buffer.Unmap();

  records_buffer.Map(CL_MAP_WRITE);
  std::fill_n(records, k_record_count, ObserverRecord{});
  records_buffer.Unmap();

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "observer_record_abi_probe", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("observer_record_abi_probe");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "observer_record_abi_probe"};

  kernel.SetArgSVMPointer(0U, layout);
  kernel.SetArgSVMPointer(1U, records);
  kernel.Run({1U}, {1U});

  std::array<std::uint64_t, k_layout_value_count> const expected_layout{
      static_cast<std::uint64_t>(sizeof(ObserverRecord)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, run_id)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, global_primary_id)),
      static_cast<std::uint64_t>(
          offsetof(ObserverRecord, source_local_primary_id)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, global_particle_id)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, track_id)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, parent_track_id)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, time_ps)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, position_x_pm)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, position_y_pm)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, position_z_pm)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, record_kind)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, particle_type)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, status)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, generation)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, direction_x)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, direction_y)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, direction_z)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, direction_w)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, energy_milli_eV)),
      static_cast<std::uint64_t>(
          offsetof(ObserverRecord, deposited_energy_milli_eV)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, weight)),
      static_cast<std::uint64_t>(offsetof(ObserverRecord, source_index)),
      static_cast<std::uint64_t>(sizeof(ObserverRecord))};

  layout_buffer.Map(CL_MAP_READ);

  for (std::size_t index = 0U; index < expected_layout.size(); ++index) {
    EXPECT_EQ(layout[index], expected_layout[index]) << index;
  }

  layout_buffer.Unmap();

  records_buffer.Map(CL_MAP_READ);

  EXPECT_EQ(records[0U].energy_milli_eV, 101ULL);
  EXPECT_EQ(records[0U].deposited_energy_milli_eV, 202ULL);
  EXPECT_FLOAT_EQ(records[0U].weight, 0.25F);
  EXPECT_EQ(records[0U].source_index, 3U);

  EXPECT_EQ(records[1U].energy_milli_eV, 303ULL);
  EXPECT_EQ(records[1U].deposited_energy_milli_eV, 404ULL);
  EXPECT_FLOAT_EQ(records[1U].weight, 0.75F);
  EXPECT_EQ(records[1U].source_index, 5U);

  records_buffer.Unmap();
}
