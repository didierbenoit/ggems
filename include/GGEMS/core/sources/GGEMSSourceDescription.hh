#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace ggems::core::sources {
class GGEMSSourceRunSnapshot;

[[nodiscard]] auto DescribeSource(GGEMSSourceRecord const &record,
                                  std::uint64_t primary_count) -> std::string;

[[nodiscard]] auto
DescribeSource(GGEMSSourceRecord const &record, std::uint64_t primary_count,
               GGEMSEnergyDistributionRecord const &energy_record,
               std::span<std::uint64_t const> energy_values) -> std::string;

[[nodiscard]] auto DescribeSourceRunSlot(std::size_t source_index,
                                         GGEMSSourceRunSnapshot const &snapshot)
    -> std::string;

[[nodiscard]] auto DescribeSourceRunSlot(std::size_t source_index,
                                         GGEMSSourceRecord const &record,
                                         GGEMSSourceRunRange const &range)
    -> std::string;

[[nodiscard]] auto
DescribeSourceRunSlot(std::size_t source_index, GGEMSSourceRecord const &record,
                      GGEMSSourceRunRange const &range,
                      GGEMSEnergyDistributionRecord const &energy_record,
                      std::span<std::uint64_t const> energy_values)
    -> std::string;
} // namespace ggems::core::sources
