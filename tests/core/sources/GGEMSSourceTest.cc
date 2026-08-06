#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <optional>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"

// =============================================================================
// =============================================================================

template <typename T>
concept HasMutableTimeWindow = requires(T value) {
  value.SetTimeWindowPicoSecond(std::uint64_t{0ULL}, std::uint64_t{1ULL});
};

static_assert(!HasMutableTimeWindow<ggems::core::sources::GGEMSSource>);

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeTestRadionuclide(std::string canonical_name = "Test")
    -> std::shared_ptr<
        ggems::core::radioactivity::GGEMSRadionuclideDefinition const> {
  std::vector<ggems::core::radioactivity::GGEMSRadionuclideEmission> emissions;
  emissions.emplace_back(
      ggems::core::particles::GGEMSParticleType::Gamma, 1.0L,
      ggems::core::sources::GGEMSEnergyDistribution::BuildMono(1'000ULL));

  return std::make_shared<
      ggems::core::radioactivity::GGEMSRadionuclideDefinition const>(
      std::move(canonical_name), std::vector<std::string>{}, 10.0L,
      std::move(emissions));
}

// =============================================================================
// =============================================================================

namespace {

constexpr std::string_view k_population_finalized_diagnostic{
    "Cannot change GGEMSSource population mode after source initialization has "
    "been finalized."};

constexpr std::string_view k_energy_finalized_diagnostic{
    "Cannot change GGEMSSource energy after source initialization has been "
    "finalized."};

constexpr std::string_view k_activity_single_particle_diagnostic{
    "Cannot use single-particle configuration on an ActivityDriven "
    "GGEMSSource."};

template <typename Function>
auto ExpectGGEMSExceptionContaining(Function &&function,
                                    std::string_view expected) -> void {
  bool caught = false;

  try {
    std::forward<Function>(function)();
  } catch (ggems::core::GGEMSExceptionBase const &exception) {
    caught = true;
    EXPECT_NE(std::string_view{exception.what()}.find(expected),
              std::string_view::npos);
  }

  EXPECT_TRUE(caught);
}

} // namespace

// =============================================================================
// =============================================================================

auto ExpectSourceFramesEqual(
    ggems::core::sources::GGEMSSourceRecord const &actual,
    ggems::core::sources::GGEMSSourceRecord const &expected) -> void {
  EXPECT_FLOAT_EQ(actual.axis_x_x, expected.axis_x_x);
  EXPECT_FLOAT_EQ(actual.axis_x_y, expected.axis_x_y);
  EXPECT_FLOAT_EQ(actual.axis_x_z, expected.axis_x_z);

  EXPECT_FLOAT_EQ(actual.axis_y_x, expected.axis_y_x);
  EXPECT_FLOAT_EQ(actual.axis_y_y, expected.axis_y_y);
  EXPECT_FLOAT_EQ(actual.axis_y_z, expected.axis_y_z);

  EXPECT_FLOAT_EQ(actual.axis_z_x, expected.axis_z_x);
  EXPECT_FLOAT_EQ(actual.axis_z_y, expected.axis_z_y);
  EXPECT_FLOAT_EQ(actual.axis_z_z, expected.axis_z_z);
}

// =============================================================================
// =============================================================================

auto ExpectSourceRecordsEqual(
    ggems::core::sources::GGEMSSourceRecord const &actual,
    ggems::core::sources::GGEMSSourceRecord const &expected) -> void {
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
  EXPECT_EQ(actual.geometry_size_z_pm, expected.geometry_size_z_pm);
  EXPECT_EQ(actual.focus_position_x_pm, expected.focus_position_x_pm);
  EXPECT_EQ(actual.focus_position_y_pm, expected.focus_position_y_pm);
  EXPECT_EQ(actual.focus_position_z_pm, expected.focus_position_z_pm);
  EXPECT_FLOAT_EQ(actual.isotropic_cos_theta_lower,
                  expected.isotropic_cos_theta_lower);
  EXPECT_FLOAT_EQ(actual.isotropic_cos_theta_upper,
                  expected.isotropic_cos_theta_upper);
  EXPECT_FLOAT_EQ(actual.isotropic_phi_min_rad, expected.isotropic_phi_min_rad);
  EXPECT_FLOAT_EQ(actual.isotropic_phi_max_rad, expected.isotropic_phi_max_rad);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, DefaultSourceIsAnalyticGammaPointSource) {
  ggems::core::sources::GGEMSSource source{};

  auto const &record = source.BuildRecord();

  EXPECT_EQ(record.source_type,
            ggems::core::sources::ToKernelSourceType(
                ggems::core::sources::GGEMSSourceType::Analytic));

  EXPECT_EQ(record.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Gamma));

  EXPECT_EQ(record.energy_milli_eV, 511'000'000ULL);

  EXPECT_EQ(record.time_start_ps, 0ULL);
  EXPECT_EQ(record.time_stop_ps, 0ULL);

  EXPECT_EQ(record.position_x_pm, 0LL);
  EXPECT_EQ(record.position_y_pm, 0LL);
  EXPECT_EQ(record.position_z_pm, 0LL);

  EXPECT_FLOAT_EQ(record.axis_x_x, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_x_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_y, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_y_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 1.0F);
  EXPECT_FLOAT_EQ(record.weight, 1.0F);
  EXPECT_EQ(record.emission_geometry_type,
            ggems::core::sources::ToKernelEmissionGeometryType(
                ggems::core::sources::GGEMSEmissionGeometryType::Point));
  EXPECT_EQ(record.angular_distribution_type,
            ggems::core::sources::ToKernelAngularDistributionType(
                ggems::core::sources::GGEMSAngularDistributionType::Fixed));
  EXPECT_EQ(record.geometry_size_x_pm, 0ULL);
  EXPECT_EQ(record.geometry_size_y_pm, 0ULL);
  EXPECT_EQ(record.geometry_size_z_pm, 0ULL);
  EXPECT_EQ(record.focus_position_x_pm, 0LL);
  EXPECT_EQ(record.focus_position_y_pm, 0LL);
  EXPECT_EQ(record.focus_position_z_pm, 0LL);
  EXPECT_FLOAT_EQ(record.isotropic_cos_theta_lower, -1.0F);
  EXPECT_FLOAT_EQ(record.isotropic_cos_theta_upper, 1.0F);
  EXPECT_FLOAT_EQ(record.isotropic_phi_min_rad, 0.0F);
  EXPECT_FLOAT_EQ(record.isotropic_phi_max_rad,
                  ggems::core::sources::k_isotropic_full_sphere_phi_max_rad);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PrimaryCountDefaultTo4096) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_EQ(source.GetPrimaryCount(), 4096ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PrimaryCountIsFluentAndAcceptsWholeUint64Range) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_EQ(&source.SetPrimaryCount(17ULL), &source);
  EXPECT_EQ(source.GetPrimaryCount(), 17ULL);

  EXPECT_EQ(&source.SetPrimaryCount(0ULL), &source);
  EXPECT_EQ(source.GetPrimaryCount(), 0ULL);

  EXPECT_EQ(&source.SetPrimaryCount(std::numeric_limits<std::uint64_t>::max()),
            &source);
  EXPECT_EQ(source.GetPrimaryCount(),
            std::numeric_limits<std::uint64_t>::max());
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PrimaryCountDoesNotAlterSourceRecord) {
  ggems::core::sources::GGEMSSource source{};

  auto record_before = source.BuildRecord();

  source.SetPrimaryCount(17ULL);

  auto record_after = source.BuildRecord();

  ExpectSourceRecordsEqual(record_after, record_before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, DefaultPopulationIsCountDriven) {
  using ggems::core::sources::GGEMSSourcePopulationMode;

  ggems::core::sources::GGEMSSource source{};

  EXPECT_EQ(source.GetPopulationMode(), GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source.GetPrimaryCount(), 4096ULL);
  EXPECT_THROW((void)source.GetActivityDrivenConfiguration(),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, BuildsOwnedActivityDrivenPopulationConfiguration) {
  using Configuration =
      ggems::core::sources::GGEMSActivityDrivenSourceConfiguration;
  using Definition = ggems::core::radioactivity::GGEMSRadionuclideDefinition;
  using Source = ggems::core::sources::GGEMSSource;

  constexpr long double k_activity_bq{12.5L};
  constexpr std::uint64_t k_reference_time_ps{37ULL};

  Source count_driven_source{};
  ExpectGGEMSExceptionContaining(
      [&count_driven_source]() -> void {
        (void)count_driven_source.BuildActivityDrivenPopulationConfiguration();
      },
      "GGEMSSource is not configured in ActivityDriven mode.");

  auto radionuclide = MakeTestRadionuclide();
  auto replacement = MakeTestRadionuclide("Replacement");
  std::weak_ptr<Definition const> retained = radionuclide;
  auto const *definition_address = radionuclide.get();
  Configuration owned_configuration{};

  {
    Source source{};
    source.SetActivityDrivenRadionuclide(radionuclide,
                                         ggems::units::Activity{k_activity_bq},
                                         k_reference_time_ps);

    owned_configuration = source.BuildActivityDrivenPopulationConfiguration();
    EXPECT_EQ(owned_configuration.radionuclide.get(), definition_address);
    EXPECT_EQ(owned_configuration.activity_at_reference_time.value,
              k_activity_bq);
    EXPECT_EQ(owned_configuration.reference_time_ps, k_reference_time_ps);

    {
      auto editable_configuration = owned_configuration;
      editable_configuration.radionuclide = replacement;
      editable_configuration.activity_at_reference_time =
          ggems::units::Activity{99.0L};
      editable_configuration.reference_time_ps = 100ULL;

      auto const current_configuration =
          source.BuildActivityDrivenPopulationConfiguration();
      EXPECT_EQ(current_configuration.radionuclide.get(), definition_address);
      EXPECT_EQ(current_configuration.activity_at_reference_time.value,
                k_activity_bq);
      EXPECT_EQ(current_configuration.reference_time_ps, k_reference_time_ps);
    }

    ExpectGGEMSExceptionContaining(
        [&source, &replacement]() -> void {
          source.SetActivityDrivenRadionuclide(
              replacement, ggems::units::Activity{-1.0L}, 99ULL);
        },
        "ActivityDriven GGEMSSource activity must be non-negative.");
    EXPECT_EQ(owned_configuration.radionuclide.get(), definition_address);
    EXPECT_EQ(owned_configuration.activity_at_reference_time.value,
              k_activity_bq);
    EXPECT_EQ(owned_configuration.reference_time_ps, k_reference_time_ps);

    EXPECT_NO_THROW(source.SetActivityDrivenRadionuclide(
        replacement, ggems::units::Activity{1.0L}, 0ULL));
    radionuclide.reset();
    EXPECT_FALSE(retained.expired());
  }

  EXPECT_FALSE(retained.expired());
  EXPECT_EQ(owned_configuration.radionuclide.get(), definition_address);
  EXPECT_EQ(owned_configuration.activity_at_reference_time.value,
            k_activity_bq);
  EXPECT_EQ(owned_configuration.reference_time_ps, k_reference_time_ps);

  owned_configuration.radionuclide.reset();
  EXPECT_TRUE(retained.expired());
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, CountDrivenPopulationAcceptsRunInitializationTimeDomains) {
  using ggems::core::GGEMSTimeWindow;
  using ggems::core::sources::GGEMSSourcePopulationMode;

  ggems::core::sources::GGEMSSource source{};
  source.SetPrimaryCount(17ULL);
  GGEMSTimeWindow const initial_time_window{.start_ps = 30ULL,
                                            .stop_ps = 40ULL};

  EXPECT_NO_THROW(source.ValidatePopulationForRunInitialization(std::nullopt));
  EXPECT_EQ(source.GetPopulationMode(), GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source.GetPrimaryCount(), 17ULL);

  EXPECT_NO_THROW(
      source.ValidatePopulationForRunInitialization(initial_time_window));
  EXPECT_EQ(source.GetPopulationMode(), GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source.GetPrimaryCount(), 17ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource,
     ActivityDrivenPopulationValidatesRunInitializationTimeDomain) {
  using ggems::core::GGEMSTimeWindow;
  using ggems::core::sources::GGEMSSourcePopulationMode;

  constexpr long double k_activity_bq{12.5L};
  constexpr std::uint64_t k_reference_before_start_ps{20ULL};
  constexpr std::uint64_t k_window_start_ps{30ULL};
  constexpr std::uint64_t k_window_stop_ps{40ULL};
  constexpr std::uint64_t k_reference_after_start_ps{31ULL};

  auto radionuclide = MakeTestRadionuclide();
  ggems::core::sources::GGEMSSource source{};
  source.SetActivityDrivenRadionuclide(radionuclide,
                                       ggems::units::Activity{k_activity_bq},
                                       k_reference_before_start_ps);
  GGEMSTimeWindow const empty_window{.start_ps = k_window_start_ps,
                                     .stop_ps = k_window_start_ps};
  GGEMSTimeWindow const initial_time_window{.start_ps = k_window_start_ps,
                                            .stop_ps = k_window_stop_ps};

  auto expect_configuration =
      [&](std::uint64_t expected_reference_time_ps) -> void {
    EXPECT_EQ(source.GetPopulationMode(),
              GGEMSSourcePopulationMode::ActivityDriven);
    auto const &configuration = source.GetActivityDrivenConfiguration();
    EXPECT_EQ(configuration.radionuclide, radionuclide);
    EXPECT_EQ(configuration.activity_at_reference_time.value, k_activity_bq);
    EXPECT_EQ(configuration.reference_time_ps, expected_reference_time_ps);
  };

  ExpectGGEMSExceptionContaining(
      [&source]() -> void {
        source.ValidatePopulationForRunInitialization(std::nullopt);
      },
      "ActivityDriven GGEMSRun sources require a configured non-empty time "
      "schedule");
  expect_configuration(k_reference_before_start_ps);

  ExpectGGEMSExceptionContaining(
      [&source, &empty_window]() -> void {
        source.ValidatePopulationForRunInitialization(empty_window);
      },
      "ActivityDriven GGEMSRun sources require a configured non-empty time "
      "schedule");
  expect_configuration(k_reference_before_start_ps);

  EXPECT_NO_THROW(
      source.ValidatePopulationForRunInitialization(initial_time_window));
  expect_configuration(k_reference_before_start_ps);

  EXPECT_NO_THROW(source.SetActivityDrivenRadionuclide(
      radionuclide, ggems::units::Activity{k_activity_bq}, k_window_start_ps));
  EXPECT_NO_THROW(
      source.ValidatePopulationForRunInitialization(initial_time_window));
  expect_configuration(k_window_start_ps);

  EXPECT_NO_THROW(source.SetActivityDrivenRadionuclide(
      radionuclide, ggems::units::Activity{k_activity_bq},
      k_reference_after_start_ps));
  ExpectGGEMSExceptionContaining(
      [&source, &initial_time_window]() -> void {
        source.ValidatePopulationForRunInitialization(initial_time_window);
      },
      "ActivityDriven source reference time must not follow the configured "
      "GGEMSRun start time");
  expect_configuration(k_reference_after_start_ps);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, OwnsAtomicImmutableRadionuclideConfiguration) {
  using ggems::core::sources::GGEMSSourcePopulationMode;

  auto radionuclide = MakeTestRadionuclide();
  std::weak_ptr<ggems::core::radioactivity::GGEMSRadionuclideDefinition const>
      retained = radionuclide;
  auto const *definition_address = radionuclide.get();

  ggems::core::sources::GGEMSSource source{};
  EXPECT_EQ(&source.SetActivityDrivenRadionuclide(
                radionuclide, ggems::units::Activity{12.5L}, 37ULL),
            &source);
  radionuclide.reset();

  EXPECT_FALSE(retained.expired());
  EXPECT_EQ(source.GetPopulationMode(),
            GGEMSSourcePopulationMode::ActivityDriven);
  auto const &configuration = source.GetActivityDrivenConfiguration();
  EXPECT_EQ(configuration.radionuclide.get(), definition_address);
  EXPECT_EQ(configuration.activity_at_reference_time.value, 12.5L);
  EXPECT_EQ(configuration.reference_time_ps, 37ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsInvalidActivityConfigurationAtomically) {
  auto radionuclide = MakeTestRadionuclide();
  auto replacement = MakeTestRadionuclide("Replacement");
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetActivityDrivenRadionuclide(
                   nullptr, ggems::units::Activity{1.0L}, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_EQ(source.GetPrimaryCount(), 4096ULL);

  source.SetActivityDrivenRadionuclide(radionuclide,
                                       ggems::units::Activity{3.0L}, 5ULL);
  auto const &before = source.GetActivityDrivenConfiguration();
  auto const *before_definition = before.radionuclide.get();
  long double const before_activity = before.activity_at_reference_time.value;
  std::uint64_t const before_reference_time = before.reference_time_ps;

  EXPECT_THROW(source.SetActivityDrivenRadionuclide(
                   replacement, ggems::units::Activity{-1.0L}, 8ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(
      source.SetActivityDrivenRadionuclide(
          replacement,
          ggems::units::Activity{std::numeric_limits<long double>::infinity()},
          8ULL),
      ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(
      source.SetActivityDrivenRadionuclide(
          replacement,
          ggems::units::Activity{std::numeric_limits<long double>::quiet_NaN()},
          8ULL),
      ggems::core::GGEMSExceptionBase);

  auto const &after = source.GetActivityDrivenConfiguration();
  EXPECT_EQ(after.radionuclide.get(), before_definition);
  EXPECT_EQ(after.activity_at_reference_time.value, before_activity);
  EXPECT_EQ(after.reference_time_ps, before_reference_time);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ActivityDrivenRejectsSingleParticleConfiguration) {
  constexpr std::array<double, 2U> k_energies{1.0, 2.0};
  constexpr std::array<double, 2U> k_weights{1.0, 1.0};

  ggems::core::sources::GGEMSSource source{};
  source.SetActivityDrivenRadionuclide(MakeTestRadionuclide(),
                                       ggems::units::Activity{1.0L}, 0ULL);

  EXPECT_THROW((void)source.GetPrimaryCount(), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetPrimaryCount(1ULL), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetEmittedParticleType(
                   ggems::core::particles::GGEMSParticleType::Electron),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetEnergyMilliElectronVolt(2'000ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetDiscreteEnergyLines(k_energies, k_weights, "keV"),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetRegularEnergySpectrum(k_energies, k_weights, "keV"),
               ggems::core::GGEMSExceptionBase);
  ExpectGGEMSExceptionContaining(
      [&source]() -> void {
        source.LoadRegularEnergySpectrum(
            "activity-source-must-not-read-this-file.dat", "keV");
      },
      "single-particle configuration");
  EXPECT_THROW((void)source.GetEnergyDistribution(),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW((void)source.BuildRecord(), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.Verbose(), ggems::core::GGEMSExceptionBase);

  EXPECT_NO_THROW(source.SetPositionPicoMeter(1LL, 2LL, 3LL)
                      .SetDirection(1.0, 0.0, 0.0)
                      .SetWeight(0.5F));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ExplicitlySwitchesBackToCountDrivenBeforeInitialization) {
  using ggems::core::sources::GGEMSSourcePopulationMode;

  ggems::core::sources::GGEMSSource source{};
  source.SetActivityDrivenRadionuclide(MakeTestRadionuclide(),
                                       ggems::units::Activity{1.0L}, 0ULL);

  EXPECT_EQ(&source.SetCountDrivenPopulation(19ULL), &source);
  EXPECT_EQ(source.GetPopulationMode(), GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source.GetPrimaryCount(), 19ULL);
  EXPECT_NO_THROW(source
                      .SetEmittedParticleType(
                          ggems::core::particles::GGEMSParticleType::Electron)
                      .SetEnergyMilliElectronVolt(2'000ULL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource,
     DirectFinalizationFreezesPopulationAndEnergyButKeepsCountMutable) {
  auto radionuclide = MakeTestRadionuclide();
  ggems::core::sources::GGEMSSource source{};
  source.SetEnergyMilliElectronVolt(2'000ULL);

  source.FinalizeInitialization();

  ExpectGGEMSExceptionContaining(
      [&source]() -> void { source.SetCountDrivenPopulation(7ULL); },
      k_population_finalized_diagnostic);
  ExpectGGEMSExceptionContaining(
      [&source, &radionuclide]() -> void {
        source.SetActivityDrivenRadionuclide(
            radionuclide, ggems::units::Activity{1.0L}, 0ULL);
      },
      k_population_finalized_diagnostic);
  ExpectGGEMSExceptionContaining(
      [&source]() -> void { source.SetEnergyMilliElectronVolt(3'000ULL); },
      k_energy_finalized_diagnostic);

  EXPECT_EQ(source.GetPopulationMode(),
            ggems::core::sources::GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source.GetEnergyDistribution().GetMonoEnergyMilliElectronVolt(),
            2'000ULL);
  EXPECT_EQ(source.GetPrimaryCount(), 4096ULL);

  EXPECT_NO_THROW(source.SetPrimaryCount(17ULL));
  EXPECT_EQ(source.GetPrimaryCount(), 17ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource,
     DirectActivityDrivenFinalizationPreservesConfigurationAndRejectsMutation) {
  auto radionuclide = MakeTestRadionuclide();
  auto replacement = MakeTestRadionuclide("Replacement");
  ggems::core::sources::GGEMSSource source{};
  source.SetActivityDrivenRadionuclide(radionuclide,
                                       ggems::units::Activity{12.5L}, 37ULL);

  source.FinalizeInitialization();

  ExpectGGEMSExceptionContaining(
      [&source]() -> void { source.SetCountDrivenPopulation(7ULL); },
      k_population_finalized_diagnostic);
  ExpectGGEMSExceptionContaining(
      [&source, &replacement]() -> void {
        source.SetActivityDrivenRadionuclide(
            replacement, ggems::units::Activity{12.5L}, 37ULL);
      },
      k_population_finalized_diagnostic);
  ExpectGGEMSExceptionContaining(
      [&source, &radionuclide]() -> void {
        source.SetActivityDrivenRadionuclide(
            radionuclide, ggems::units::Activity{13.5L}, 37ULL);
      },
      k_population_finalized_diagnostic);
  ExpectGGEMSExceptionContaining(
      [&source, &radionuclide]() -> void {
        source.SetActivityDrivenRadionuclide(
            radionuclide, ggems::units::Activity{12.5L}, 38ULL);
      },
      k_population_finalized_diagnostic);

  ExpectGGEMSExceptionContaining(
      [&source]() -> void {
        source.SetEmittedParticleType(
            ggems::core::particles::GGEMSParticleType::Electron);
      },
      k_activity_single_particle_diagnostic);
  ExpectGGEMSExceptionContaining(
      [&source]() -> void { source.SetEnergyMilliElectronVolt(2'000ULL); },
      k_activity_single_particle_diagnostic);
  ExpectGGEMSExceptionContaining(
      [&source]() -> void { source.SetPrimaryCount(7ULL); },
      k_activity_single_particle_diagnostic);

  EXPECT_EQ(source.GetPopulationMode(),
            ggems::core::sources::GGEMSSourcePopulationMode::ActivityDriven);
  auto const &configuration = source.GetActivityDrivenConfiguration();
  EXPECT_EQ(configuration.radionuclide, radionuclide);
  EXPECT_EQ(configuration.activity_at_reference_time.value, 12.5L);
  EXPECT_EQ(configuration.reference_time_ps, 37ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, CopiesAndMovesActivityConfigurationByManagedOwnership) {
  auto radionuclide = MakeTestRadionuclide();
  ggems::core::sources::GGEMSSource source{};
  source.SetActivityDrivenRadionuclide(radionuclide,
                                       ggems::units::Activity{8.0L}, 9ULL);

  ggems::core::sources::GGEMSSource copied{source};
  EXPECT_EQ(copied.GetActivityDrivenConfiguration().radionuclide, radionuclide);
  EXPECT_EQ(
      copied.GetActivityDrivenConfiguration().activity_at_reference_time.value,
      8.0L);

  ggems::core::sources::GGEMSSource moved{std::move(copied)};
  EXPECT_EQ(moved.GetActivityDrivenConfiguration().radionuclide, radionuclide);
  EXPECT_EQ(moved.GetActivityDrivenConfiguration().reference_time_ps, 9ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsZeroEnergy) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetEnergyMilliElectronVolt(0ULL),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, StandaloneRecordUsesCanonicalStaticTime) {
  ggems::core::sources::GGEMSSource source{};

  auto const record = source.BuildRecord();
  EXPECT_EQ(record.time_start_ps, 0ULL);
  EXPECT_EQ(record.time_stop_ps, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsZeroDirection) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetDirection(0.0F, 0.0F, 0.0F),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNonFiniteDirectionComponents) {
  ggems::core::sources::GGEMSSource source{};

  auto nan = std::numeric_limits<double>::quiet_NaN();
  auto infinity = std::numeric_limits<double>::infinity();

  EXPECT_THROW(source.SetDirection(nan, 0.0F, 1.0F),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetDirection(0.0F, infinity, 1.0F),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetDirection(0.0F, 1.0F, -infinity),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNegativeWeight) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetWeight(-1.0F), ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNonFiniteWeight) {
  ggems::core::sources::GGEMSSource source{};

  float nan = std::numeric_limits<float>::quiet_NaN();
  float infinity = std::numeric_limits<float>::infinity();

  EXPECT_THROW(source.SetWeight(nan), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetWeight(infinity), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetWeight(-infinity), ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, AcceptsZeroWeight) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_NO_THROW(source.SetWeight(0.0F));
  EXPECT_FLOAT_EQ(source.BuildRecord().weight, 0.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, NormalisesDirection) {
  ggems::core::sources::GGEMSSource source{};

  source.SetDirection(0.0, 0.0, 2.0);

  auto const record = source.BuildRecord();

  EXPECT_FLOAT_EQ(record.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 1.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, SetDirectionBuildsAutomaticCtFrame) {
  ggems::core::sources::GGEMSSource source{};
  source.SetDirection(1.0, 0.0, 0.0);

  auto const record = source.BuildRecord();
  EXPECT_FLOAT_EQ(record.axis_x_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_x, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 0.0F);
}

TEST(GGEMSSource, SetOrientationStoresValidatedCompleteFrame) {
  ggems::core::sources::GGEMSSource source{};
  source.SetOrientation({1.0, 0.0, 0.0}, {1.0, 0.0, 2.0});

  auto const record = source.BuildRecord();
  EXPECT_FLOAT_EQ(record.axis_x_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_x, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 0.0F);
}

TEST(GGEMSSource, RejectedOrientationDoesNotMutateExistingFrame) {
  ggems::core::sources::GGEMSSource source{};
  source.SetDirection(1.0, 0.0, 0.0);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetOrientation({0.0, 0.0, 1.0}, {0.001, 0.0, 1.0}),
               ggems::core::GGEMSExceptionBase);

  auto const after = source.BuildRecord();

  ExpectSourceFramesEqual(after, before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, BuildRecordReturnsIndependentOwnedSnapshots) {
  ggems::core::sources::GGEMSSource source{};

  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(111'000'000ULL)
      .SetPositionPicoMeter(11LL, -22LL, 33LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25);

  auto record_a = source.BuildRecord();

  source
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(222'000'000ULL)
      .SetPositionPicoMeter(-44LL, 55LL, -66LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75);

  auto record_b = source.BuildRecord();

  std::uint32_t analytic_source_type = ggems::core::sources::ToKernelSourceType(
      ggems::core::sources::GGEMSSourceType::Analytic);

  EXPECT_EQ(record_a.source_type, analytic_source_type);
  EXPECT_EQ(record_a.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Gamma));
  EXPECT_EQ(record_a.energy_milli_eV, 111'000'000ULL);
  EXPECT_EQ(record_a.time_start_ps, 0ULL);
  EXPECT_EQ(record_a.time_stop_ps, 0ULL);
  EXPECT_EQ(record_a.position_x_pm, 11LL);
  EXPECT_EQ(record_a.position_y_pm, -22LL);
  EXPECT_EQ(record_a.position_z_pm, 33LL);
  EXPECT_FLOAT_EQ(record_a.axis_x_x, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(record_a.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record_a.axis_z_x, 1.0F);
  EXPECT_FLOAT_EQ(record_a.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_z_z, 0.0F);
  EXPECT_FLOAT_EQ(record_a.weight, 0.25F);

  EXPECT_EQ(record_b.source_type, analytic_source_type);
  EXPECT_EQ(record_b.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Electron));

  EXPECT_EQ(record_b.energy_milli_eV, 222'000'000ULL);
  EXPECT_EQ(record_b.time_start_ps, 0ULL);
  EXPECT_EQ(record_b.time_stop_ps, 0ULL);
  EXPECT_EQ(record_b.position_x_pm, -44LL);
  EXPECT_EQ(record_b.position_y_pm, 55LL);
  EXPECT_EQ(record_b.position_z_pm, -66LL);
  EXPECT_FLOAT_EQ(record_b.axis_x_x, 1.0F);
  EXPECT_FLOAT_EQ(record_b.axis_x_y, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record_b.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_z_y, -1.0F);
  EXPECT_FLOAT_EQ(record_b.axis_z_z, 0.0F);
  EXPECT_FLOAT_EQ(record_b.weight, 0.75F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ExecutionRecordPreservesSourceStateAcrossPopulationModes) {
  using ggems::core::particles::GGEMSParticleType;

  ggems::core::sources::GGEMSSource source{};
  source.SetEmittedParticleType(GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(123'456ULL)
      .SetBoxEmissionPicoMeter(11ULL, 13ULL, 17ULL)
      .SetPositionPicoMeter(101LL, -202LL, 303LL)
      .SetOrientation({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0})
      .SetFocusedAngularDistributionPicoMeter(10'000LL, 20'000LL, -30'000LL)
      .SetWeight(0.375F);

  auto const count_driven_record = source.BuildRecord();
  ExpectSourceRecordsEqual(source.BuildExecutionRecord(), count_driven_record);

  auto expected_activity_record = count_driven_record;
  expected_activity_record.emitted_particle_type =
      ggems::core::particles::ToKernelParticleType(GGEMSParticleType::Unknown);
  expected_activity_record.energy_milli_eV = 0ULL;

  source.SetActivityDrivenRadionuclide(MakeTestRadionuclide(),
                                       ggems::units::Activity{7.5L}, 42ULL);
  auto const activity_driven_record = source.BuildExecutionRecord();

  ExpectSourceRecordsEqual(activity_driven_record, expected_activity_record);
  EXPECT_EQ(activity_driven_record.time_start_ps, 0ULL);
  EXPECT_EQ(activity_driven_record.time_stop_ps, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresPointRectangleEllipseAndCircle) {
  using ggems::core::sources::FromKernelEmissionGeometryType;
  using ggems::core::sources::GGEMSEmissionGeometryType;

  ggems::core::sources::GGEMSSource source{};

  source.SetRectangleEmissionPicoMeter(40ULL, 20ULL);
  auto rectangle = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(rectangle.emission_geometry_type),
            GGEMSEmissionGeometryType::Rectangle);
  EXPECT_EQ(rectangle.geometry_size_x_pm, 40ULL);
  EXPECT_EQ(rectangle.geometry_size_y_pm, 20ULL);
  EXPECT_EQ(rectangle.geometry_size_z_pm, 0ULL);

  source.SetEllipseEmissionPicoMeter(30ULL, 10ULL);
  auto ellipse = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(ellipse.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(ellipse.geometry_size_x_pm, 30ULL);
  EXPECT_EQ(ellipse.geometry_size_y_pm, 10ULL);
  EXPECT_EQ(ellipse.geometry_size_z_pm, 0ULL);

  source.SetCircleEmissionPicoMeter(12ULL);
  auto circle = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(circle.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(circle.geometry_size_x_pm, 12ULL);
  EXPECT_EQ(circle.geometry_size_y_pm, 12ULL);
  EXPECT_EQ(circle.geometry_size_z_pm, 0ULL);

  source.SetPointEmission();
  auto point = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(point.emission_geometry_type),
            GGEMSEmissionGeometryType::Point);
  EXPECT_EQ(point.geometry_size_x_pm, 0ULL);
  EXPECT_EQ(point.geometry_size_y_pm, 0ULL);
  EXPECT_EQ(point.geometry_size_z_pm, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsInvalidEmissionDimensionsAtomically) {
  ggems::core::sources::GGEMSSource source{};
  source.SetRectangleEmissionPicoMeter(40ULL, 20ULL);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetRectangleEmissionPicoMeter(0ULL, 20ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetEllipseEmissionPicoMeter(10ULL, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetCircleEmissionPicoMeter(0ULL),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresAngularDistributionsAndCanonicalisesFocus) {
  using ggems::core::sources::FromKernelAngularDistributionType;
  using ggems::core::sources::GGEMSAngularDistributionType;

  ggems::core::sources::GGEMSSource source{};
  source.SetPositionPicoMeter(10LL, 20LL, 30LL)
      .SetFocusedAngularDistributionPicoMeter(40LL, 50LL, 60LL);

  auto focused = source.BuildRecord();
  EXPECT_EQ(
      FromKernelAngularDistributionType(focused.angular_distribution_type),
      GGEMSAngularDistributionType::Focused);
  EXPECT_EQ(focused.focus_position_x_pm, 40LL);
  EXPECT_EQ(focused.focus_position_y_pm, 50LL);
  EXPECT_EQ(focused.focus_position_z_pm, 60LL);

  source.SetIsotropicAngularDistribution();
  auto isotropic = source.BuildRecord();
  EXPECT_EQ(
      FromKernelAngularDistributionType(isotropic.angular_distribution_type),
      GGEMSAngularDistributionType::Isotropic);
  EXPECT_EQ(isotropic.focus_position_x_pm, 0LL);
  EXPECT_EQ(isotropic.focus_position_y_pm, 0LL);
  EXPECT_EQ(isotropic.focus_position_z_pm, 0LL);

  source.SetFixedAngularDistribution();
  auto fixed = source.BuildRecord();
  EXPECT_EQ(FromKernelAngularDistributionType(fixed.angular_distribution_type),
            GGEMSAngularDistributionType::Fixed);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsDegenerateFocusedConfigurations) {
  ggems::core::sources::GGEMSSource point{};

  EXPECT_THROW(point.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 0LL),
               ggems::core::GGEMSExceptionBase);

  ggems::core::sources::GGEMSSource rectangle{};
  rectangle.SetRectangleEmissionPicoMeter(10'000ULL, 20'000ULL);

  EXPECT_THROW(
      rectangle.SetFocusedAngularDistributionPicoMeter(10LL, 20LL, 0LL),
      ggems::core::GGEMSExceptionBase);

  EXPECT_NO_THROW(
      rectangle.SetFocusedAngularDistributionPicoMeter(10LL, 20LL, 1'000LL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PoseChangesDoNotSelectAnotherAngularMode) {
  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution()
      .SetDirection(1.0, 0.0, 0.0)
      .SetOrientation({0.0, 1.0, 0.0}, {0.0, 0.0, 1.0});

  EXPECT_EQ(ggems::core::sources::FromKernelAngularDistributionType(
                source.BuildRecord().angular_distribution_type),
            ggems::core::sources::GGEMSAngularDistributionType::Isotropic);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresCanonicalVolumeDimensionsAndSwitching) {
  using ggems::core::sources::FromKernelEmissionGeometryType;
  using ggems::core::sources::GGEMSEmissionGeometryType;

  ggems::core::sources::GGEMSSource source{};

  source.SetBoxEmissionPicoMeter(40ULL, 20ULL, 10ULL);
  auto const box = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(box.emission_geometry_type),
            GGEMSEmissionGeometryType::Box);
  EXPECT_EQ(box.geometry_size_x_pm, 40ULL);
  EXPECT_EQ(box.geometry_size_y_pm, 20ULL);
  EXPECT_EQ(box.geometry_size_z_pm, 10ULL);

  source.SetSphereEmissionPicoMeter(12ULL);
  auto const sphere = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(sphere.emission_geometry_type),
            GGEMSEmissionGeometryType::Sphere);
  EXPECT_EQ(sphere.geometry_size_x_pm, 12ULL);
  EXPECT_EQ(sphere.geometry_size_y_pm, 12ULL);
  EXPECT_EQ(sphere.geometry_size_z_pm, 12ULL);

  source.SetCylinderEmissionPicoMeter(14ULL, 30ULL);
  auto const cylinder = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(cylinder.emission_geometry_type),
            GGEMSEmissionGeometryType::Cylinder);
  EXPECT_EQ(cylinder.geometry_size_x_pm, 14ULL);
  EXPECT_EQ(cylinder.geometry_size_y_pm, 14ULL);
  EXPECT_EQ(cylinder.geometry_size_z_pm, 30ULL);

  source.SetCircleEmissionPicoMeter(8ULL);
  auto const circle = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(circle.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(circle.geometry_size_x_pm, 8ULL);
  EXPECT_EQ(circle.geometry_size_y_pm, 8ULL);
  EXPECT_EQ(circle.geometry_size_z_pm, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsInvalidVolumeDimensionsAtomically) {
  ggems::core::sources::GGEMSSource source{};
  source.SetRectangleEmissionPicoMeter(40ULL, 20ULL);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetBoxEmissionPicoMeter(0ULL, 20ULL, 10ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetBoxEmissionPicoMeter(40ULL, 0ULL, 10ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetBoxEmissionPicoMeter(40ULL, 20ULL, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetSphereEmissionPicoMeter(0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetCylinderEmissionPicoMeter(0ULL, 10ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetCylinderEmissionPicoMeter(10ULL, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetSphereEmissionPicoMeter(
                   std::numeric_limits<std::uint64_t>::max()),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresBoundedAndCanonicalFullSphereIsotropicDomains) {
  constexpr long double k_pi{std::numbers::pi_v<long double>};
  using ggems::units::MakeRadians;

  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution(
      MakeRadians(0.0L), MakeRadians(0.5L * k_pi), MakeRadians(-0.5L * k_pi),
      MakeRadians(0.5L * k_pi));

  auto const bounded = source.BuildRecord();
  EXPECT_NEAR(bounded.isotropic_cos_theta_lower, 0.0F,
              std::numeric_limits<float>::epsilon());
  EXPECT_FLOAT_EQ(bounded.isotropic_cos_theta_upper, 1.0F);
  EXPECT_FLOAT_EQ(bounded.isotropic_phi_min_rad,
                  static_cast<float>(-0.5L * k_pi));
  EXPECT_FLOAT_EQ(bounded.isotropic_phi_max_rad,
                  static_cast<float>(0.5L * k_pi));
  EXPECT_EQ(bounded.focus_position_x_pm, 0LL);
  EXPECT_EQ(bounded.focus_position_y_pm, 0LL);
  EXPECT_EQ(bounded.focus_position_z_pm, 0LL);

  source.SetIsotropicAngularDistribution(MakeRadians(0.0L), MakeRadians(k_pi),
                                         MakeRadians(0.0L),
                                         MakeRadians(2.0L * k_pi));
  auto const explicit_full_sphere = source.BuildRecord();

  ggems::core::sources::GGEMSSource no_argument{};
  no_argument.SetIsotropicAngularDistribution();
  auto const canonical_full_sphere = no_argument.BuildRecord();

  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_cos_theta_lower,
                  canonical_full_sphere.isotropic_cos_theta_lower);
  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_cos_theta_upper,
                  canonical_full_sphere.isotropic_cos_theta_upper);
  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_phi_min_rad,
                  canonical_full_sphere.isotropic_phi_min_rad);
  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_phi_max_rad,
                  canonical_full_sphere.isotropic_phi_max_rad);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsInvalidOrCollapsedAngularDomainsAtomically) {
  constexpr long double k_pi{std::numbers::pi_v<long double>};
  using ggems::units::MakeRadians;

  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution(MakeRadians(0.1L), MakeRadians(1.0L),
                                         MakeRadians(-1.0L), MakeRadians(1.0L));
  auto const before = source.BuildRecord();
  long double const nan = std::numeric_limits<long double>::quiet_NaN();
  long double const infinity = std::numeric_limits<long double>::infinity();

  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(nan), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(infinity), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(-0.1L), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(1.0L), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(k_pi + 0.1L),
                   MakeRadians(0.0L), MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(1.0L), MakeRadians(1.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(2.0L * k_pi + 0.1L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(1.0L), MakeRadians(1.0L),
                   MakeRadians(std::nextafter(1.0L, 2.0L))),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsFocusedTargetsInsideVolumeSupport) {
  ggems::core::sources::GGEMSSource box{};
  box.SetBoxEmissionPicoMeter(100ULL, 80ULL, 60ULL);
  EXPECT_THROW(box.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 0LL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_NO_THROW(
      box.SetFocusedAngularDistributionPicoMeter(1'000LL, 0LL, 0LL));

  ggems::core::sources::GGEMSSource sphere{};
  sphere.SetSphereEmissionPicoMeter(100ULL);
  EXPECT_THROW(sphere.SetFocusedAngularDistributionPicoMeter(50LL, 0LL, 0LL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_NO_THROW(
      sphere.SetFocusedAngularDistributionPicoMeter(1'000LL, 0LL, 0LL));

  ggems::core::sources::GGEMSSource cylinder{};
  cylinder.SetCylinderEmissionPicoMeter(100ULL, 200ULL);
  EXPECT_THROW(cylinder.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 100LL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_NO_THROW(
      cylinder.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 1'000LL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectedDirectionMovingFocusedBoxPreservesRecord) {
  ggems::core::sources::GGEMSSource source{};
  source.SetBoxEmissionPicoMeter(100ULL, 2ULL, 2ULL)
      .SetFocusedAngularDistributionPicoMeter(0LL, 40LL, 0LL);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetDirection(1.0, 0.0, 0.0),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}
