#include <algorithm>
#include <array>
#include <bit>
#include <limits>
#include <span>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"

namespace {

using ObserverConfigRecord = ggems::core::observer::GGEMSObserverConfigRecord;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;

struct ObserverConfigAlignmentProbe {
  std::uint8_t prefix;
  ObserverConfigRecord config;
};

constexpr std::size_t k_layout_value_count{32U};
constexpr std::size_t k_record_count{2U};
constexpr std::size_t k_config_count{2U};

// =============================================================================
// =============================================================================

class GGEMSObserverRecordKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
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
    std::format("-I{}", kernel_root.generic_string());

  auto layout_buffer = context.CreateSVMBuffer(
    ggems::units::Bytes{k_layout_value_count * sizeof(std::uint64_t)});

  auto records_buffer = context.CreateSVMBuffer(
    ggems::units::Bytes{k_record_count * sizeof(ObserverRecord)});

  auto configs_buffer = context.CreateSVMBuffer(
    ggems::units::Bytes{k_config_count * sizeof(ObserverConfigRecord)});

  auto *layout = static_cast<std::uint64_t *>(layout_buffer.GetData());
  auto *records = static_cast<ObserverRecord *>(records_buffer.GetData());
  auto *configs = static_cast<ObserverConfigRecord *>(configs_buffer.GetData());

  layout_buffer.Map(CL_MAP_WRITE);
  std::fill_n(layout, k_layout_value_count, 0ULL);
  layout_buffer.Unmap();

  records_buffer.Map(CL_MAP_WRITE);
  std::fill_n(records, k_record_count, ObserverRecord{});
  records_buffer.Unmap();

  configs_buffer.Map(CL_MAP_WRITE);
  std::fill_n(configs, k_config_count, ObserverConfigRecord{});
  configs_buffer.Unmap();

  auto const &program = opencl.GetOrCreateProgram(
    context, kernel_test_root, "observer_record_abi_probe", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("observer_record_abi_probe");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "observer_record_abi_probe"};

  kernel.SetArgSVMPointer(0U, layout);
  kernel.SetArgSVMPointer(1U, records);
  kernel.SetArgSVMPointer(2U, configs);
  kernel.Run({1U}, {1U});

  std::array<std::uint64_t, k_layout_value_count> const expected_layout{
    {static_cast<std::uint64_t>(sizeof(ObserverRecord)),
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
     static_cast<std::uint64_t>(offsetof(ObserverRecord, energy_micro_eV)),
     static_cast<std::uint64_t>(
       offsetof(ObserverRecord, deposited_energy_micro_eV)),
     static_cast<std::uint64_t>(offsetof(ObserverRecord, source_index)),
     static_cast<std::uint64_t>(sizeof(ObserverRecord)),
     static_cast<std::uint64_t>(sizeof(ObserverConfigRecord)),
     static_cast<std::uint64_t>(offsetof(ObserverConfigRecord, enabled)),
     static_cast<std::uint64_t>(
       offsetof(ObserverConfigRecord, capture_first_primary_count_per_source)),
     static_cast<std::uint64_t>(
       offsetof(ObserverConfigRecord, capture_specific_primary_enabled)),
     static_cast<std::uint64_t>(
       offsetof(ObserverConfigRecord, capture_source_index)),
     static_cast<std::uint64_t>(
       offsetof(ObserverConfigRecord, capture_source_local_primary_id)),
     static_cast<std::uint64_t>(sizeof(ObserverConfigRecord)),
     static_cast<std::uint64_t>(offsetof(ObserverConfigAlignmentProbe, config)),
     static_cast<std::uint64_t>(offsetof(ObserverRecord, position_valid))}};

  layout_buffer.Map(CL_MAP_READ);

  for (std::size_t index = 0U; index < expected_layout.size(); ++index) {
    EXPECT_EQ(layout[index], expected_layout[index]) << index;
  }

  layout_buffer.Unmap();

  records_buffer.Map(CL_MAP_READ);

  EXPECT_EQ(records[0U].energy_micro_eV, 101ULL);
  EXPECT_EQ(records[0U].deposited_energy_micro_eV, 202ULL);
  EXPECT_EQ(records[0U].source_index, 3U);

  EXPECT_EQ(records[1U].energy_micro_eV, 303ULL);
  EXPECT_EQ(records[1U].deposited_energy_micro_eV, 404ULL);
  EXPECT_EQ(records[1U].source_index, 5U);

  records_buffer.Unmap();

  configs_buffer.Map(CL_MAP_READ);

  EXPECT_EQ(configs[0U].enabled, 1U);
  EXPECT_EQ(configs[0U].capture_first_primary_count_per_source, 3U);
  EXPECT_EQ(configs[0U].capture_specific_primary_enabled, 1U);
  EXPECT_EQ(configs[0U].capture_source_index, 7U);
  EXPECT_EQ(configs[0U].capture_source_local_primary_id, 11ULL);

  EXPECT_EQ(configs[1U].enabled, 0U);
  EXPECT_EQ(configs[1U].capture_first_primary_count_per_source, 5U);
  EXPECT_EQ(configs[1U].capture_specific_primary_enabled, 1U);
  EXPECT_EQ(configs[1U].capture_source_index, 9U);
  EXPECT_EQ(configs[1U].capture_source_local_primary_id, 13ULL);

