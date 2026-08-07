#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;

// =============================================================================
// =============================================================================

struct SourceRecordAlignmentProbe {
  std::uint8_t prefix;
  SourceRecord record;
};

// =============================================================================
// =============================================================================

struct SourceRunRangeAlignmentProbe {
  std::uint8_t prefix;
  SourceRunRange range;
};

// =============================================================================
// =============================================================================

constexpr std::size_t k_layout_value_count{41U};
constexpr std::size_t k_host_value_count{35U};
constexpr std::size_t k_source_record_count{2U};
constexpr std::size_t k_source_range_count{2U};

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeHostSourceRecord() -> SourceRecord {
  return {.source_id = 101ULL,
          .time_start_ps = 102ULL,
          .time_stop_ps = 103ULL,
          .energy_milli_eV = 104ULL,
          .position_x_pm = -105LL,
          .position_y_pm = 106LL,
          .position_z_pm = -107LL,
          .source_type = 108U,
          .emitted_particle_type = 109U,
          .flags = 110U,
          .reserved_0 = 111U,
          .axis_x_x = -0.125F,
          .axis_x_y = 0.25F,
          .axis_x_z = -0.5F,
          .axis_y_x = 1.25F,
          .axis_y_y = -2.5F,
          .axis_y_z = 3.75F,
          .axis_z_x = -4.125F,
          .axis_z_y = 5.25F,
          .axis_z_z = -6.5F,
          .weight = 0.875F,
          .emission_geometry_type = 112U,
          .angular_distribution_type = 113U,
          .geometry_size_x_pm = 114ULL,
          .geometry_size_y_pm = 115ULL,
          .focus_position_x_pm = -116LL,
          .focus_position_y_pm = 117LL,
          .focus_position_z_pm = -118LL,
          .geometry_size_z_pm = 119ULL,
          .isotropic_cos_theta_lower = -0.625F,
          .isotropic_cos_theta_upper = 0.75F,
          .isotropic_phi_min_rad = -1.125F,
          .isotropic_phi_max_rad = 2.25F};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDeviceSourceRecord() -> SourceRecord {
  return {.source_id = 201ULL,
          .time_start_ps = 202ULL,
          .time_stop_ps = 203ULL,
          .energy_milli_eV = 204ULL,
          .position_x_pm = -205LL,
          .position_y_pm = 206LL,
          .position_z_pm = -207LL,
          .source_type = 208U,
          .emitted_particle_type = 209U,
          .flags = 210U,
          .reserved_0 = 211U,
          .axis_x_x = -7.25F,
          .axis_x_y = 8.5F,
          .axis_x_z = -9.75F,
          .axis_y_x = 10.125F,
          .axis_y_y = -11.25F,
          .axis_y_z = 12.5F,
          .axis_z_x = -13.75F,
          .axis_z_y = 14.875F,
          .axis_z_z = -15.5F,
          .weight = 0.625F,
          .emission_geometry_type =
              ggems::core::sources::ToKernelEmissionGeometryType(
                  ggems::core::sources::GGEMSEmissionGeometryType::Ellipse),
          .angular_distribution_type =
              ggems::core::sources::ToKernelAngularDistributionType(
                  ggems::core::sources::GGEMSAngularDistributionType::Focused),
          .geometry_size_x_pm = 212ULL,
          .geometry_size_y_pm = 213ULL,
          .focus_position_x_pm = -214LL,
          .focus_position_y_pm = 215LL,
          .focus_position_z_pm = -216LL,
          .geometry_size_z_pm = 217ULL,
          .isotropic_cos_theta_lower = -0.75F,
          .isotropic_cos_theta_upper = 0.875F,
          .isotropic_phi_min_rad = -1.25F,
          .isotropic_phi_max_rad = 2.5F};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto EncodeSourceRecord(SourceRecord const &record)
    -> std::array<std::uint64_t, 33U> {
  return {
      {record.source_id,
       record.time_start_ps,
       record.time_stop_ps,
       record.energy_milli_eV,
       std::bit_cast<std::uint64_t>(record.position_x_pm),
       std::bit_cast<std::uint64_t>(record.position_y_pm),
       std::bit_cast<std::uint64_t>(record.position_z_pm),
       static_cast<std::uint64_t>(record.source_type),
       static_cast<std::uint64_t>(record.emitted_particle_type),
       static_cast<std::uint64_t>(record.flags),
       static_cast<std::uint64_t>(record.reserved_0),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_x_x)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_x_y)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_x_z)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_y_x)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_y_y)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_y_z)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_z_x)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_z_y)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.axis_z_z)),
       static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(record.weight)),
       static_cast<std::uint64_t>(record.emission_geometry_type),
       static_cast<std::uint64_t>(record.angular_distribution_type),
       record.geometry_size_x_pm,
       record.geometry_size_y_pm,
       std::bit_cast<std::uint64_t>(record.focus_position_x_pm),
       std::bit_cast<std::uint64_t>(record.focus_position_y_pm),
       std::bit_cast<std::uint64_t>(record.focus_position_z_pm),
       record.geometry_size_z_pm,
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.isotropic_cos_theta_lower)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.isotropic_cos_theta_upper)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.isotropic_phi_min_rad)),
       static_cast<std::uint64_t>(
           std::bit_cast<std::uint32_t>(record.isotropic_phi_max_rad))}};
}

