#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <set>
#include <span>
#include <string_view>
#include <tuple>
#include <iterator>
#include <vector>
#include <utility>
#include <functional>
#include <cstddef>
#include <memory>

#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/transport/GGEMSDiagnosticProjection.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"

namespace {

// =============================================================================
// =============================================================================

using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using ObserverRecordKind = ggems::core::observer::GGEMSObserverRecordKind;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportRunReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;
using SourceConfigurationSnapshotPtr =
    ggems::core::sources::GGEMSSourceConfigurationSnapshotPtr;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeMonoSourceConfiguration(std::size_t source_count)
    -> SourceConfigurationSnapshotPtr {
  std::vector<std::shared_ptr<ggems::core::sources::GGEMSSource>> sources;
  sources.reserve(source_count);

  for (std::size_t source_index = 0U; source_index < source_count;
       ++source_index) {
    sources.push_back(std::make_shared<ggems::core::sources::GGEMSSource>());
  }

  return ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildRanges(std::span<std::uint64_t const> counts)
    -> std::vector<SourceRunRange> {
  std::vector<SourceRunRange> ranges;
  std::uint64_t begin{0ULL};

  for (std::uint64_t count : counts) {
    ranges.push_back(
        {.projection_primary_begin = begin, .primary_count = count});
    begin += count;
  }

  return ranges;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeConfig(std::vector<SourceRecord> records,
                              std::span<std::uint64_t const> counts)
    -> TransportRunConfig {
  TransportRunConfig config{};
  config.total_primary_count = static_cast<std::uint32_t>(
      std::ranges::fold_left(counts, 0ULL, std::plus{}));
  config.source_records = std::move(records);
  config.source_population_records.resize(config.source_records.size());
  config.source_ranges = BuildRanges(counts);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source =
      static_cast<std::uint32_t>(config.total_primary_count);
  return config;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RecordsOfKind(TransportRunReport const &report,
                                 ObserverRecordKind kind)
    -> std::vector<ObserverRecord> {
  std::vector<ObserverRecord> records;
  std::uint32_t const kernel_kind =
      ggems::core::observer::ToKernelObserverRecordKind(kind);

  std::ranges::copy_if(report.observer_records, std::back_inserter(records),
                       [kernel_kind](ObserverRecord const &record) -> bool {
                         return record.record_kind == kernel_kind;
                       });

  return records;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindTerminal(TransportRunReport const &report,
                                ObserverRecord const &source)
    -> ObserverRecord const * {
  auto const terminal_kind = ggems::core::observer::ToKernelObserverRecordKind(
      ObserverRecordKind::Terminal);

  auto iterator = std::ranges::find_if(
      report.observer_records, [&](ObserverRecord const &record) -> bool {
        return record.record_kind == terminal_kind &&
               record.global_primary_id == source.global_primary_id &&
               record.global_particle_id == source.global_particle_id;
      });

  return iterator == report.observer_records.end() ? nullptr : &*iterator;
}

// =============================================================================
// =============================================================================

auto ExpectDiagnosticProjection(TransportRunReport const &report,
                                ObserverRecord const &source) -> void {
  ObserverRecord const *terminal = FindTerminal(report, source);
  ASSERT_NE(terminal, nullptr);

  std::array<float, 3U> const direction{source.direction_x, source.direction_y,
                                        source.direction_z};
  std::array<std::int64_t, 3U> const begin{
      source.position_x_pm, source.position_y_pm, source.position_z_pm};
  std::array<std::int64_t, 3U> const end{terminal->position_x_pm,
                                         terminal->position_y_pm,
                                         terminal->position_z_pm};

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    std::int64_t displacement{0LL};
    ASSERT_TRUE(ggems::core::transport::TryScaleDiagnosticProjectionComponent(
        direction[axis], displacement));
    EXPECT_EQ(end[axis], begin[axis] + displacement);
  }
}

// =============================================================================
// =============================================================================

class GGEMSSourceSamplingTransportTest : public ::testing::Test {
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

TEST_F(GGEMSSourceSamplingTransportTest,
       PointFixedAndFocusedDoNotConsumeRandom) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<std::uint64_t, 1U> k_eight{8ULL};
  constexpr std::array<std::uint64_t, 1U> k_one{1ULL};

  for (std::string_view engine : k_engines) {
    SCOPED_TRACE(engine);

    ggems::core::random::GGEMSRandom random_a{};
    ggems::core::random::GGEMSRandom random_b{};
    random_a.SetEngine(engine).SetSeed(7'777ULL);
    random_b.SetEngine(engine).SetSeed(7'777ULL);

    TransportWorkload workload_a{Context(),
                                 std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                                 random_a,
                                 1U,
                                 *MakeMonoSourceConfiguration(1U),
                                 0ULL,
                                 0U,
                                 16U};
    TransportWorkload workload_b{Context(),
                                 std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                                 random_b,
                                 1U,
                                 *MakeMonoSourceConfiguration(1U),
                                 0ULL,
                                 0U,
                                 2U};

    ggems::core::sources::GGEMSSource point_fixed{};
    auto fixed_config = MakeConfig({point_fixed.BuildRecord()}, k_eight);
    fixed_config.observer_config.enabled = 0U;
    workload_a.Run(fixed_config);

    ggems::core::sources::GGEMSSource point_focused{};
    point_focused.SetFocusedAngularDistributionPicoMeter(0LL, 0LL,
                                                         1'000'000'000'000LL);
    auto focused_config = MakeConfig({point_focused.BuildRecord()}, k_eight);
    focused_config.observer_config.enabled = 0U;
    workload_a.Run(focused_config);

    ggems::core::sources::GGEMSSource rectangle{};
    rectangle.SetRectangleEmissionPicoMeter(40'000'000'000ULL,
                                            20'000'000'000ULL);

    auto rectangle_config_a = MakeConfig({rectangle.BuildRecord()}, k_one);
    auto rectangle_config_b = MakeConfig({rectangle.BuildRecord()}, k_one);

    auto const report_a = workload_a.Run(rectangle_config_a);
    auto const report_b = workload_b.Run(rectangle_config_b);

    auto const source_a = RecordsOfKind(report_a, ObserverRecordKind::Source);
    auto const source_b = RecordsOfKind(report_b, ObserverRecordKind::Source);

    ASSERT_EQ(source_a.size(), 1U);
    ASSERT_EQ(source_b.size(), 1U);
    EXPECT_EQ(source_a[0U].position_x_pm, source_b[0U].position_x_pm);
    EXPECT_EQ(source_a[0U].position_y_pm, source_b[0U].position_y_pm);
    EXPECT_EQ(source_a[0U].position_z_pm, source_b[0U].position_z_pm);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingTransportTest,
       SamplesRectangleEllipseAndCircleInTheirLocalPlanes) {
  ggems::core::sources::GGEMSSource rectangle{};
  rectangle.SetPositionPicoMeter(100'000'000'000LL, 0LL, 0LL)
      .SetOrientation({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0})
      .SetRectangleEmissionPicoMeter(40'000'000'000ULL, 20'000'000'000ULL);

  ggems::core::sources::GGEMSSource ellipse{};
  ellipse.SetEllipseEmissionPicoMeter(30'000'000'000ULL, 10'000'000'000ULL);

  ggems::core::sources::GGEMSSource circle{};
  circle.SetCircleEmissionPicoMeter(20'000'000'000ULL);

  constexpr std::array<std::uint64_t, 3U> k_counts{256ULL, 256ULL, 256ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(8'888ULL);

  TransportWorkload workload{Context(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(3U),
                             0ULL,
                             0U,
                             1'536U};

  auto const report = workload.Run(MakeConfig(
      {rectangle.BuildRecord(), ellipse.BuildRecord(), circle.BuildRecord()},
      k_counts));

  auto const source_records = RecordsOfKind(report, ObserverRecordKind::Source);
  ASSERT_EQ(source_records.size(), 768U);

  std::array<std::set<std::tuple<std::int64_t, std::int64_t, std::int64_t>>, 3U>
      distinct_positions;

  std::array<SourceRecord, 3U> const configured_sources{
      rectangle.BuildRecord(), ellipse.BuildRecord(), circle.BuildRecord()};

  for (ObserverRecord const &record : source_records) {
    auto const source_index = static_cast<std::size_t>(record.source_index);

    ASSERT_LT(source_index, configured_sources.size());

    SourceRecord const &source = configured_sources[source_index];

    long double const delta_x_pm =
        static_cast<long double>(record.position_x_pm) -
        static_cast<long double>(source.position_x_pm);
    long double const delta_y_pm =
        static_cast<long double>(record.position_y_pm) -
        static_cast<long double>(source.position_y_pm);
    long double const delta_z_pm =
        static_cast<long double>(record.position_z_pm) -
        static_cast<long double>(source.position_z_pm);

    long double const local_x = (delta_x_pm * source.axis_x_x) +
                                (delta_y_pm * source.axis_x_y) +
                                (delta_z_pm * source.axis_x_z);
    long double const local_y = (delta_x_pm * source.axis_y_x) +
                                (delta_y_pm * source.axis_y_y) +
                                (delta_z_pm * source.axis_y_z);
    long double const local_z = (delta_x_pm * source.axis_z_x) +
                                (delta_y_pm * source.axis_z_y) +
                                (delta_z_pm * source.axis_z_z);

    EXPECT_NEAR(static_cast<double>(local_z), 0.0, 2.0);

    auto const geometry_size_x_pm =
        static_cast<long double>(source.geometry_size_x_pm);
    auto const geometry_size_y_pm =
        static_cast<long double>(source.geometry_size_y_pm);

    if (record.source_index == 0U) {
      EXPECT_LE(std::abs(local_x), (0.5L * geometry_size_x_pm) + 2.0L);
      EXPECT_LE(std::abs(local_y), (0.5L * geometry_size_y_pm) + 2.0L);
    } else {
      long double const normalised_x = 2.0L * local_x / geometry_size_x_pm;
      long double const normalised_y = 2.0L * local_y / geometry_size_y_pm;

      EXPECT_LE((normalised_x * normalised_x) + (normalised_y * normalised_y),
                1.00001L);
    }

    EXPECT_FLOAT_EQ(record.direction_x, source.axis_z_x);
    EXPECT_FLOAT_EQ(record.direction_y, source.axis_z_y);
    EXPECT_FLOAT_EQ(record.direction_z, source.axis_z_z);

    distinct_positions[record.source_index].emplace(
        record.position_x_pm, record.position_y_pm, record.position_z_pm);

    ExpectDiagnosticProjection(report, record);
  }

  for (auto const &positions : distinct_positions) {
    EXPECT_GT(positions.size(), 1U);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingTransportTest,
       SamplesFullSphereIsotropicDirectionsForEveryEngine) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::uint32_t k_primary_count{2'048U};
  constexpr std::array<std::uint64_t, 1U> k_counts{k_primary_count};

  for (std::string_view engine : k_engines) {
    SCOPED_TRACE(engine);

    ggems::core::sources::GGEMSSource source{};
    source.SetIsotropicAngularDistribution();

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(9'999ULL);

    TransportWorkload workload{Context(),
                               std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                               random,
                               64U,
                               *MakeMonoSourceConfiguration(1U),
                               0ULL,
                               0U,
                               2U * k_primary_count};

    auto const report =
        workload.Run(MakeConfig({source.BuildRecord()}, k_counts));
    auto const records = RecordsOfKind(report, ObserverRecordKind::Source);
    ASSERT_EQ(records.size(), k_primary_count);

    std::array<long double, 3U> sums{};
    std::array<bool, 3U> positive{};
    std::array<bool, 3U> negative{};

    for (ObserverRecord const &record : records) {
      std::array<float, 3U> const direction{
          record.direction_x, record.direction_y, record.direction_z};

      long double norm_squared{0.0L};

      for (std::size_t axis = 0U; axis < 3U; ++axis) {
        EXPECT_TRUE(std::isfinite(direction[axis]));
        EXPECT_GE(direction[axis], -1.00001F);
        EXPECT_LE(direction[axis], 1.00001F);

        norm_squared +=
            static_cast<long double>(direction[axis]) * direction[axis];
        sums[axis] += direction[axis];
        positive[axis] = positive[axis] || direction[axis] > 0.0F;
        negative[axis] = negative[axis] || direction[axis] < 0.0F;
      }

      EXPECT_NEAR(static_cast<double>(norm_squared), 1.0, 2.0e-5);
      ExpectDiagnosticProjection(report, record);
    }

    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      EXPECT_TRUE(positive[axis]);
      EXPECT_TRUE(negative[axis]);
      EXPECT_LT(std::abs(sums[axis] / k_primary_count), 0.08L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingTransportTest,
       FocusedDirectionsUseEachSampledGlobalPosition) {
  constexpr std::array<std::int64_t, 3U> k_focus{
      300'000'000'000LL, -200'000'000'000LL, 800'000'000'000LL};

  ggems::core::sources::GGEMSSource point{};
  point.SetFocusedAngularDistributionPicoMeter(k_focus[0U], k_focus[1U],
                                               k_focus[2U]);

  ggems::core::sources::GGEMSSource rectangle{};
  rectangle.SetRectangleEmissionPicoMeter(100'000'000'000ULL, 60'000'000'000ULL)
      .SetFocusedAngularDistributionPicoMeter(k_focus[0U], k_focus[1U],
                                              k_focus[2U]);

  ggems::core::sources::GGEMSSource ellipse{};
  ellipse.SetOrientation({1.0, 1.0, 1.0}, {0.0, 0.0, 1.0})
      .SetEllipseEmissionPicoMeter(80'000'000'000ULL, 40'000'000'000ULL)
      .SetFocusedAngularDistributionPicoMeter(k_focus[0U], k_focus[1U],
                                              k_focus[2U]);

  SourceRecord invalid_zero_slot = point.BuildRecord();
  invalid_zero_slot.emission_geometry_type = 99U;

  constexpr std::array<std::uint64_t, 5U> k_counts{1ULL, 128ULL, 128ULL, 0ULL,
                                                   128ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("pcg32").SetSeed(10'101ULL);

  TransportWorkload workload{Context(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(5U),
                             0ULL,
                             0U,
                             770U};

  auto const report = workload.Run(MakeConfig(
      {point.BuildRecord(), rectangle.BuildRecord(), ellipse.BuildRecord(),
       invalid_zero_slot, rectangle.BuildRecord()},
      k_counts));

  auto const records = RecordsOfKind(report, ObserverRecordKind::Source);
  ASSERT_EQ(records.size(), 385U);

  for (ObserverRecord const &record : records) {
    ASSERT_NE(record.source_index, 3U);

    long double const delta_x_pm =
        static_cast<long double>(k_focus[0U]) -
        static_cast<long double>(record.position_x_pm);
    long double const delta_y_pm =
        static_cast<long double>(k_focus[1U]) -
        static_cast<long double>(record.position_y_pm);
    long double const delta_z_pm =
        static_cast<long double>(k_focus[2U]) -
        static_cast<long double>(record.position_z_pm);
    long double const norm = std::hypot(delta_x_pm, delta_y_pm, delta_z_pm);

    long double const dot = (record.direction_x * delta_x_pm) +
                            (record.direction_y * delta_y_pm) +
                            (record.direction_z * delta_z_pm);

    EXPECT_GT(dot, 0.0L);
    EXPECT_NEAR(static_cast<double>(dot / norm), 1.0, 2.0e-5);
    EXPECT_LT(record.source_local_primary_id, k_counts[record.source_index]);
    ExpectDiagnosticProjection(report, record);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingTransportTest,
       RandomStateContinuesWithoutPerRunAllocation) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<std::uint64_t, 1U> k_counts{8ULL};

  for (std::string_view engine : k_engines) {
    SCOPED_TRACE(engine);

    ggems::core::sources::GGEMSSource source{};
    source.SetRectangleEmissionPicoMeter(40'000'000'000ULL, 20'000'000'000ULL);

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(11'111ULL);

    TransportWorkload workload{Context(),
                               std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                               random,
                               1U,
                               *MakeMonoSourceConfiguration(1U),
                               0ULL,
                               0U,
                               16U};

    auto const allocated = Context().GetAllocatedVRAM().value;
    auto const allocation_count = Context().GetAllocationCountVRAM();

    auto first = workload.Run(MakeConfig({source.BuildRecord()}, k_counts));
    auto second = workload.Run(MakeConfig({source.BuildRecord()}, k_counts));

    auto first_sources = RecordsOfKind(first, ObserverRecordKind::Source);
    auto second_sources = RecordsOfKind(second, ObserverRecordKind::Source);

    ASSERT_EQ(first_sources.size(), second_sources.size());
    EXPECT_FALSE(std::ranges::equal(
        first_sources, second_sources, {},
        [](ObserverRecord const &record)
            -> std::tuple<std::int64_t, std::int64_t, std::int64_t> {
          return std::tuple{record.position_x_pm, record.position_y_pm,
                            record.position_z_pm};
        },
        [](ObserverRecord const &record)
            -> std::tuple<std::int64_t, std::int64_t, std::int64_t> {
          return std::tuple{record.position_x_pm, record.position_y_pm,
                            record.position_z_pm};
        }));

    EXPECT_EQ(Context().GetAllocatedVRAM().value, allocated);
    EXPECT_EQ(Context().GetAllocationCountVRAM(), allocation_count);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingTransportTest,
       MixedLegacyAndVolumeSourcesPreserveObserverAndPaddingContracts) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<std::uint64_t, 8U> k_counts{1ULL, 2ULL, 2ULL, 2ULL,
                                                   2ULL, 2ULL, 0ULL, 2ULL};
  constexpr std::uint32_t k_total_primary_count{13U};

  ggems::core::sources::GGEMSSource point{};
  point.SetPointEmission().SetFixedAngularDistribution();

  ggems::core::sources::GGEMSSource rectangle{};
  rectangle.SetRectangleEmissionPicoMeter(40'000'000ULL, 20'000'000ULL)
      .SetIsotropicAngularDistribution();

  ggems::core::sources::GGEMSSource ellipse{};
  ellipse.SetEllipseEmissionPicoMeter(40'000'000ULL, 20'000'000ULL)
      .SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 1'000'000'000LL);

  ggems::core::sources::GGEMSSource box{};
  box.SetBoxEmissionPicoMeter(40'000'000ULL, 20'000'000ULL, 10'000'000ULL)
      .SetFixedAngularDistribution();

  ggems::core::sources::GGEMSSource sphere{};
  sphere.SetSphereEmissionPicoMeter(30'000'000ULL)
      .SetIsotropicAngularDistribution(
          ggems::units::MakeDegrees(10.0L), ggems::units::MakeDegrees(60.0L),
          ggems::units::MakeDegrees(-45.0L), ggems::units::MakeDegrees(45.0L));

  ggems::core::sources::GGEMSSource cylinder{};
  cylinder.SetCylinderEmissionPicoMeter(30'000'000ULL, 50'000'000ULL)
      .SetIsotropicAngularDistribution();

  SourceRecord invalid_zero_slot = point.BuildRecord();
  invalid_zero_slot.emission_geometry_type = 99U;

  std::vector<SourceRecord> const records{
      point.BuildRecord(), rectangle.BuildRecord(), ellipse.BuildRecord(),
      box.BuildRecord(),   sphere.BuildRecord(),    cylinder.BuildRecord(),
      invalid_zero_slot,   box.BuildRecord()};

  for (std::string_view const engine : k_engines) {
    SCOPED_TRACE(engine);
    ggems::core::random::GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(12'345ULL);

    TransportWorkload workload{Context(),
                               std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                               random,
                               65U,
                               *MakeMonoSourceConfiguration(records.size()),
                               0ULL,
                               0U,
                               2U * k_total_primary_count};

    auto const report = workload.Run(MakeConfig(records, k_counts));
    EXPECT_EQ(workload.GetWorkerCount(), 65U);
    EXPECT_EQ(report.counters.consumed_primary_count, k_total_primary_count);
    EXPECT_EQ(report.counters.completed_history_count, k_total_primary_count);
    EXPECT_EQ(report.counters.terminal_particle_count, k_total_primary_count);
    EXPECT_EQ(report.counters.created_secondary_count, 0U);
    EXPECT_EQ(report.counters.overflow_count, 0U);
    EXPECT_EQ(report.observer_counters.overflow_count, 0U);

    auto const source_observations =
        RecordsOfKind(report, ObserverRecordKind::Source);
    ASSERT_EQ(source_observations.size(), k_total_primary_count);

    for (ObserverRecord const &source_observation : source_observations) {
      ASSERT_LT(source_observation.source_index, k_counts.size());
      EXPECT_NE(source_observation.source_index, 6U);
      EXPECT_LT(source_observation.source_local_primary_id,
                k_counts[source_observation.source_index]);

      ObserverRecord const *terminal = FindTerminal(report, source_observation);
      ASSERT_NE(terminal, nullptr);
      EXPECT_FLOAT_EQ(terminal->direction_x, source_observation.direction_x);
      EXPECT_FLOAT_EQ(terminal->direction_y, source_observation.direction_y);
      EXPECT_FLOAT_EQ(terminal->direction_z, source_observation.direction_z);
      EXPECT_EQ(terminal->energy_milli_eV, source_observation.energy_milli_eV);
      EXPECT_EQ(terminal->source_index, source_observation.source_index);
      EXPECT_EQ(terminal->source_local_primary_id,
                source_observation.source_local_primary_id);
      ExpectDiagnosticProjection(report, source_observation);
    }
  }
}
