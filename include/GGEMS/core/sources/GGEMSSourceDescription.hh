#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace ggems::core::sources {
[[nodiscard]] auto DescribeSource(GGEMSSourceRecord const &record,
                                  std::uint64_t primary_count) -> std::string;

[[nodiscard]] auto DescribeSourceRunSlot(std::size_t source_index,
                                         GGEMSSourceRecord const &record,
                                         GGEMSSourceRunRange const &range)
    -> std::string;
} // namespace ggems::core::sources