// =============================================================================
// =============================================================================

auto ExpectSourceRecordsEqual(SourceRecord const &actual,
                              SourceRecord const &expected) -> void {
  EXPECT_EQ(actual.source_id, expected.source_id);
  EXPECT_EQ(actual.time_start_ps, expected.time_start_ps);
  EXPECT_EQ(actual.time_stop_ps, expected.time_stop_ps);
  EXPECT_EQ(actual.energy_milli_eV, expected.energy_milli_eV);
  EXPECT_EQ(actual.position_x_pm, expected.position_x_pm);
  EXPECT_EQ(actual.position_y_pm, expected.position_y_pm);
  EXPECT_EQ(actual.position_z_pm, expected.position_z_pm);
  EXPECT_EQ(actual.source_type, expected.source_type);
  EXPECT_EQ(actual.emitted_particle_type, expected.emitted_particle_type);
  EXPECT_EQ(actual.flags, expected.flags);
  EXPECT_EQ(actual.reserved_0, expected.reserved_0);
  EXPECT_FLOAT_EQ(actual.axis_x_x, expected.axis_x_x);
  EXPECT_FLOAT_EQ(actual.axis_x_y, expected.axis_x_y);
  EXPECT_FLOAT_EQ(actual.axis_x_z, expected.axis_x_z);
  EXPECT_FLOAT_EQ(actual.axis_y_x, expected.axis_y_x);
  EXPECT_FLOAT_EQ(actual.axis_y_y, expected.axis_y_y);
  EXPECT_FLOAT_EQ(actual.axis_y_z, expected.axis_y_z);
  EXPECT_FLOAT_EQ(actual.axis_z_x, expected.axis_z_x);
  EXPECT_FLOAT_EQ(actual.axis_z_y, expected.axis_z_y);
  EXPECT_FLOAT_EQ(actual.axis_z_z, expected.axis_z_z);
  EXPECT_FLOAT_EQ(actual.weight, expected.weight);
  EXPECT_EQ(actual.emission_geometry_type, expected.emission_geometry_type);
  EXPECT_EQ(actual.angular_distribution_type,
            expected.angular_distribution_type);
  EXPECT_EQ(actual.geometry_size_x_pm, expected.geometry_size_x_pm);
  EXPECT_EQ(actual.geometry_size_y_pm, expected.geometry_size_y_pm);
  EXPECT_EQ(actual.focus_position_x_pm, expected.focus_position_x_pm);
  EXPECT_EQ(actual.focus_position_y_pm, expected.focus_position_y_pm);
  EXPECT_EQ(actual.focus_position_z_pm, expected.focus_position_z_pm);
  EXPECT_EQ(actual.geometry_size_z_pm, expected.geometry_size_z_pm);
  EXPECT_FLOAT_EQ(actual.isotropic_cos_theta_lower,
                  expected.isotropic_cos_theta_lower);
  EXPECT_FLOAT_EQ(actual.isotropic_cos_theta_upper,
                  expected.isotropic_cos_theta_upper);
  EXPECT_FLOAT_EQ(actual.isotropic_phi_min_rad, expected.isotropic_phi_min_rad);
  EXPECT_FLOAT_EQ(actual.isotropic_phi_max_rad, expected.isotropic_phi_max_rad);
}

// =============================================================================
// =============================================================================

class GGEMSSourceRecordKernelTest : public ::testing::Test {
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

TEST_F(GGEMSSourceRecordKernelTest, HostAndKernelLayoutsAndValuesMatch) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::array<SourceRecord, k_source_record_count> source_records{
      MakeHostSourceRecord(), SourceRecord{}};
  std::array<SourceRunRange, k_source_range_count> source_ranges{
      SourceRunRange{.projection_primary_begin = 301ULL,
                     .primary_count = 302ULL},
      SourceRunRange{}};
  std::array<std::uint64_t, k_layout_value_count> layout{};
  std::array<std::uint64_t, k_host_value_count> host_values{};

