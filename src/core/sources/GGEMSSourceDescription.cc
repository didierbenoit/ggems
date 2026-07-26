#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <span>

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/sources/GGEMSSourceValidation.hh"

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
    GGEMS_CHECK_INTERNAL(source_record.energy_milli_eV > 0ULL &&
                             energy_record.table_offset == 0ULL &&
                             energy_record.table_count == 0U &&
                             energy_record.regular_bin_width_milli_eV == 0ULL,
                         "Invalid Mono energy description record.");

    return std::format("Energy: Mono ({})",
                       ggems::units::HumanReadable(ggems::units::Energy{
                           source_record.energy_milli_eV}));
  }

  GGEMS_CHECK_INTERNAL(
      distribution_type == GGEMSEnergyDistributionType::DiscreteLines ||
          distribution_type == GGEMSEnergyDistributionType::RegularSpectrum,
      "Unsupported source energy distribution in description.");

  GGEMS_CHECK_INTERNAL(energy_record.table_offset <=
                           static_cast<std::uint64_t>(energy_values.size()),
                       "Source energy description table offset is invalid.");

  auto const table_offset =
      static_cast<std::size_t>(energy_record.table_offset);
  auto const table_count = static_cast<std::size_t>(energy_record.table_count);

  GGEMS_CHECK_INTERNAL(table_count >= 2U &&
                           table_count <= energy_values.size() - table_offset,
                       "Source energy description table range is invalid.");

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
                         energy_record.regular_bin_width_milli_eV}));
}

} // namespace

// =============================================================================
// =============================================================================

auto DescribeSource(GGEMSSourceRecord const &record,
                    std::uint64_t primary_count) -> std::string {
  GGEMS_CHECK_INTERNAL(
      record.energy_milli_eV > 0ULL,
      "The energy-aware DescribeSource overload is required for a "
      "table-backed source.");

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
  auto const &energy_records = snapshot.GetEnergyDistributionRecords();

  GGEMS_CHECK_INTERNAL(source_index < records.size() &&
                           source_index < ranges.size() &&
                           source_index < energy_records.size(),
                       "Source description index is outside the run snapshot.");

  return DescribeSourceRunSlot(source_index, records[source_index],
                               ranges[source_index],
                               energy_records[source_index],
                               snapshot.GetEnergyValuesMilliElectronVolt());
}

// =============================================================================
// =============================================================================

auto DescribeSourceRunSlot(std::size_t source_index,
                           GGEMSSourceRecord const &record,
                           GGEMSSourceRunRange const &range) -> std::string {
  GGEMS_CHECK_INTERNAL(
      record.energy_milli_eV > 0ULL,
      "The energy-aware DescribeSourceRunSlot overload is required for a "
      "table-backed source.");

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
