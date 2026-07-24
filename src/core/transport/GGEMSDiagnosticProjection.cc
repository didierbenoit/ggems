#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <span>
#include <string_view>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/transport/GGEMSDiagnosticProjection.hh"

namespace {

// =============================================================================
// =============================================================================

constexpr std::uint32_t k_sign_mask{0x8000'0000U};
constexpr std::uint32_t k_exponent_mask{0x7F80'0000U};
constexpr std::uint32_t k_fraction_mask{0x007F'FFFFU};
constexpr std::uint32_t k_implicit_significand_bit{0x0080'0000U};
constexpr std::uint32_t k_non_finite_exponent{0xFFU};
constexpr std::uint64_t k_int64_min_magnitude{0x8000'0000'0000'0000ULL};
constexpr std::uint64_t k_max_binary32_significand{0x00FF'FFFFULL};

static_assert(k_max_binary32_significand <=
              std::numeric_limits<std::uint64_t>::max() /
                  ggems::core::transport::k_diagnostic_projection_distance_pm);

// =============================================================================
// =============================================================================

[[nodiscard]] auto TryRoundMagnitude(std::uint64_t product,
                                     std::int32_t exponent2,
                                     std::uint64_t magnitude_limit,
                                     std::uint64_t &magnitude) noexcept
    -> bool {
  std::uint64_t rounded_magnitude{0ULL};

  if (exponent2 >= 0) {
    auto const left_shift = static_cast<std::uint32_t>(exponent2);

    if (left_shift >= 64U || product > (magnitude_limit >> left_shift)) {
      return false;
    }

    rounded_magnitude = product << left_shift;
  } else {
    auto const right_shift = static_cast<std::uint32_t>(-exponent2);

    if (right_shift > 64U) {
      rounded_magnitude = 0ULL;
    } else if (right_shift == 64U) {
      rounded_magnitude = product >= k_int64_min_magnitude ? 1ULL : 0ULL;
    } else {
      std::uint64_t const quotient = product >> right_shift;
      std::uint64_t const remainder_mask = (1ULL << right_shift) - 1ULL;
      std::uint64_t const remainder = product & remainder_mask;
      std::uint64_t const half = 1ULL << (right_shift - 1U);

      rounded_magnitude = quotient + (remainder >= half ? 1ULL : 0ULL);
    }

    if (rounded_magnitude > magnitude_limit) {
      return false;
    }
  }

  magnitude = rounded_magnitude;
  return true;
}
} // namespace

namespace ggems::core::transport {

// =============================================================================
// =============================================================================

auto TryScaleDiagnosticProjectionComponent(
    float component, std::int64_t &displacement_pm) noexcept -> bool {
  auto const bits = std::bit_cast<std::uint32_t>(component);
  bool const negative = (bits & k_sign_mask) != 0U;
  std::uint32_t const exponent = (bits & k_exponent_mask) >> 23U;
  std::uint32_t const fraction = bits & k_fraction_mask;

  if (exponent == k_non_finite_exponent) {
    return false;
  }

  if (exponent == 0U && fraction == 0U) {
    displacement_pm = 0LL;
    return true;
  }

  std::uint64_t significand{0ULL};
  std::int32_t exponent2{0};

  if (exponent == 0U) {
    significand = fraction;
    exponent2 = -149;
  } else {
    significand = k_implicit_significand_bit | fraction;
    exponent2 = static_cast<std::int32_t>(exponent) - 150;
  }

  std::uint64_t const product =
      significand * k_diagnostic_projection_distance_pm;

  std::uint64_t const magnitude_limit =
      negative ? k_int64_min_magnitude
               : static_cast<std::uint64_t>(
                     std::numeric_limits<std::int64_t>::max());

  std::uint64_t magnitude{0ULL};

  if (!TryRoundMagnitude(product, exponent2, magnitude_limit, magnitude)) {
    return false;
  }

  std::int64_t result{0LL};

  if (!negative) {
    result = static_cast<std::int64_t>(magnitude);
  } else if (magnitude == k_int64_min_magnitude) {
    result = std::numeric_limits<std::int64_t>::min();
  } else {
    result = -static_cast<std::int64_t>(magnitude);
  }

  displacement_pm = result;
  return true;
}

// =============================================================================
// =============================================================================

auto TryAddDiagnosticProjectionDisplacement(std::int64_t position_pm,
                                            std::int64_t displacement_pm,
                                            std::int64_t &endpoint_pm) noexcept
    -> bool {
  if (displacement_pm > 0LL &&
      position_pm >
          std::numeric_limits<std::int64_t>::max() - displacement_pm) {
    return false;
  }

  if (displacement_pm < 0LL &&
      position_pm <
          std::numeric_limits<std::int64_t>::min() - displacement_pm) {
    return false;
  }

  endpoint_pm = position_pm + displacement_pm;
  return true;
}

// =============================================================================
// =============================================================================

auto ValidateDiagnosticTransportSources(
    std::span<sources::GGEMSSourceRecord const> source_records,
    std::span<sources::GGEMSSourceRunRange const> source_ranges) -> void {
  GGEMS_CHECK_RECOVERABLE(
      source_records.size() == source_ranges.size(),
      "Diagnostic transport source record and range counts must match.");

  constexpr std::array<std::string_view, 3U> k_axis_names{"X", "Y", "Z"};

  for (std::size_t source_index = 0U; source_index < source_records.size();
       ++source_index) {
    if (source_ranges[source_index].primary_count == 0ULL) {
      continue;
    }

    auto const &source = source_records[source_index];

    sources::GGEMSSourceType const source_type =
        sources::FromKernelSourceType(source.source_type);

    GGEMS_CHECK_RECOVERABLE(
        source_type == sources::GGEMSSourceType::Analytic,
        std::format(
            "Diagnostic transport supports only Analytic sources; source "
            "slot {} has type {}.",
            source_index, sources::ToLongName(source_type)));

    std::array<float, 3U> const direction{source.axis_z_x, source.axis_z_y,
                                          source.axis_z_z};

    std::array<std::int64_t, 3U> const position{
        source.position_x_pm, source.position_y_pm, source.position_z_pm};

    for (std::size_t axis = 0U; axis < direction.size(); ++axis) {
      std::int64_t displacement_pm{0LL};

      GGEMS_CHECK_RECOVERABLE(
          TryScaleDiagnosticProjectionComponent(direction[axis],
                                                displacement_pm),
          std::format(
              "Diagnostic projection cannot scale source slot {} axis_z.{} "
              "into int64 picometres.",
              source_index, k_axis_names[axis]));

      std::int64_t endpoint_pm{0LL};

      GGEMS_CHECK_RECOVERABLE(
          TryAddDiagnosticProjectionDisplacement(position[axis],
                                                 displacement_pm, endpoint_pm),
          std::format(
              "Diagnostic projection endpoint overflows int64 at source "
              "slot {} axis {}.",
              source_index, k_axis_names[axis]));
    }
  }
}
} // namespace ggems::core::transport
