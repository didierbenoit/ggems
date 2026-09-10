#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <span>


#include "GGEMS/GGEMSException.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceDescription.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourceValidation.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::core::sources {

// =============================================================================
// =============================================================================

namespace {
[[nodiscard]] auto DescribeTime(GGEMSSourceRecord const &record)
    -> std::string {
  std::string const time_start =
      ggems::units::HumanReadable(ggems::units::Time{record.time_start_ps});

  if (record.time_stop_ps <= record.time_start_ps) {
    return std::format("Time: fixed at {}", time_start);
  }

  std::string const time_stop =
      ggems::units::HumanReadable(ggems::units::Time{record.time_stop_ps});

  return std::format("Time window: [{}, {})", time_start, time_stop);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeEmission(GGEMSSourceRecord const &record)
    -> std::string {
  GGEMSEmissionGeometryType const geometry_type =
      FromKernelEmissionGeometryType(record.emission_geometry_type);

  if (geometry_type == GGEMSEmissionGeometryType::Rectangle) {
    return std::format("Emission: Rectangle | Size: {} x {}",
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_x_pm}),
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_y_pm}));
  }

  if (geometry_type == GGEMSEmissionGeometryType::Ellipse) {
    return std::format("Emission: Ellipse | Diameter: {} x {}",
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_x_pm}),
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_y_pm}));
  }

  if (geometry_type == GGEMSEmissionGeometryType::Box) {
    return std::format("Emission: Box | Size: {} x {} x {}",
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_x_pm}),
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_y_pm}),
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_z_pm}));
  }

  if (geometry_type == GGEMSEmissionGeometryType::Sphere) {
    return std::format("Emission: Sphere | Diameter: {}",
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_x_pm}));
  }

  if (geometry_type == GGEMSEmissionGeometryType::Cylinder) {
    return std::format("Emission: Cylinder | Diameter: {} | Height: {}",
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_x_pm}),
                       ggems::units::HumanReadable(
                           ggems::units::Length{record.geometry_size_z_pm}));
  }

  return std::format("Emission: {}", ToLongName(geometry_type));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeAngularDistribution(GGEMSSourceRecord const &record)
    -> std::string {
  GGEMSAngularDistributionType const distribution_type =
      FromKernelAngularDistributionType(record.angular_distribution_type);

  if (distribution_type == GGEMSAngularDistributionType::Isotropic) {
    if (IsDefaultFullSphereIsotropicDomain(record)) {
      return "Angular: Isotropic | Domain: Full sphere";
    }

    long double const theta_min = std::acos(
        std::clamp(static_cast<long double>(record.isotropic_cos_theta_upper),
                   -1.0L, 1.0L));
    long double const theta_max = std::acos(
        std::clamp(static_cast<long double>(record.isotropic_cos_theta_lower),
                   -1.0L, 1.0L));

    return std::format(
        "Angular: Isotropic | Theta: {} to {} | Phi: {} to {}",
        ggems::units::HumanReadable(ggems::units::MakeRadians(theta_min)),
        ggems::units::HumanReadable(ggems::units::MakeRadians(theta_max)),
        ggems::units::HumanReadable(ggems::units::MakeRadians(
            static_cast<long double>(record.isotropic_phi_min_rad))),
        ggems::units::HumanReadable(ggems::units::MakeRadians(
            static_cast<long double>(record.isotropic_phi_max_rad))));
  }

  if (distribution_type != GGEMSAngularDistributionType::Focused) {
    return std::format("Angular: {}", ToLongName(distribution_type));
  }

  return std::format(
      "Angular: Focused | Focus: ({}, {}, {})",
      ggems::units::HumanReadableSignedLength(record.focus_position_x_pm),
      ggems::units::HumanReadableSignedLength(record.focus_position_y_pm),
      ggems::units::HumanReadableSignedLength(record.focus_position_z_pm));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
DescribeEnergy(GGEMSSourceRecord const &source_record,
               GGEMSEnergyDistributionRecord const &energy_record,
               std::span<std::uint64_t const> energy_values) -> std::string {
  GGEMSEnergyDistributionType const distribution_type =
      FromKernelEnergyDistributionType(energy_record.distribution_type);

  if (distribution_type == GGEMSEnergyDistributionType::Mono) {
    if (!(source_record.energy_micro_eV > 0ULL &&
          energy_record.table_offset == 0ULL &&
          energy_record.table_count == 0U &&
          energy_record.regular_bin_width_micro_eV == 0ULL)) {
      throw ggems::core::GGEMSInternal("Invalid Mono energy description record.");
    }

    return std::format("Energy: Mono ({})",
                       ggems::units::HumanReadable(ggems::units::Energy{
                           source_record.energy_micro_eV}));
  }

  if (!(distribution_type == GGEMSEnergyDistributionType::DiscreteLines ||
          distribution_type == GGEMSEnergyDistributionType::RegularSpectrum)) {
    throw ggems::core::GGEMSInternal("Unsupported source energy distribution in description.");
  }

  if (!(energy_record.table_offset <=
                           static_cast<std::uint64_t>(energy_values.size()))) {
    throw ggems::core::GGEMSInternal("Source energy description table offset is invalid.");
  }

  auto const table_offset =
      static_cast<std::size_t>(energy_record.table_offset);
  auto const table_count = static_cast<std::size_t>(energy_record.table_count);

  if (!(table_count >= 2U &&
                           table_count <= energy_values.size() - table_offset)) {
    throw ggems::core::GGEMSInternal("Source energy description table range is invalid.");
  }

  std::string const first = ggems::units::HumanReadable(
      ggems::units::Energy{energy_values[table_offset]});
  std::string const last = ggems::units::HumanReadable(
      ggems::units::Energy{energy_values[table_offset + table_count - 1U]});

  if (distribution_type == GGEMSEnergyDistributionType::DiscreteLines) {
    return std::format("Energy: Discrete lines | Line count: {} | "
                       "Energy range: [{}, {}]",
                       table_count, first, last);
  }

  return std::format("Energy: Regular spectrum | Bin count: {} | "
                     "Center range: [{}, {}] | Bin width: {}",
                     table_count, first, last,
                     ggems::units::HumanReadable(ggems::units::Energy{
                         energy_record.regular_bin_width_micro_eV}));
}

} // namespace

