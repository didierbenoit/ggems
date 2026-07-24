#include <cstddef>
#include <cstdint>
#include <format>
#include <string>

#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"

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

  return std::format("Emission: {}", ToLongName(geometry_type));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeAngularDistribution(GGEMSSourceRecord const &record)
    -> std::string {
  GGEMSAngularDistributionType const distribution_type =
      FromKernelAngularDistributionType(record.angular_distribution_type);

  if (distribution_type != GGEMSAngularDistributionType::Focused) {
    return std::format("Angular: {}", ToLongName(distribution_type));
  }

  return std::format(
      "Angular: Focused | Focus: ({}, {}, {})",
      ggems::units::HumanReadableSignedLength(record.focus_position_x_pm),
      ggems::units::HumanReadableSignedLength(record.focus_position_y_pm),
      ggems::units::HumanReadableSignedLength(record.focus_position_z_pm));
}
} // namespace

// =============================================================================
// =============================================================================

auto DescribeSource(GGEMSSourceRecord const &record,
                    std::uint64_t primary_count) -> std::string {
  GGEMSSourceType const source_type = FromKernelSourceType(record.source_type);

  particles::GGEMSParticleType const particle_type =
      particles::FromKernelParticleType(record.emitted_particle_type);

  return std::format(
      "Type: {} | Primary count: {} | Particle: {} ({}) | "
      "{} | {} | Energy: {} | {} | Position: ({}, {}, {}) | "
      "Axis Z: ({}, {}, {}) | Weight: {}",
      ToLongName(source_type), primary_count,
      particles::ToLongName(particle_type),
      particles::ToShortName(particle_type), DescribeEmission(record),
      DescribeAngularDistribution(record),
      ggems::units::HumanReadable(ggems::units::Energy{record.energy_milli_eV}),
      DescribeTime(record),
      ggems::units::HumanReadableSignedLength(record.position_x_pm),
      ggems::units::HumanReadableSignedLength(record.position_y_pm),
      ggems::units::HumanReadableSignedLength(record.position_z_pm),
      record.axis_z_x, record.axis_z_y, record.axis_z_z, record.weight);
}

// =============================================================================
// =============================================================================

auto DescribeSourceRunSlot(std::size_t source_index,
                           GGEMSSourceRecord const &record,
                           GGEMSSourceRunRange const &range) -> std::string {
  return std::format("Source slot: {} | Projection primary begin: {} | {}",
                     source_index, range.projection_primary_begin,
                     DescribeSource(record, range.primary_count));
}

} // namespace ggems::core::sources
