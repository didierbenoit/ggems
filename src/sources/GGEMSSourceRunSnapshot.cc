#include <array>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <span>
#include <cstddef>
#include <format>
#include <limits>
#include <utility>
#include <vector>
#include <cmath>
#include <numbers>

#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/GGEMSException.hh"

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourcePopulationPlan.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRecord.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRange.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::core::sources {

namespace {

// =============================================================================
// =============================================================================

struct PackedSourceConfiguration {
  std::size_t source_count{0U};
  std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records;
  std::vector<std::uint64_t> energy_values_micro_eV;
  std::vector<double> relative_weights;
  std::vector<std::uint64_t> cumulative_ticket_upper;
  std::vector<GGEMSSourceEmissionRecord> source_emission_records;
  std::vector<std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
    radionuclide_definitions;
};

// =============================================================================
// =============================================================================

auto AppendEnergyDistribution(PackedSourceConfiguration &packed,
                              GGEMSEnergyDistribution const &distribution)
  -> void {
  auto const energy_values = distribution.GetEnergyValuesMicroElectronVolt();
  auto const relative_weights = distribution.GetRelativeWeights();
  auto const cumulative_ticket_upper =
    distribution.GetCumulativeTicketUpperBounds();

  auto const table_offset =
    static_cast<std::uint64_t>(packed.energy_values_micro_eV.size());
  packed.energy_distribution_records.push_back(
    distribution.BuildRecord(table_offset));
  packed.energy_values_micro_eV.insert(packed.energy_values_micro_eV.end(),
                                       energy_values.begin(),
                                       energy_values.end());
  packed.relative_weights.insert(packed.relative_weights.end(),
                                 relative_weights.begin(),
                                 relative_weights.end());
  packed.cumulative_ticket_upper.insert(packed.cumulative_ticket_upper.end(),
                                        cumulative_ticket_upper.begin(),
                                        cumulative_ticket_upper.end());
}

// =============================================================================
// =============================================================================

auto PackSourceConfiguration(std::span<GGEMSSource const *const> sources)
  -> PackedSourceConfiguration {

  PackedSourceConfiguration packed;
  packed.source_count = sources.size();

  std::size_t total_table_count{0U};
  std::size_t total_emission_count{0U};

  for (std::size_t source_index = 0U; source_index < sources.size();
       ++source_index) {
    GGEMSSource const *source = sources[source_index];

    if (source == nullptr) {
      throw ggems::core::GGEMSRecoverable(
        std::format("Cannot pack source configuration: source at index {} is "
                    "null.",
                    source_index));
    }

    if (source->GetPopulationMode() == GGEMSSourcePopulationMode::CountDriven) {
      total_table_count += source->GetEnergyDistribution()
                             .GetEnergyValuesMicroElectronVolt()
                             .size();
      continue;
    }

    auto const configuration =
      source->BuildActivityDrivenPopulationConfiguration();

    auto const emissions = configuration.radionuclide->GetEmissions();
    total_emission_count += emissions.size();
    for (auto const &emission : emissions) {
      total_table_count += emission.GetEnergyDistribution()
                             .GetEnergyValuesMicroElectronVolt()
                             .size();
    }
  }

  std::size_t const total_energy_record_count =
    sources.size() + total_emission_count;

  packed.energy_distribution_records.reserve(total_energy_record_count);
  packed.energy_values_micro_eV.reserve(total_table_count);
  packed.relative_weights.reserve(total_table_count);
  packed.cumulative_ticket_upper.reserve(total_table_count);
  packed.source_emission_records.reserve(total_emission_count);
  packed.radionuclide_definitions.reserve(sources.size());

  // Keep the first N energy records source-indexed for CountDriven lookup.
  for (GGEMSSource const *source : sources) {
    if (source->GetPopulationMode() == GGEMSSourcePopulationMode::CountDriven) {
      AppendEnergyDistribution(packed, source->GetEnergyDistribution());
      packed.radionuclide_definitions.push_back(nullptr);
    } else {
      packed.energy_distribution_records.emplace_back();
      auto const configuration =
        source->BuildActivityDrivenPopulationConfiguration();
      packed.radionuclide_definitions.push_back(configuration.radionuclide);
    }
  }

  for (GGEMSSource const *source : sources) {
    if (source->GetPopulationMode() == GGEMSSourcePopulationMode::CountDriven) {
      continue;
    }

    auto const configuration =
      source->BuildActivityDrivenPopulationConfiguration();
    auto const &definition = *configuration.radionuclide;
    for (auto const &emission : definition.GetEmissions()) {
      auto const energy_record_index =
        static_cast<std::uint32_t>(packed.energy_distribution_records.size());
      auto const &distribution = emission.GetEnergyDistribution();
      std::uint64_t const mono_energy_micro_eV =
        distribution.GetType() == GGEMSEnergyDistributionType::Mono
          ? distribution.GetMonoEnergyMicroElectronVolt()
          : 0ULL;

      packed.source_emission_records.push_back({
        .particle_type =
          particles::ToKernelParticleType(emission.GetParticleType()),
        .energy_distribution_record_index = energy_record_index,
        .mono_energy_micro_eV = mono_energy_micro_eV,
      });
      AppendEnergyDistribution(packed, distribution);
    }
  }

  return packed;
}

// =============================================================================
// =============================================================================

auto ValidateTimeWindow(GGEMSTimeWindow time_window) -> void {
  if (time_window.start_ps > time_window.stop_ps) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSSourceRunSnapshot time window stop precedes its start.");
  }
}

// =============================================================================
// =============================================================================

auto AppendCountDrivenRunSnapshotEntry(
  std::vector<GGEMSSourceRecord> &records,
  std::vector<GGEMSSourceRunRange> &ranges,
  std::vector<GGEMSSourcePopulationRecord> &population_records,
  std::uint64_t &total_primary_count, GGEMSSource const &source,
  std::size_t source_index, GGEMSTimeWindow time_window) -> void {
  if (source.GetPopulationMode() != GGEMSSourcePopulationMode::CountDriven) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Cannot build a standalone GGEMSSourceRunSnapshot: source "
                  "at index {} is ActivityDriven and requires a planner "
                  "candidate.",
                  source_index));
  }

  GGEMSSourceRecord source_record = source.BuildRecord();
  source_record.time_start_ps = time_window.start_ps;
  source_record.time_stop_ps = time_window.stop_ps;
  std::uint64_t const source_primary_count = source.GetPrimaryCount();

  records.push_back(source_record);
  ranges.push_back({
    .projection_primary_begin = total_primary_count,
    .primary_count = source_primary_count,
  });
  population_records.emplace_back();

  total_primary_count += source_primary_count;
}