  configs_buffer.Unmap();
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSObserverRecordKernelTest,
       ExportsCompactStateWithoutSaturatingPosition) {
  using ParticleState = ggems::core::particles::GGEMSParticleState;
  using ObserverCounters = ggems::core::observer::GGEMSObserverCounters;
  struct ExportCase {
    std::array<float, 3U> position;
    std::uint32_t valid;
    std::array<std::int64_t, 3U> expected;
  };
  std::array<ExportCase, 8U> const cases{{
    {.position = {0.5F, -0.5F, 1.5F},
     .valid = 1U,
     .expected = {1LL, -1LL, 2LL}},
    {.position = {-1.5F, 0.25F, -0.25F},
     .valid = 1U,
     .expected = {-2LL, 0LL, 0LL}},
    {.position = {std::bit_cast<float>(0x5EFFFFFFU),
                  std::bit_cast<float>(0xDF000000U), 0.0F},
     .valid = 1U,
     .expected = {9'223'371'487'098'961'920LL,
                  std::numeric_limits<std::int64_t>::min(), 0LL}},
    {.position = {std::bit_cast<float>(0x5F000000U), 1.0F, 2.0F},
     .valid = 0U,
     .expected = {0LL, 0LL, 0LL}},
    {.position = {1.0F, std::bit_cast<float>(0xDF000001U), 2.0F},
     .valid = 0U,
     .expected = {0LL, 0LL, 0LL}},
    {.position = {1.0F, 2.0F, std::numeric_limits<float>::infinity()},
     .valid = 0U,
     .expected = {0LL, 0LL, 0LL}},
    {.position = {-std::numeric_limits<float>::infinity(), 1.0F, 2.0F},
     .valid = 0U,
     .expected = {0LL, 0LL, 0LL}},
    {.position = {1.0F, std::numeric_limits<float>::quiet_NaN(), 2.0F},
     .valid = 0U,
     .expected = {0LL, 0LL, 0LL}},
  }};
  std::array<ParticleState, cases.size()> particles{};
  for (std::size_t index = 0U; index < cases.size(); ++index) {
    auto &particle = particles[index];
    particle.global_particle_id = 4294967297ULL;
    particle.track_id =
      index == 0U ? ggems::core::particles::k_invalid_id_u32 : 7U;
    particle.parent_track_id = ggems::core::particles::k_invalid_id_u32;
    particle.position_x_pm = cases[index].position[0U];
    particle.position_y_pm = cases[index].position[1U];
    particle.position_z_pm = cases[index].position[2U];
    particle.particle_type = 7U;
    particle.status = 4U;
    particle.generation = 255U;
    particle.energy_micro_eV = 511'000'000'000ULL;
  }

  auto &context = GetContext();
  auto particles_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(particles)});
  auto records_buffer = context.CreateSVMBuffer(
    ggems::units::Bytes{cases.size() * sizeof(ObserverRecord)});
  auto counters_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(ObserverCounters)});
  auto *records = static_cast<ObserverRecord *>(records_buffer.GetData());
  auto *counters = static_cast<ObserverCounters *>(counters_buffer.GetData());
  ggems::ocl::WriteSVMFromHost(particles_buffer,
                               std::span<ParticleState const>{particles});
  counters_buffer.Map(CL_MAP_WRITE);
  *counters = ObserverCounters{.captured_primary_count = 3U};
  counters_buffer.Unmap();

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  auto const &program =
    ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, kernel_root / "tests", "observer_record_abi_probe",
      std::format("-I{}", kernel_root.generic_string()));
  auto raw_kernel = program.CreateKernel("observer_particle_export_probe");
  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "observer_particle_export_probe"};
  kernel.SetArgSVMPointer(0U, particles_buffer.GetData());
  kernel.SetArgSVMPointer(1U, counters);
  kernel.SetArgSVMPointer(2U, records);
  kernel.SetArg(3U, static_cast<std::uint32_t>(cases.size()));
  kernel.Run({1U}, {1U});

  counters_buffer.Map(CL_MAP_READ);
  EXPECT_EQ(counters->record_count, cases.size());
  EXPECT_EQ(counters->overflow_count, 0U);
  EXPECT_EQ(counters->captured_primary_count, 3U);
  counters_buffer.Unmap();
  records_buffer.Map(CL_MAP_READ);
  for (std::size_t index = 0U; index < cases.size(); ++index) {
    SCOPED_TRACE(index);
    auto const &record = records[index];
    EXPECT_EQ(record.position_valid, cases[index].valid);
    EXPECT_EQ(record.position_x_pm, cases[index].expected[0U]);
    EXPECT_EQ(record.position_y_pm, cases[index].expected[1U]);
    EXPECT_EQ(record.position_z_pm, cases[index].expected[2U]);
    EXPECT_EQ(record.global_particle_id, 4294967297ULL);
    EXPECT_EQ(record.track_id,
              index == 0U ? ggems::core::particles::k_invalid_id_u64 : 7ULL);
    EXPECT_EQ(record.parent_track_id, ggems::core::particles::k_invalid_id_u64);
    EXPECT_EQ(record.particle_type, 7U);
    EXPECT_EQ(record.status, 4U);
    EXPECT_EQ(record.generation, 255U);
    EXPECT_FLOAT_EQ(record.direction_w, 0.0F);
    EXPECT_EQ(record.energy_micro_eV, 511'000'000'000ULL);
  }
  records_buffer.Unmap();
}