// =============================================================================
// =============================================================================

auto DescribeSource(GGEMSSource const &source) -> std::string {
  if (source.GetPopulationMode() == GGEMSSourcePopulationMode::CountDriven) {
    GGEMSSourceRecord const source_record = source.BuildRecord();
    auto const &energy_distribution = source.GetEnergyDistribution();
    GGEMSEnergyDistributionRecord const energy_record =
        energy_distribution.BuildRecord(0ULL);

    return DescribeSource(
        source_record, source.GetPrimaryCount(), energy_record,
        energy_distribution.GetEnergyValuesMicroElectronVolt());
  }

  GGEMSSourceRecord const source_record = source.BuildExecutionRecord();
  auto const configuration =
      source.BuildActivityDrivenPopulationConfiguration();
  auto const &radionuclide = *configuration.radionuclide;

  return std::format(
      "Type: {} | Population: ActivityDriven | Radionuclide: {} | "
      "Activity at reference time: {} | Reference time: {} | "
      "Emission count: {} | {} | {} | Position: ({}, {}, {}) | "
      "Axis Z: ({}, {}, {}) | Weight: {}",
      ToLongName(FromKernelSourceType(source_record.source_type)),
      radionuclide.GetCanonicalName(),
      ggems::units::HumanReadable(configuration.activity_at_reference_time),
      ggems::units::HumanReadable(
          ggems::units::TimePoint{.value = configuration.reference_time_ps}),
      radionuclide.GetEmissions().size(), DescribeEmission(source_record),
      DescribeAngularDistribution(source_record),
      ggems::units::HumanReadableSignedLength(source_record.position_x_pm),
      ggems::units::HumanReadableSignedLength(source_record.position_y_pm),
      ggems::units::HumanReadableSignedLength(source_record.position_z_pm),
      source_record.axis_z_x, source_record.axis_z_y, source_record.axis_z_z,
      source_record.weight);
}

// =============================================================================
// =============================================================================

auto DescribeSource(GGEMSSourceRecord const &record,
                    std::uint64_t primary_count) -> std::string {
  if (!(record.energy_micro_eV > 0ULL)) {
    throw ggems::core::GGEMSInternal("The energy-aware DescribeSource overload is required for a "
      "table-backed source.");
  }

  GGEMSEnergyDistributionRecord const energy_record{
      .distribution_type =
          ToKernelEnergyDistributionType(GGEMSEnergyDistributionType::Mono)};

  return DescribeSource(record, primary_count, energy_record, {});
}

// =============================================================================
// =============================================================================

auto DescribeSource(GGEMSSourceRecord const &record,
                    std::uint64_t primary_count,
                    GGEMSEnergyDistributionRecord const &energy_record,
                    std::span<std::uint64_t const> energy_values)
    -> std::string {

  GGEMSSourceType const source_type = FromKernelSourceType(record.source_type);

  particles::GGEMSParticleType const particle_type =
      particles::FromKernelParticleType(record.emitted_particle_type);

  return std::format(
      "Type: {} | Primary count: {} | Particle: {} ({}) | "
      "{} | {} | {} | {} | Position: ({}, {}, {}) | "
      "Axis Z: ({}, {}, {}) | Weight: {}",
      ToLongName(source_type), primary_count,
      particles::ToLongName(particle_type),
      particles::ToShortName(particle_type), DescribeEmission(record),
      DescribeAngularDistribution(record),
      DescribeEnergy(record, energy_record, energy_values),
      DescribeTime(record),
      ggems::units::HumanReadableSignedLength(record.position_x_pm),
      ggems::units::HumanReadableSignedLength(record.position_y_pm),
      ggems::units::HumanReadableSignedLength(record.position_z_pm),
      record.axis_z_x, record.axis_z_y, record.axis_z_z, record.weight);
}

