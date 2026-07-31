#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmissionPlan.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace {

// =============================================================================
// =============================================================================

using Definition = ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using Emission = ggems::core::radioactivity::GGEMSRadionuclideEmission;
using EnergyDistribution = ggems::core::sources::GGEMSEnergyDistribution;
using EnergyDistributionRecord =
    ggems::core::sources::GGEMSEnergyDistributionRecord;
using EnergyDistributionType =
    ggems::core::sources::GGEMSEnergyDistributionType;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using ObserverRecordKind = ggems::core::observer::GGEMSObserverRecordKind;
using ParticleType = ggems::core::particles::GGEMSParticleType;
using Plan = ggems::core::radioactivity::GGEMSRadionuclideEmissionPlan;
using Planner = ggems::core::radioactivity::GGEMSRadionuclideEmissionPlanner;
using Random = ggems::core::random::GGEMSRandom;
using Source = ggems::core::sources::GGEMSSource;
using SourceConfigurationSnapshotPtr =
    ggems::core::sources::GGEMSSourceConfigurationSnapshotPtr;
using SourcePtr = std::shared_ptr<Source>;
using SourceRunSnapshot = ggems::core::sources::GGEMSSourceRunSnapshot;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportRunReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;

constexpr std::uint32_t k_worker_count{64U};
constexpr std::uint64_t k_run_id{37ULL};
constexpr std::uint64_t k_projection_history_offset{12'000ULL};
constexpr std::uint32_t k_regular_capture_count{128U};
constexpr ggems::core::GGEMSTimeWindow k_time_window{
    .start_ps = 1'000'000'000'000ULL, .stop_ps = 1'001'000'000'000ULL};

// =============================================================================
// =============================================================================

struct ActivityScenario {
  std::vector<SourcePtr> sources;
  SourceConfigurationSnapshotPtr source_configuration;
  Plan emission_plan;
  SourceRunSnapshot source_snapshot;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRandom(std::uint64_t seed) -> Random {
  Random random{};
  random.SetEngine("philox").SetSeed(seed);
  return random;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeActivityScenario(Definition definition,
                                        ggems::units::Activity activity,
                                        Random const &random)
    -> ActivityScenario {
  auto source = std::make_shared<Source>();
  source->SetPointEmission()
      .SetFixedAngularDistribution()
      .SetPositionPicoMeter(0LL, 0LL, 0LL)
      .SetDirection(0.0, 0.0, 1.0)
      .SetActivityDrivenRadionuclide(
          std::make_shared<Definition const>(std::move(definition)), activity,
          k_time_window.start_ps);

  std::vector<SourcePtr> sources{std::move(source)};
  auto source_configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  Planner planner{sources, random};
  auto candidate = planner.BuildCandidate(k_time_window);
  auto source_snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, source_configuration, candidate.GetPlan());

  return {.sources = std::move(sources),
          .source_configuration = std::move(source_configuration),
          .emission_plan = candidate.GetPlan(),
          .source_snapshot = std::move(source_snapshot)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRunConfig(ActivityScenario const &scenario)
    -> TransportRunConfig {
  TransportRunConfig config{};
  config.run_id = k_run_id;
  config.total_primary_count = scenario.source_snapshot.GetTotalPrimaryCount();
  config.projection_history_offset = k_projection_history_offset;
  config.device_primary_offset = 0ULL;
  config.source_records = scenario.source_snapshot.GetRecords();
  config.source_population_records =
      scenario.source_snapshot.GetPopulationRecords();
  config.source_ranges = scenario.source_snapshot.GetRanges();
  config.radionuclide_group_ranges = scenario.source_snapshot.GetGroupRanges();
  config.observer_config.enabled = 1U;
  return config;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetSourceRecords(TransportRunReport const &report)
    -> std::vector<ObserverRecord const *> {
  std::vector<ObserverRecord const *> records;

  for (ObserverRecord const &record : report.observer_records) {
    if (ggems::core::observer::FromKernelObserverRecordKind(
            record.record_kind) == ObserverRecordKind::Source) {
      records.push_back(&record);
    }
  }

  return records;
}

// =============================================================================
// =============================================================================

auto ExpectTransportReport(TransportRunReport const &report,
                           std::uint64_t expected_primary_count,
                           std::uint32_t expected_captured_primary_count)
    -> void {
  EXPECT_EQ(report.counters.consumed_primary_count, expected_primary_count);
  EXPECT_EQ(report.counters.completed_history_count, expected_primary_count);
  EXPECT_EQ(report.counters.terminal_particle_count, expected_primary_count);
  EXPECT_EQ(report.counters.created_secondary_count, 0ULL);
  EXPECT_EQ(report.counters.aionino_to_gamma_count, 0ULL);
  EXPECT_EQ(report.counters.gamma_to_electron_count, 0ULL);
  EXPECT_EQ(report.counters.electron_to_electron_count, 0ULL);
  EXPECT_EQ(report.counters.overflow_count, 0ULL);
  EXPECT_EQ(report.counters.max_stack_depth, 0ULL);
  EXPECT_EQ(report.counters.total_fake_step_count, 0ULL);

  auto const expected_record_count =
      static_cast<std::uint64_t>(expected_captured_primary_count) * 2ULL;
  EXPECT_EQ(report.logical_observer_counters.captured_primary_count,
            expected_captured_primary_count);
  EXPECT_EQ(report.logical_observer_counters.record_count,
            expected_record_count);
  EXPECT_EQ(report.logical_observer_counters.overflow_count, 0ULL);
  EXPECT_EQ(report.observer_records.size(), expected_record_count);
}

// =============================================================================
// =============================================================================

auto ExpectCommonActivityRecord(ObserverRecord const &record) -> void {
  EXPECT_EQ(record.run_id, k_run_id);
  EXPECT_EQ(record.global_primary_id,
            k_projection_history_offset + record.source_local_primary_id);
  EXPECT_EQ(record.global_particle_id, record.global_primary_id);
  EXPECT_EQ(record.source_index, 0U);
  EXPECT_GE(record.time_ps, k_time_window.start_ps);
  EXPECT_LT(record.time_ps, k_time_window.stop_ps);
}

// =============================================================================
// =============================================================================

auto ExpectNoUnsupportedF18Records(TransportRunReport const &report) -> void {
  for (ObserverRecord const &record : report.observer_records) {
    auto const particle_type =
        ggems::core::particles::FromKernelParticleType(record.particle_type);

    EXPECT_NE(particle_type, ParticleType::Aionino);
    EXPECT_FALSE(particle_type == ParticleType::Gamma &&
                 record.energy_milli_eV == 511'000'000ULL);
    EXPECT_FALSE(particle_type == ParticleType::Electron &&
                 record.energy_milli_eV >= 456'000ULL &&
                 record.energy_milli_eV <= 502'000ULL);
  }
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetEmissionEnergyRecord(ActivityScenario const &scenario,
                                           std::size_t emission_index)
    -> EnergyDistributionRecord const & {
  auto const &emission_records =
      scenario.source_configuration->GetRadionuclideEmissionRecords();
  auto const &energy_records =
      scenario.source_configuration->GetEnergyDistributionRecords();

  return energy_records.at(
      emission_records.at(emission_index).energy_distribution_record_index);
}

// =============================================================================
// =============================================================================

auto ExpectRegularSpectrumSamples(
    ActivityScenario const &scenario, std::uint64_t expected_upper_edge,
    std::uint32_t expected_table_count,
    std::vector<ObserverRecord const *> const &source_records) -> void {
  auto const &record = GetEmissionEnergyRecord(scenario, 0U);
  auto const &energy_values =
      scenario.source_configuration->GetEnergyValuesMilliElectronVolt();

  ASSERT_EQ(ggems::core::sources::FromKernelEnergyDistributionType(
                record.distribution_type),
            EnergyDistributionType::RegularSpectrum);
  ASSERT_EQ(record.table_count, expected_table_count);
  ASSERT_GT(record.regular_bin_width_milli_eV, 0ULL);
  ASSERT_LE(record.table_offset,
            static_cast<std::uint64_t>(energy_values.size()));
  ASSERT_LE(static_cast<std::uint64_t>(record.table_count),
            static_cast<std::uint64_t>(energy_values.size()) -
                record.table_offset);

  auto const table_offset = static_cast<std::size_t>(record.table_offset);
  std::uint64_t const half_width = record.regular_bin_width_milli_eV / 2ULL;
  std::uint64_t const lower_edge = energy_values[table_offset] - half_width;
  std::uint64_t const upper_edge =
      energy_values[table_offset + record.table_count - 1U] + half_width;
  ASSERT_EQ(upper_edge, expected_upper_edge);

  for (ObserverRecord const *source_record : source_records) {
    ASSERT_NE(source_record, nullptr);
    ExpectCommonActivityRecord(*source_record);
    EXPECT_EQ(ggems::core::particles::FromKernelParticleType(
                  source_record->particle_type),
              ParticleType::Positron);
    EXPECT_GE(source_record->energy_milli_eV, lower_edge);
    EXPECT_LT(source_record->energy_milli_eV, upper_edge);

    std::uint64_t const sampled_bin =
        (source_record->energy_milli_eV - lower_edge) /
        record.regular_bin_width_milli_eV;
    ASSERT_LT(sampled_bin, record.table_count);
    std::uint64_t const selected_center =
        energy_values[table_offset + static_cast<std::size_t>(sampled_bin)];
    std::uint64_t const selected_lower_edge = selected_center - half_width;
    EXPECT_GE(source_record->energy_milli_eV, selected_lower_edge);
    EXPECT_LT(source_record->energy_milli_eV,
              selected_lower_edge + record.regular_bin_width_milli_eV);
  }
}

// =============================================================================
// =============================================================================

class GGEMSBuiltInRadionuclideTransportTest : public ::testing::Test {
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

TEST_F(GGEMSBuiltInRadionuclideTransportTest,
       F18PreservesAllThreeDeviceSignaturesWithoutPlaceholders) {
  constexpr std::uint64_t k_f18_endpoint_milli_eV{633'900'000ULL};
  constexpr std::uint32_t k_f18_bin_count{1'268U};
  constexpr std::uint64_t k_electron_energy_milli_eV{14'300ULL};
  constexpr std::uint64_t k_gamma_energy_milli_eV{525'000ULL};
  constexpr std::uint64_t k_annihilation_energy_milli_eV{511'000'000ULL};
  constexpr std::uint32_t k_observer_capacity{4U};

  Random const random = MakeRandom(0xF18B32ULL);
  ActivityScenario const scenario = MakeActivityScenario(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide(),
      ggems::units::Activity{100'000'000.0L}, random);

  auto const &emission_records =
      scenario.source_configuration->GetRadionuclideEmissionRecords();
  auto const plan_groups = scenario.emission_plan.GetGroups();
  auto const &group_ranges = scenario.source_snapshot.GetGroupRanges();
  ASSERT_EQ(emission_records.size(), 3U);
  ASSERT_EQ(plan_groups.size(), 3U);
  ASSERT_EQ(group_ranges.size(), 3U);
  ASSERT_GT(group_ranges[0U].primary_count, 0ULL);
  ASSERT_GT(group_ranges[1U].primary_count, 0ULL);
  ASSERT_GT(group_ranges[2U].primary_count, 0ULL);
  constexpr std::array<long double, 3U> k_expected_yields{0.9686L, 0.00229L,
                                                          0.00020L};
  for (std::size_t index = 0U; index < plan_groups.size(); ++index) {
    EXPECT_EQ(plan_groups[index].yield_per_decay, k_expected_yields[index]);
    EXPECT_EQ(group_ranges[index].source_local_primary_begin,
              plan_groups[index].source_local_primary_begin);
    EXPECT_EQ(group_ranges[index].primary_count,
              plan_groups[index].sampled_primary_count);
  }

  EXPECT_EQ(ggems::core::particles::FromKernelParticleType(
                emission_records[0U].particle_type),
            ParticleType::Positron);
  EXPECT_EQ(ggems::core::particles::FromKernelParticleType(
                emission_records[1U].particle_type),
            ParticleType::Electron);
  EXPECT_EQ(ggems::core::particles::FromKernelParticleType(
                emission_records[2U].particle_type),
            ParticleType::Gamma);
  EXPECT_EQ(emission_records[0U].mono_energy_milli_eV, 0ULL);
  EXPECT_EQ(emission_records[1U].mono_energy_milli_eV,
            k_electron_energy_milli_eV);
  EXPECT_EQ(emission_records[2U].mono_energy_milli_eV, k_gamma_energy_milli_eV);

  auto const &positron_energy = GetEmissionEnergyRecord(scenario, 0U);
  EXPECT_EQ(ggems::core::sources::FromKernelEnergyDistributionType(
                positron_energy.distribution_type),
            EnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(positron_energy.table_count, k_f18_bin_count);

  std::size_t oxygen_x_ray_count{0U};
  for (auto const &emission_record : emission_records) {
    auto const particle_type = ggems::core::particles::FromKernelParticleType(
        emission_record.particle_type);
    EXPECT_NE(particle_type, ParticleType::Aionino);
    EXPECT_FALSE(particle_type == ParticleType::Gamma &&
                 emission_record.mono_energy_milli_eV ==
                     k_annihilation_energy_milli_eV);
    EXPECT_FALSE(particle_type == ParticleType::Electron &&
                 emission_record.mono_energy_milli_eV >= 456'000ULL &&
                 emission_record.mono_energy_milli_eV <= 502'000ULL);

    if (particle_type == ParticleType::Gamma &&
        emission_record.mono_energy_milli_eV == k_gamma_energy_milli_eV) {
      ++oxygen_x_ray_count;
    }
  }
  EXPECT_EQ(oxygen_x_ray_count, 1U);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             k_worker_count,
                             *scenario.source_configuration,
                             0ULL,
                             0U,
                             k_observer_capacity};

  auto positron_and_gamma_config = MakeRunConfig(scenario);
  positron_and_gamma_config.observer_config
      .capture_first_primary_count_per_source = 1U;
  positron_and_gamma_config.observer_config.capture_specific_primary_enabled =
      1U;
  positron_and_gamma_config.observer_config.capture_source_index = 0U;
  positron_and_gamma_config.observer_config.capture_source_local_primary_id =
      group_ranges[2U].source_local_primary_begin;

  auto const positron_and_gamma_report =
      workload.Run(positron_and_gamma_config);
  ExpectTransportReport(positron_and_gamma_report,
                        scenario.source_snapshot.GetTotalPrimaryCount(), 2U);
  ExpectNoUnsupportedF18Records(positron_and_gamma_report);
  auto const positron_and_gamma_records =
      GetSourceRecords(positron_and_gamma_report);
  ASSERT_EQ(positron_and_gamma_records.size(), 2U);

  ObserverRecord const *positron_record{nullptr};
  ObserverRecord const *gamma_record{nullptr};
  for (ObserverRecord const *record : positron_and_gamma_records) {
    ASSERT_NE(record, nullptr);
    ExpectCommonActivityRecord(*record);
    auto const particle_type =
        ggems::core::particles::FromKernelParticleType(record->particle_type);
    EXPECT_NE(particle_type, ParticleType::Aionino);

    if (record->source_local_primary_id ==
        group_ranges[0U].source_local_primary_begin) {
      positron_record = record;
    } else if (record->source_local_primary_id ==
               group_ranges[2U].source_local_primary_begin) {
      gamma_record = record;
    }
  }

  ASSERT_NE(positron_record, nullptr);
  ASSERT_NE(gamma_record, nullptr);
  EXPECT_EQ(ggems::core::particles::FromKernelParticleType(
                gamma_record->particle_type),
            ParticleType::Gamma);
  EXPECT_EQ(gamma_record->energy_milli_eV, k_gamma_energy_milli_eV);
  ExpectRegularSpectrumSamples(scenario, k_f18_endpoint_milli_eV,
                               k_f18_bin_count, {positron_record});

  auto electron_config = MakeRunConfig(scenario);
  electron_config.observer_config.capture_specific_primary_enabled = 1U;
  electron_config.observer_config.capture_source_index = 0U;
  electron_config.observer_config.capture_source_local_primary_id =
      group_ranges[1U].source_local_primary_begin;

  auto const electron_report = workload.Run(electron_config);
  ExpectTransportReport(electron_report,
                        scenario.source_snapshot.GetTotalPrimaryCount(), 1U);
  ExpectNoUnsupportedF18Records(electron_report);
  auto const electron_records = GetSourceRecords(electron_report);
  ASSERT_EQ(electron_records.size(), 1U);
  ExpectCommonActivityRecord(*electron_records[0U]);
  EXPECT_EQ(ggems::core::particles::FromKernelParticleType(
                electron_records[0U]->particle_type),
            ParticleType::Electron);
  EXPECT_EQ(electron_records[0U]->energy_milli_eV, k_electron_energy_milli_eV);
  EXPECT_FALSE(electron_records[0U]->energy_milli_eV >= 456'000ULL &&
               electron_records[0U]->energy_milli_eV <= 502'000ULL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSBuiltInRadionuclideTransportTest,
       C11SamplesItsSinglePositronGridOnDevice) {
  constexpr std::uint64_t k_endpoint_milli_eV{960'500'000ULL};
  constexpr std::uint32_t k_bin_count{1'921U};
  constexpr std::uint32_t k_observer_capacity{k_regular_capture_count * 2U};

  Random const random = MakeRandom(0xC11B32ULL);
  auto definition =
      ggems::core::radioactivity::builtins::BuildC11Radionuclide();
  ASSERT_EQ(definition.GetEmissions().size(), 1U);
  EXPECT_EQ(definition.GetEmissions()[0U].GetYieldPerDecay(), 0.99750L);
  ActivityScenario const scenario = MakeActivityScenario(
      std::move(definition), ggems::units::Activity{1'024'000.0L}, random);

  ASSERT_EQ(
      scenario.source_configuration->GetRadionuclideEmissionRecords().size(),
      1U);
  ASSERT_EQ(scenario.source_snapshot.GetGroupRanges().size(), 1U);
  ASSERT_EQ(scenario.emission_plan.GetGroups().size(), 1U);
  EXPECT_EQ(scenario.emission_plan.GetGroups()[0U].yield_per_decay, 0.99750L);
  EXPECT_EQ(scenario.source_snapshot.GetGroupRanges()[0U].primary_count,
            scenario.emission_plan.GetGroups()[0U].sampled_primary_count);
  ASSERT_GT(scenario.source_snapshot.GetGroupRanges()[0U].primary_count,
            k_regular_capture_count);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             k_worker_count,
                             *scenario.source_configuration,
                             0ULL,
                             0U,
                             k_observer_capacity};
  auto config = MakeRunConfig(scenario);
  config.observer_config.capture_first_primary_count_per_source =
      k_regular_capture_count;

  auto const report = workload.Run(config);
  ExpectTransportReport(report, scenario.source_snapshot.GetTotalPrimaryCount(),
                        k_regular_capture_count);
  auto const source_records = GetSourceRecords(report);
  ASSERT_EQ(source_records.size(), k_regular_capture_count);
  ExpectRegularSpectrumSamples(scenario, k_endpoint_milli_eV, k_bin_count,
                               source_records);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSBuiltInRadionuclideTransportTest,
       O15SamplesItsSinglePositronGridOnDevice) {
  constexpr std::uint64_t k_endpoint_milli_eV{1'732'180'000ULL};
  constexpr std::uint32_t k_bin_count{3'465U};
  constexpr std::uint32_t k_observer_capacity{k_regular_capture_count * 2U};

  Random const random = MakeRandom(0x015B32ULL);
  auto definition =
      ggems::core::radioactivity::builtins::BuildO15Radionuclide();
  ASSERT_EQ(definition.GetEmissions().size(), 1U);
  EXPECT_EQ(definition.GetEmissions()[0U].GetYieldPerDecay(), 0.999001L);
  ActivityScenario const scenario = MakeActivityScenario(
      std::move(definition), ggems::units::Activity{1'024'000.0L}, random);

  ASSERT_EQ(
      scenario.source_configuration->GetRadionuclideEmissionRecords().size(),
      1U);
  ASSERT_EQ(scenario.source_snapshot.GetGroupRanges().size(), 1U);
  ASSERT_EQ(scenario.emission_plan.GetGroups().size(), 1U);
  EXPECT_EQ(scenario.emission_plan.GetGroups()[0U].yield_per_decay, 0.999001L);
  EXPECT_EQ(scenario.source_snapshot.GetGroupRanges()[0U].primary_count,
            scenario.emission_plan.GetGroups()[0U].sampled_primary_count);
  ASSERT_GT(scenario.source_snapshot.GetGroupRanges()[0U].primary_count,
            k_regular_capture_count);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             k_worker_count,
                             *scenario.source_configuration,
                             0ULL,
                             0U,
                             k_observer_capacity};
  auto config = MakeRunConfig(scenario);
  config.observer_config.capture_first_primary_count_per_source =
      k_regular_capture_count;

  auto const report = workload.Run(config);
  ExpectTransportReport(report, scenario.source_snapshot.GetTotalPrimaryCount(),
                        k_regular_capture_count);
  auto const source_records = GetSourceRecords(report);
  ASSERT_EQ(source_records.size(), k_regular_capture_count);
  ExpectRegularSpectrumSamples(scenario, k_endpoint_milli_eV, k_bin_count,
                               source_records);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSBuiltInRadionuclideTransportTest,
       SyntheticDiscreteLinesRemainExactOnDevice) {
  constexpr std::array<double, 3U> k_line_energies_keV{10.0, 20.0, 40.0};
  constexpr std::array<double, 3U> k_line_weights{1.0, 1.0, 1.0};
  constexpr std::array<std::uint64_t, 3U> k_line_energies_milli_eV{
      10'000'000ULL, 20'000'000ULL, 40'000'000ULL};
  constexpr std::uint32_t k_capture_count{256U};
  constexpr std::uint32_t k_observer_capacity{k_capture_count * 2U};

  std::vector<Emission> emissions;
  emissions.emplace_back(ParticleType::Gamma, 1.0L,
                         EnergyDistribution::BuildDiscreteLines(
                             k_line_energies_keV, k_line_weights, "keV"));
  Definition definition{
      "Synthetic-DiscreteLines", {}, 1'000.0L, std::move(emissions)};

  Random const random = MakeRandom(0xD15C32ULL);
  ActivityScenario const scenario = MakeActivityScenario(
      std::move(definition), ggems::units::Activity{1'024'000.0L}, random);

  ASSERT_EQ(
      scenario.source_configuration->GetRadionuclideEmissionRecords().size(),
      1U);
  ASSERT_EQ(scenario.source_snapshot.GetGroupRanges().size(), 1U);
  ASSERT_GT(scenario.source_snapshot.GetGroupRanges()[0U].primary_count,
            k_capture_count);

  auto const &energy_record = GetEmissionEnergyRecord(scenario, 0U);
  EXPECT_EQ(ggems::core::sources::FromKernelEnergyDistributionType(
                energy_record.distribution_type),
            EnergyDistributionType::DiscreteLines);
  EXPECT_EQ(energy_record.table_count, k_line_energies_milli_eV.size());

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             k_worker_count,
                             *scenario.source_configuration,
                             0ULL,
                             0U,
                             k_observer_capacity};
  auto config = MakeRunConfig(scenario);
  config.observer_config.capture_first_primary_count_per_source =
      k_capture_count;

  auto const report = workload.Run(config);
  ExpectTransportReport(report, scenario.source_snapshot.GetTotalPrimaryCount(),
                        k_capture_count);
  auto const source_records = GetSourceRecords(report);
  ASSERT_EQ(source_records.size(), k_capture_count);

  std::array<bool, k_line_energies_milli_eV.size()> seen_lines{};
  for (ObserverRecord const *record : source_records) {
    ASSERT_NE(record, nullptr);
    ExpectCommonActivityRecord(*record);
    EXPECT_EQ(
        ggems::core::particles::FromKernelParticleType(record->particle_type),
        ParticleType::Gamma);

    auto const line =
        std::ranges::find(k_line_energies_milli_eV, record->energy_milli_eV);
    ASSERT_NE(line, k_line_energies_milli_eV.end());
    seen_lines[static_cast<std::size_t>(
        line - k_line_energies_milli_eV.begin())] = true;
  }

  EXPECT_TRUE(std::ranges::all_of(seen_lines, [](bool seen) { return seen; }));
}
