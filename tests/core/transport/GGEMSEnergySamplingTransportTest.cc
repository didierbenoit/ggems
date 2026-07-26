#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace {

// =============================================================================
// =============================================================================

using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using ObserverRecordKind = ggems::core::observer::GGEMSObserverRecordKind;
using Source = ggems::core::sources::GGEMSSource;
using SourceSnapshot = ggems::core::sources::GGEMSSourceRunSnapshot;
using TransportConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;

// =============================================================================
// =============================================================================

template <typename T>
concept HasPerRunEnergyTables = requires(T value) {
  value.energy_distribution_records;
  value.energy_values_milli_eV;
  value.cumulative_ticket_upper;
};

static_assert(!HasPerRunEnergyTables<TransportConfig>);

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeConfig(SourceSnapshot const &snapshot,
                              std::uint32_t workload_primary_count,
                              std::uint64_t device_primary_offset)
    -> TransportConfig {
  TransportConfig config{};
  config.total_primary_count = workload_primary_count;
  config.projection_history_offset = 100ULL;
  config.device_primary_offset = device_primary_offset;
  config.source_records = snapshot.GetRecords();
  config.source_ranges = snapshot.GetRanges();
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 3U;
  return config;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindTerminal(TransportReport const &report,
                                ObserverRecord const &source)
    -> ObserverRecord const * {
  auto const terminal_kind = ggems::core::observer::ToKernelObserverRecordKind(
      ObserverRecordKind::Terminal);

  auto const terminal = std::ranges::find_if(
      report.observer_records, [&](ObserverRecord const &record) -> bool {
        return record.record_kind == terminal_kind &&
               record.global_primary_id == source.global_primary_id &&
               record.global_particle_id == source.global_particle_id;
      });

  return terminal == report.observer_records.end() ? nullptr : &*terminal;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindSource(TransportReport const &report)
    -> ObserverRecord const * {
  constexpr auto source_kind =
      ggems::core::observer::ToKernelObserverRecordKind(
          ObserverRecordKind::Source);
  auto const source = std::ranges::find_if(
      report.observer_records, [](ObserverRecord const &record) -> bool {
        return record.record_kind == source_kind;
      });

  return source == report.observer_records.end() ? nullptr : &*source;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeGeometrySource(std::size_t geometry_case)
    -> std::shared_ptr<Source> {
  auto source = std::make_shared<Source>();
  source->SetPrimaryCount(1ULL);

  if (geometry_case == 0U) {
    source->SetRectangleEmissionPicoMeter(4'000'000ULL, 2'000'000ULL)
        .SetFixedAngularDistribution();
  } else if (geometry_case == 1U) {
    source->SetEllipseEmissionPicoMeter(4'000'000ULL, 2'000'000ULL)
        .SetIsotropicAngularDistribution();
  } else {
    source->SetRectangleEmissionPicoMeter(4'000'000ULL, 2'000'000ULL)
        .SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 1'000'000'000'000LL);
  }

  return source;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FloatBits(float value) noexcept -> std::uint32_t {
  return std::bit_cast<std::uint32_t>(value);
}

// =============================================================================
// =============================================================================

class GGEMSEnergySamplingTransportTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialise();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static auto Context() -> ggems::ocl::GGEMSOpenCLContext & {
    return ggems::ocl::GGEMSOpenCL::GetInstance().GetContext().front();
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergySamplingTransportTest,
       SamplesMixedModesAcrossSourceBoundaryPaddingAndAllEngines) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<double, 3U> k_line_energies{2.0, 4.0, 6.0};
  constexpr std::array<double, 3U> k_line_weights{1.0, 0.0, 1.0};
  constexpr std::array<double, 3U> k_bin_centers{10.0, 12.0, 14.0};
  constexpr std::array<double, 3U> k_bin_weights{1.0, 0.0, 1.0};

  auto mono = std::make_shared<Source>();
  mono->SetPrimaryCount(2ULL)
      .SetPointEmission()
      .SetFixedAngularDistribution()
      .SetEnergyMilliElectronVolt(511'000'000ULL);

  auto discrete = std::make_shared<Source>();
  discrete->SetPrimaryCount(2ULL)
      .SetPointEmission()
      .SetFixedAngularDistribution()
      .SetDiscreteEnergyLines(k_line_energies, k_line_weights, "MeV");

  auto regular = std::make_shared<Source>();
  regular->SetPrimaryCount(2ULL)
      .SetPointEmission()
      .SetFixedAngularDistribution()
      .SetRegularEnergySpectrum(k_bin_centers, k_bin_weights, "MeV");

  std::vector<std::shared_ptr<Source>> sources{mono, discrete, regular};
  auto const snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  ASSERT_EQ(snapshot.GetEnergyValuesMilliElectronVolt().size(), 6U);

  for (std::string_view engine : k_engines) {
    SCOPED_TRACE(engine);

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(88'881ULL);

    TransportWorkload workload{Context(),
                               std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                               random,
                               65U,
                               snapshot.GetSourceConfiguration(),
                               0ULL,
                               0U,
                               8U};

    // Logical projection IDs [1, 5) cross both source boundaries. The
    // 65-worker launch is padded to 128 work-items.
    auto const report = workload.Run(MakeConfig(snapshot, 4U, 1ULL));

    EXPECT_EQ(report.counters.consumed_primary_count, 4U);
    EXPECT_EQ(report.counters.completed_history_count, 4U);
    EXPECT_EQ(report.counters.terminal_particle_count, 4U);
    EXPECT_EQ(report.counters.created_secondary_count, 0U);
    EXPECT_EQ(report.counters.overflow_count, 0U);
    EXPECT_EQ(report.observer_counters.captured_primary_count, 4U);
    EXPECT_EQ(report.observer_counters.record_count, 8U);
    EXPECT_EQ(report.observer_counters.overflow_count, 0U);

    auto const source_kind = ggems::core::observer::ToKernelObserverRecordKind(
        ObserverRecordKind::Source);
    std::array<std::uint32_t, 3U> source_counts{};

    for (ObserverRecord const &record : report.observer_records) {
      if (record.record_kind != source_kind) {
        continue;
      }

      ASSERT_LT(record.source_index, source_counts.size());
      ++source_counts[record.source_index];

      ObserverRecord const *terminal = FindTerminal(report, record);
      ASSERT_NE(terminal, nullptr);
      EXPECT_EQ(terminal->energy_milli_eV, record.energy_milli_eV);

      if (record.source_index == 0U) {
        EXPECT_EQ(record.energy_milli_eV, 511'000'000ULL);
      } else if (record.source_index == 1U) {
        EXPECT_TRUE(record.energy_milli_eV == 2'000'000'000ULL ||
                    record.energy_milli_eV == 6'000'000'000ULL);
        EXPECT_NE(record.energy_milli_eV, 4'000'000'000ULL);
      } else {
        bool const first_center_defined_bin =
            record.energy_milli_eV >= 9'000'000'000ULL &&
            record.energy_milli_eV < 11'000'000'000ULL;
        bool const third_center_defined_bin =
            record.energy_milli_eV >= 13'000'000'000ULL &&
            record.energy_milli_eV < 15'000'000'000ULL;

        EXPECT_TRUE(first_center_defined_bin || third_center_defined_bin);
        EXPECT_FALSE(record.energy_milli_eV >= 11'000'000'000ULL &&
                     record.energy_milli_eV < 13'000'000'000ULL);
      }
    }

    EXPECT_EQ(source_counts[0U], 1U);
    EXPECT_EQ(source_counts[1U], 2U);
    EXPECT_EQ(source_counts[2U], 1U);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergySamplingTransportTest,
       SamplesEnergyAfterGeometryAndDirectionForEveryEngine) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<std::string_view, 3U> k_geometry_cases{
      "Rectangle + Fixed", "Ellipse + Isotropic", "Rectangle + Focused"};
  constexpr std::array<double, 2U> k_line_energies{2.0, 6.0};
  constexpr std::array<double, 2U> k_line_weights{1.0, 1.0};

  for (std::string_view engine : k_engines) {
    for (std::size_t geometry_case = 0U;
         geometry_case < k_geometry_cases.size(); ++geometry_case) {
      SCOPED_TRACE(engine);
      SCOPED_TRACE(k_geometry_cases[geometry_case]);

      auto mono_source = MakeGeometrySource(geometry_case);
      mono_source->SetEnergyMilliElectronVolt(511'000'000ULL);
      auto table_source = MakeGeometrySource(geometry_case);
      table_source->SetDiscreteEnergyLines(k_line_energies, k_line_weights,
                                           "MeV");

      std::array<std::shared_ptr<Source>, 1U> mono_sources{mono_source};
      std::array<std::shared_ptr<Source>, 1U> table_sources{table_source};
      auto const mono_snapshot =
          ggems::core::sources::BuildSourceRunSnapshot(mono_sources);
      auto const table_snapshot =
          ggems::core::sources::BuildSourceRunSnapshot(table_sources);

      ggems::core::random::GGEMSRandom random{};
      random.SetEngine(engine).SetSeed(91'337ULL);

      TransportWorkload mono_workload{
          Context(),
          std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
          random,
          1U,
          mono_snapshot.GetSourceConfiguration(),
          0ULL,
          0U,
          4U};
      TransportWorkload table_workload{
          Context(),
          std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
          random,
          1U,
          table_snapshot.GetSourceConfiguration(),
          0ULL,
          0U,
          4U};

      auto const mono_report =
          mono_workload.Run(MakeConfig(mono_snapshot, 1U, 0ULL));
      auto const table_report =
          table_workload.Run(MakeConfig(table_snapshot, 1U, 0ULL));

      ObserverRecord const *mono_record = FindSource(mono_report);
      ObserverRecord const *table_record = FindSource(table_report);
      ASSERT_NE(mono_record, nullptr);
      ASSERT_NE(table_record, nullptr);

      EXPECT_EQ(mono_record->position_x_pm, table_record->position_x_pm);
      EXPECT_EQ(mono_record->position_y_pm, table_record->position_y_pm);
      EXPECT_EQ(mono_record->position_z_pm, table_record->position_z_pm);
      EXPECT_EQ(FloatBits(mono_record->direction_x),
                FloatBits(table_record->direction_x));
      EXPECT_EQ(FloatBits(mono_record->direction_y),
                FloatBits(table_record->direction_y));
      EXPECT_EQ(FloatBits(mono_record->direction_z),
                FloatBits(table_record->direction_z));

      EXPECT_EQ(mono_record->energy_milli_eV, 511'000'000ULL);
      EXPECT_TRUE(table_record->energy_milli_eV == 2'000'000'000ULL ||
                  table_record->energy_milli_eV == 6'000'000'000ULL);
    }
  }
}