  auto layout_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{layout.size() * sizeof(std::uint64_t)});
  auto source_records_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{source_records.size() * sizeof(SourceRecord)});
  auto source_ranges_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{source_ranges.size() * sizeof(SourceRunRange)});
  auto host_values_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{host_values.size() * sizeof(std::uint64_t)});

  ggems::ocl::WriteSVMFromHost(layout_buffer, std::span{layout});
  ggems::ocl::WriteSVMFromHost(source_records_buffer,
                               std::span{source_records});
  ggems::ocl::WriteSVMFromHost(source_ranges_buffer, std::span{source_ranges});
  ggems::ocl::WriteSVMFromHost(host_values_buffer, std::span{host_values});

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";
  std::string const build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "source_record_abi_probe", build_options);
  cl::Kernel raw_kernel = program.CreateKernel("source_record_abi_probe");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "source_record_abi_probe"};

  kernel.SetArgSVMPointer(0U, layout_buffer.GetData());
  kernel.SetArgSVMPointer(1U, source_records_buffer.GetData());
  kernel.SetArgSVMPointer(2U, source_ranges_buffer.GetData());
  kernel.SetArgSVMPointer(3U, host_values_buffer.GetData());
  kernel.Run({1U}, {1U});

  ggems::ocl::ReadSVMToHost(layout_buffer, std::span{layout});
  ggems::ocl::ReadSVMToHost(source_records_buffer, std::span{source_records});
  ggems::ocl::ReadSVMToHost(source_ranges_buffer, std::span{source_ranges});
  ggems::ocl::ReadSVMToHost(host_values_buffer, std::span{host_values});

  std::array<std::uint64_t, k_layout_value_count> const expected_layout{{
      static_cast<std::uint64_t>(sizeof(SourceRecord)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, source_id)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, time_start_ps)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, time_stop_ps)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, energy_milli_eV)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, position_x_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, position_y_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, position_z_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, source_type)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, emitted_particle_type)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, flags)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, reserved_0)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_x_x)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_x_y)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_x_z)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_y_x)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_y_y)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_y_z)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_z_x)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_z_y)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, axis_z_z)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, weight)),
      static_cast<std::uint64_t>(
          offsetof(SourceRecord, emission_geometry_type)),
      static_cast<std::uint64_t>(
          offsetof(SourceRecord, angular_distribution_type)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, geometry_size_x_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, geometry_size_y_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, focus_position_x_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, focus_position_y_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, focus_position_z_pm)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, geometry_size_z_pm)),
      static_cast<std::uint64_t>(
          offsetof(SourceRecord, isotropic_cos_theta_lower)),
      static_cast<std::uint64_t>(
          offsetof(SourceRecord, isotropic_cos_theta_upper)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, isotropic_phi_min_rad)),
      static_cast<std::uint64_t>(offsetof(SourceRecord, isotropic_phi_max_rad)),
      static_cast<std::uint64_t>(sizeof(SourceRecord)),
      static_cast<std::uint64_t>(offsetof(SourceRecordAlignmentProbe, record)),
      static_cast<std::uint64_t>(sizeof(SourceRunRange)),
      static_cast<std::uint64_t>(
          offsetof(SourceRunRange, projection_primary_begin)),
      static_cast<std::uint64_t>(offsetof(SourceRunRange, primary_count)),
      static_cast<std::uint64_t>(sizeof(SourceRunRange)),
      static_cast<std::uint64_t>(offsetof(SourceRunRangeAlignmentProbe, range)),
  }};

  for (std::size_t index = 0U; index < expected_layout.size(); ++index) {
    EXPECT_EQ(layout[index], expected_layout[index]) << index;
  }

  auto const expected_host_record_values =
      EncodeSourceRecord(MakeHostSourceRecord());

  for (std::size_t index = 0U; index < expected_host_record_values.size();
       ++index) {
    EXPECT_EQ(host_values[index], expected_host_record_values[index]) << index;
  }

  EXPECT_EQ(host_values[33U], 301ULL);
  EXPECT_EQ(host_values[34U], 302ULL);

  ExpectSourceRecordsEqual(source_records[1U], MakeDeviceSourceRecord());
  EXPECT_EQ(source_ranges[1U].projection_primary_begin, 401ULL);
  EXPECT_EQ(source_ranges[1U].primary_count, 402ULL);
}
