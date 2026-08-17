#pragma once

#include <cstdint>
#include <span>

#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"

namespace ggems::core::transport {
inline constexpr std::uint64_t k_diagnostic_projection_distance_pm{
    1'000'000'000'000ULL};

[[nodiscard]] auto TryScaleDiagnosticProjectionComponent(
    float component, std::int64_t &displacement_pm) noexcept -> bool;

[[nodiscard]] auto TryAddDiagnosticProjectionDisplacement(
    std::int64_t position_pm, std::int64_t displacement_pm,
    std::int64_t &endpoint_pm) noexcept -> bool;

auto ValidateDiagnosticTransportSources(
    std::span<sources::GGEMSSourceRecord const> source_records,
    std::span<sources::GGEMSSourceRunRange const> source_ranges) -> void;
} // namespace ggems::core::transport
