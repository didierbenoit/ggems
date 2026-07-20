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
} // namespace

// =============================================================================
// =============================================================================

auto DescribeSource(GGEMSSourceRecord const &record,
                    std::uint64_t primary_count) -> std::string {
  GGEMSSourceType const source_type = FromKernelSourceType(record.source_type);

  particles::GGEMSParticleType const particle_type =
      particles::FromKernelParticleType(record.emitted_particle_type);

  return std::format(
      "Type: {} | State: {} | Primary count: {} | Particle: {} ({}) | "
      "Energy: {} | {} | Position: ({}, {}, {}) | Direction: ({}, {}, {}) | "
      "Weight: {}",
      ToLongName(source_type), primary_count == 0ULL ? "Disabled" : "Active",
      primary_count, particles::ToLongName(particle_type),
      particles::ToShortName(particle_type),
      ggems::units::HumanReadable(ggems::units::Energy{record.energy_milli_eV}),
      DescribeTime(record),
      ggems::units::HumanReadableSignedLength(record.position_x_pm),
      ggems::units::HumanReadableSignedLength(record.position_y_pm),
      ggems::units::HumanReadableSignedLength(record.position_z_pm),
      record.direction_x, record.direction_y, record.direction_z,
      record.weight);
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