// =============================================================================
// =============================================================================

auto DescribeSourceRunSlot(std::size_t source_index,
                           GGEMSSourceRunSnapshot const &snapshot)
    -> std::string {
  auto const &records = snapshot.GetRecords();
  auto const &ranges = snapshot.GetRanges();
  auto const &population_records = snapshot.GetPopulationRecords();
  auto const &energy_records = snapshot.GetEnergyDistributionRecords();

  if (!(source_index < records.size() &&
                           source_index < ranges.size() &&
                           source_index < population_records.size() &&
                           source_index < energy_records.size())) {
    throw ggems::core::GGEMSInternal("Source description index is outside the run snapshot.");
  }

  auto const &population = population_records[source_index];
  if (population.population_mode ==
      ToKernelSourcePopulationMode(GGEMSSourcePopulationMode::ActivityDriven)) {
    auto const &record = records[source_index];
    auto const &range = ranges[source_index];
    auto const &emission_records = snapshot.GetEmissionRecords();
    auto const &group_ranges = snapshot.GetGroupRanges();
    auto const &definitions = snapshot.GetRadionuclideDefinitions();

    if (!(source_index < definitions.size() &&
            definitions[source_index] != nullptr &&
            population.first_emission_index <= emission_records.size() &&
            population.emission_count <=
                emission_records.size() - population.first_emission_index &&
            group_ranges.size() == emission_records.size())) {
      throw ggems::core::GGEMSInternal(
          "ActivityDriven source description metadata is inconsistent.");
    }

    std::string groups;
    for (std::uint32_t offset = 0U; offset < population.emission_count;
         ++offset) {
      std::size_t const emission_index =
          static_cast<std::size_t>(population.first_emission_index) + offset;
      auto const &emission = emission_records[emission_index];
      if (!(emission.energy_distribution_record_index < energy_records.size())) {
        throw ggems::core::GGEMSInternal(
            "ActivityDriven energy record index is outside the snapshot.");
      }

      if (!groups.empty()) {
        groups += "; ";
      }
      groups +=
          std::format("#{} {} count={}", offset,
                      particles::ToLongName(particles::FromKernelParticleType(
                          emission.particle_type)),
                      group_ranges[emission_index].primary_count);
    }

    return std::format(
        "Source slot: {} | Projection primary begin: {} | Type: {} | "
        "Population: ActivityDriven | Radionuclide: {} | Primary count: {} | "
        "Emission groups: {} [{}] | {} | {} | {} | Position: ({}, {}, {}) | "
        "Axis Z: ({}, {}, {}) | Weight: {}",
        source_index, range.projection_primary_begin,
        ToLongName(FromKernelSourceType(record.source_type)),
        definitions[source_index]->GetCanonicalName(), range.primary_count,
        population.emission_count, groups, DescribeEmission(record),
        DescribeAngularDistribution(record), DescribeTime(record),
        ggems::units::HumanReadableSignedLength(record.position_x_pm),
        ggems::units::HumanReadableSignedLength(record.position_y_pm),
        ggems::units::HumanReadableSignedLength(record.position_z_pm),
        record.axis_z_x, record.axis_z_y, record.axis_z_z, record.weight);
  }

  return DescribeSourceRunSlot(source_index, records[source_index],
                               ranges[source_index],
                               energy_records[source_index],
                               snapshot.GetEnergyValuesMicroElectronVolt());
}

// =============================================================================
// =============================================================================

auto DescribeSourceRunSlot(std::size_t source_index,
                           GGEMSSourceRecord const &record,
                           GGEMSSourceRunRange const &range) -> std::string {
  if (!(record.energy_micro_eV > 0ULL)) {
    throw ggems::core::GGEMSInternal(
        "The energy-aware DescribeSourceRunSlot overload is required for a "
      "table-backed source.");
  }

  GGEMSEnergyDistributionRecord const energy_record{
      .distribution_type =
          ToKernelEnergyDistributionType(GGEMSEnergyDistributionType::Mono)};

  return DescribeSourceRunSlot(source_index, record, range, energy_record, {});
}

// =============================================================================
// =============================================================================

auto DescribeSourceRunSlot(std::size_t source_index,
                           GGEMSSourceRecord const &record,
                           GGEMSSourceRunRange const &range,
                           GGEMSEnergyDistributionRecord const &energy_record,
                           std::span<std::uint64_t const> energy_values)
    -> std::string {

  return std::format("Source slot: {} | Projection primary begin: {} | {}",
                     source_index, range.projection_primary_begin,
                     DescribeSource(record, range.primary_count, energy_record,
                                    energy_values));
}

} // namespace ggems::core::sources