// =============================================================================
// =============================================================================

auto BuildScaledDecay(
  GGEMSTimeWindow time_window,
  radioactivity::GGEMSRadionuclideDefinition const &definition) -> float {
  std::uint64_t const duration_ps = time_window.stop_ps - time_window.start_ps;
  auto const duration_seconds =
    *units::ConvertTo(units::Duration{duration_ps}, "s");
  long double const scaled_decay = std::numbers::ln2_v<long double> *
                                   duration_seconds /
                                   definition.GetHalfLifeSeconds();

  if (!(std::isfinite(scaled_decay) && scaled_decay >= 0.0L &&
        scaled_decay <=
          static_cast<long double>(std::numeric_limits<float>::max()))) {
    throw ggems::core::GGEMSRecoverable(
      "ActivityDriven scaled decay cannot be represented as finite binary32.");
  }

  return static_cast<float>(scaled_decay);
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSSourceConfigurationSnapshot::GGEMSSourceConfigurationSnapshot(
  std::size_t source_count,
  std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records,
  std::vector<std::uint64_t> energy_values_micro_eV,
  std::vector<double> relative_weights,
  std::vector<std::uint64_t> cumulative_ticket_upper,
  std::vector<GGEMSSourceEmissionRecord> source_emission_records,
  std::vector<std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>>
    radionuclide_definitions)
    : source_count_{source_count},
      energy_distribution_records_{std::move(energy_distribution_records)},
      energy_values_micro_eV_{std::move(energy_values_micro_eV)},
      relative_weights_{std::move(relative_weights)},
      cumulative_ticket_upper_{std::move(cumulative_ticket_upper)},
      source_emission_records_{std::move(source_emission_records)},
      radionuclide_definitions_{std::move(radionuclide_definitions)} {}

// =============================================================================
// =============================================================================

auto GGEMSSourceConfigurationSnapshot::Create(GGEMSSource const &source)
  -> GGEMSSourceConfigurationSnapshotPtr {
  std::array<GGEMSSource const *, 1U> sources{&source};
  PackedSourceConfiguration packed = PackSourceConfiguration(sources);

  return GGEMSSourceConfigurationSnapshotPtr{
    new GGEMSSourceConfigurationSnapshot{
      packed.source_count, std::move(packed.energy_distribution_records),
      std::move(packed.energy_values_micro_eV),
      std::move(packed.relative_weights),
      std::move(packed.cumulative_ticket_upper),
      std::move(packed.source_emission_records),
      std::move(packed.radionuclide_definitions)}};
}

// =============================================================================
// =============================================================================

auto BuildSourceConfigurationSnapshot(GGEMSSource const &source)
  -> GGEMSSourceConfigurationSnapshotPtr {
  return GGEMSSourceConfigurationSnapshot::Create(source);
}

// -----------------------------------------------------------------------------

auto GGEMSSourceConfigurationSnapshot::Create(

  std::span<std::shared_ptr<GGEMSSource> const> sources)
  -> GGEMSSourceConfigurationSnapshotPtr {
  std::vector<GGEMSSource const *> source_pointers;
  source_pointers.reserve(sources.size());

  for (auto const &source : sources) {
    source_pointers.push_back(source.get());
  }

  PackedSourceConfiguration packed = PackSourceConfiguration(source_pointers);

  return GGEMSSourceConfigurationSnapshotPtr{
    new GGEMSSourceConfigurationSnapshot{
      packed.source_count, std::move(packed.energy_distribution_records),
      std::move(packed.energy_values_micro_eV),
      std::move(packed.relative_weights),
      std::move(packed.cumulative_ticket_upper),
      std::move(packed.source_emission_records),
      std::move(packed.radionuclide_definitions)}};
}

// =============================================================================
// =============================================================================

auto BuildSourceConfigurationSnapshot(
  std::span<std::shared_ptr<GGEMSSource> const> sources)
  -> GGEMSSourceConfigurationSnapshotPtr {
  return GGEMSSourceConfigurationSnapshot::Create(sources);
}

// =============================================================================
// =============================================================================

GGEMSSourceRunSnapshot::GGEMSSourceRunSnapshot(
  std::vector<GGEMSSourceRecord> records,
  std::vector<GGEMSSourceRunRange> ranges,
  std::vector<GGEMSSourcePopulationRecord> population_records,
  std::vector<GGEMSSourceEmissionRange> emission_ranges,
  GGEMSTimeWindow time_window,
  GGEMSSourceConfigurationSnapshotPtr source_configuration,
  std::uint64_t total_primary_count)
    : records_{std::move(records)}, ranges_{std::move(ranges)},
      population_records_{std::move(population_records)},
      emission_ranges_{std::move(emission_ranges)}, time_window_{time_window},
      source_configuration_{std::move(source_configuration)},
      total_primary_count_{total_primary_count} {}

// =============================================================================
// =============================================================================

auto GGEMSSourceRunSnapshot::HasActivityDrivenSource() const noexcept -> bool {
  auto const activity_mode =
    ToKernelSourcePopulationMode(GGEMSSourcePopulationMode::ActivityDriven);

  return std::ranges::any_of(
    population_records_,
    [](GGEMSSourcePopulationRecord const &record) noexcept -> bool {
      return record.population_mode == activity_mode;
    });
}

// =============================================================================
// =============================================================================

auto GGEMSSourceRunSnapshot::Create(
  std::span<std::shared_ptr<GGEMSSource> const> sources,
  GGEMSSourceConfigurationSnapshotPtr source_configuration,
  GGEMSSourcePopulationPlan const &population_plan) -> GGEMSSourceRunSnapshot {
  GGEMSTimeWindow const time_window = population_plan.GetTimeWindow();
  ValidateTimeWindow(time_window);
  if (source_configuration == nullptr) {
    throw ggems::core::GGEMSInternal(
      "GGEMSSource configuration snapshot is null.");
  }
  if (source_configuration->GetSourceCount() != sources.size()) {
    throw ggems::core::GGEMSInternal(
      "GGEMSSource configuration and run snapshot slot counts do not match.");
  }

  auto const plan_sources = population_plan.GetSources();
  auto const plan_emissions = population_plan.GetGroups();
  auto const plan_definitions = population_plan.GetRadionuclideDefinitions();
  auto const &stable_definitions =
    source_configuration->GetRadionuclideDefinitions();

  if (!(plan_sources.size() == sources.size() &&
        plan_definitions.size() == sources.size() &&
        stable_definitions.size() == sources.size())) {
    throw ggems::core::GGEMSInternal(
      "Emission plan and source configuration slot counts do not match.");
  }
  if (plan_emissions.size() != source_configuration->GetEmissionCount()) {
    throw ggems::core::GGEMSInternal(
      "Emission plan and immutable emission counts do not match.");
  }

  std::vector<GGEMSSourceRecord> records;
  std::vector<GGEMSSourceRunRange> ranges;
  std::vector<GGEMSSourcePopulationRecord> population_records;
  std::vector<GGEMSSourceEmissionRange> emission_ranges;
  records.reserve(sources.size());
  ranges.reserve(sources.size());
  population_records.reserve(sources.size());
  emission_ranges.reserve(plan_emissions.size());

  for (std::size_t source_index = 0U; source_index < sources.size();
       ++source_index) {
    auto const &source = sources[source_index];
    if (source == nullptr) {
      throw ggems::core::GGEMSRecoverable(std::format(
        "Cannot build GGEMSSourceRunSnapshot: source at index {} is null.",
        source_index));
    }

    auto const &plan_source = plan_sources[source_index];
    auto const population_mode = source->GetPopulationMode();
    if (plan_source.source_index != static_cast<std::uint32_t>(source_index) ||
        plan_source.population_mode != population_mode) {
      throw ggems::core::GGEMSInternal(
        "Emission plan source order or population mode does not match.");
    }

    std::uint64_t const source_primary_count =
      plan_source.run_primary_end - plan_source.run_primary_begin;

    GGEMSSourceRecord source_record =
      population_mode == GGEMSSourcePopulationMode::CountDriven
        ? source->BuildRecord()
        : source->BuildExecutionRecord();
    source_record.time_start_ps = time_window.start_ps;
    source_record.time_stop_ps = time_window.stop_ps;

    GGEMSSourcePopulationRecord population_record{};
    population_record.population_mode =
      ToKernelSourcePopulationMode(population_mode);

    if (population_mode == GGEMSSourcePopulationMode::CountDriven) {
      if (!(plan_source.emission_count == 0ULL &&
            plan_definitions[source_index] == nullptr &&
            stable_definitions[source_index] == nullptr)) {
        throw ggems::core::GGEMSInternal(
          "CountDriven emission plan contains radionuclide configuration.");
      }
    } else {
      auto const activity =
        source->BuildActivityDrivenPopulationConfiguration();
      if (!(activity.radionuclide != nullptr &&
            activity.radionuclide == plan_definitions[source_index] &&
            activity.radionuclide == stable_definitions[source_index])) {
        throw ggems::core::GGEMSInternal(
          "ActivityDriven radionuclide definition does not match immutable "
          "configuration.");
      }
      if (plan_source.emission_count !=
          activity.radionuclide->GetEmissions().size()) {
        throw ggems::core::GGEMSInternal(
          "ActivityDriven emission range does not match its definition.");
      }

      population_record.first_emission_index =
        static_cast<std::uint32_t>(plan_source.emission_begin);
      population_record.emission_count =
        static_cast<std::uint32_t>(plan_source.emission_count);
      population_record.scaled_decay =
        BuildScaledDecay(time_window, *activity.radionuclide);

      for (std::uint64_t emission_offset = 0ULL;
           emission_offset < plan_source.emission_count; ++emission_offset) {
        auto const emission_index = static_cast<std::size_t>(
          plan_source.emission_begin + emission_offset);
        auto const &emission = plan_emissions[emission_index];

        if (emission.source_index != static_cast<std::uint32_t>(source_index) ||
            emission.emission_index !=
              static_cast<std::uint32_t>(emission_offset)) {
          throw ggems::core::GGEMSInternal(
            "Emission plan group order does not match its source.");
        }

        emission_ranges.push_back({
          .source_local_primary_begin = emission.source_local_primary_begin,
          .primary_count = emission.sampled_primary_count,
        });
      }
    }

    records.push_back(source_record);
    ranges.push_back({
      .projection_primary_begin = plan_source.run_primary_begin,
      .primary_count = source_primary_count,
    });
    population_records.push_back(population_record);
  }

  return GGEMSSourceRunSnapshot{std::move(records),
                                std::move(ranges),
                                std::move(population_records),
                                std::move(emission_ranges),
                                time_window,
                                std::move(source_configuration),
                                population_plan.GetTotalPrimaryCount()};
}

// =============================================================================
// =============================================================================

auto BuildSourceRunSnapshot(
  std::span<std::shared_ptr<GGEMSSource> const> sources,
  GGEMSSourceConfigurationSnapshotPtr source_configuration,
  GGEMSSourcePopulationPlan const &population_plan) -> GGEMSSourceRunSnapshot {
  return GGEMSSourceRunSnapshot::Create(
    sources, std::move(source_configuration), population_plan);
}

// =============================================================================
// =============================================================================

auto GGEMSSourceRunSnapshot::Create(
  std::span<std::shared_ptr<GGEMSSource> const> sources,
  GGEMSSourceConfigurationSnapshotPtr source_configuration,
  GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot {
  ValidateTimeWindow(time_window);
  if (source_configuration == nullptr) {
    throw ggems::core::GGEMSInternal(
      "GGEMSSource configuration snapshot is null.");
  }
  if (source_configuration->GetSourceCount() != sources.size()) {
    throw ggems::core::GGEMSInternal(
      "GGEMSSource configuration and run snapshot slot counts do not match.");
  }

  std::vector<GGEMSSourceRecord> records;
  std::vector<GGEMSSourceRunRange> ranges;
  std::vector<GGEMSSourcePopulationRecord> population_records;
  records.reserve(sources.size());
  ranges.reserve(sources.size());
  population_records.reserve(sources.size());

  std::uint64_t total_primary_count{0ULL};

  for (std::size_t source_index = 0U; source_index < sources.size();
       ++source_index) {
    auto const &source = sources[source_index];

    if (source == nullptr) {
      throw ggems::core::GGEMSRecoverable(std::format(
        "Cannot build GGEMSSourceRunSnapshot: source at index {} is null.",
        source_index));
    }

    AppendCountDrivenRunSnapshotEntry(records, ranges, population_records,
                                      total_primary_count, *source,
                                      source_index, time_window);
  }

  return GGEMSSourceRunSnapshot{std::move(records),
                                std::move(ranges),
                                std::move(population_records),
                                {},
                                time_window,
                                std::move(source_configuration),
                                total_primary_count};
}

// =============================================================================
// =============================================================================

auto BuildSourceRunSnapshot(
  std::span<std::shared_ptr<GGEMSSource> const> sources,
  GGEMSSourceConfigurationSnapshotPtr source_configuration,
  GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot {
  return GGEMSSourceRunSnapshot::Create(
    sources, std::move(source_configuration), time_window);
}

// =============================================================================
// =============================================================================

auto BuildSourceRunSnapshot(
  std::span<std::shared_ptr<GGEMSSource> const> sources,
  GGEMSSourceConfigurationSnapshotPtr source_configuration)
  -> GGEMSSourceRunSnapshot {
  return BuildSourceRunSnapshot(sources, std::move(source_configuration), {});
}

// =============================================================================
// =============================================================================

auto BuildSourceRunSnapshot(
  std::span<std::shared_ptr<GGEMSSource> const> sources,
  GGEMSTimeWindow time_window) -> GGEMSSourceRunSnapshot {
  auto source_configuration = BuildSourceConfigurationSnapshot(sources);
  return BuildSourceRunSnapshot(sources, std::move(source_configuration),
                                time_window);
}

// =============================================================================
// =============================================================================

auto BuildSourceRunSnapshot(
  std::span<std::shared_ptr<GGEMSSource> const> sources)
  -> GGEMSSourceRunSnapshot {
  return BuildSourceRunSnapshot(sources, GGEMSTimeWindow{});
}

// =============================================================================
// =============================================================================

auto GGEMSSourceRunSnapshot::Create(GGEMSSource const &source,
                                    GGEMSTimeWindow time_window)
  -> GGEMSSourceRunSnapshot {
  ValidateTimeWindow(time_window);
  std::vector<GGEMSSourceRecord> records;
  std::vector<GGEMSSourceRunRange> ranges;
  std::vector<GGEMSSourcePopulationRecord> population_records;
  records.reserve(1U);
  ranges.reserve(1U);
  population_records.reserve(1U);

  std::uint64_t total_primary_count{0ULL};
  AppendCountDrivenRunSnapshotEntry(records, ranges, population_records,
                                    total_primary_count, source, 0U,
                                    time_window);

  return GGEMSSourceRunSnapshot{std::move(records),
                                std::move(ranges),
                                std::move(population_records),
                                {},
                                time_window,
                                BuildSourceConfigurationSnapshot(source),
                                total_primary_count};
}

// =============================================================================
// =============================================================================

auto BuildSourceRunSnapshot(GGEMSSource const &source,
                            GGEMSTimeWindow time_window)
  -> GGEMSSourceRunSnapshot {
  return GGEMSSourceRunSnapshot::Create(source, time_window);
}

// =============================================================================
// =============================================================================

auto BuildSourceRunSnapshot(GGEMSSource const &source)
  -> GGEMSSourceRunSnapshot {
  return BuildSourceRunSnapshot(source, {});
}
} // namespace ggems::core::sources
