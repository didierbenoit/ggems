#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <cstddef>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceFrame.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceValidation.hh"

namespace ggems::core::sources {
namespace {

// =============================================================================
// =============================================================================

constexpr long double k_binary32_sampling_margin{1.0e-5L};
constexpr std::uint64_t k_signed_ordinal_bias{0x8000'0000'0000'0000ULL};

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildRecordFrame(GGEMSSourceRecord const &record) noexcept
    -> GGEMSSourceFrame {
  return {.axis_x = {.x = record.axis_x_x,
                     .y = record.axis_x_y,
                     .z = record.axis_x_z},
          .axis_y = {.x = record.axis_y_x,
                     .y = record.axis_y_y,
                     .z = record.axis_y_z},
          .axis_z = {.x = record.axis_z_x,
                     .y = record.axis_z_y,
                     .z = record.axis_z_z}};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ComputePlanarBasisScale(GGEMSSourceRecord const &record) noexcept
    -> long double {
  long double const axis_x_squared =
      (static_cast<long double>(record.axis_x_x) * record.axis_x_x) +
      (static_cast<long double>(record.axis_x_y) * record.axis_x_y) +
      (static_cast<long double>(record.axis_x_z) * record.axis_x_z);

  long double const axis_y_squared =
      (static_cast<long double>(record.axis_y_x) * record.axis_y_x) +
      (static_cast<long double>(record.axis_y_y) * record.axis_y_y) +
      (static_cast<long double>(record.axis_y_z) * record.axis_y_z);

  long double const axes_dot =
      (static_cast<long double>(record.axis_x_x) * record.axis_y_x) +
      (static_cast<long double>(record.axis_x_y) * record.axis_y_y) +
      (static_cast<long double>(record.axis_x_z) * record.axis_y_z);

  long double const largest_eigenvalue =
      0.5L * (axis_x_squared + axis_y_squared +
              std::hypot(axis_x_squared - axis_y_squared, 2.0L * axes_dot));

  return std::sqrt(largest_eigenvalue);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto IsZeroFocus(GGEMSSourceRecord const &record) noexcept
    -> bool {
  return record.focus_position_x_pm == 0ULL &&
         record.focus_position_y_pm == 0ULL &&
         record.focus_position_z_pm == 0ULL;
}

// =============================================================================
// =============================================================================

auto ValidateFocusedDistribution(GGEMSSourceRecord const &record,
                                 GGEMSEmissionGeometryType geometry_type,
                                 GGEMSEmissionBounds const bounds) -> void {
  bool const focus_equals_origin =
      record.focus_position_x_pm == record.position_x_pm &&
      record.focus_position_y_pm == record.position_y_pm &&
      record.focus_position_z_pm == record.position_z_pm;

  GGEMS_CHECK_RECOVERABLE(
      !focus_equals_origin,
      "A Focused Point source must not target its emission position.");

  if (geometry_type == GGEMSEmissionGeometryType::Point) {
    return;
  }

  long double const delta_x =
      static_cast<long double>(record.focus_position_x_pm) -
      static_cast<long double>(record.position_x_pm);
  long double const delta_y =
      static_cast<long double>(record.focus_position_y_pm) -
      static_cast<long double>(record.position_y_pm);
  long double const delta_z =
      static_cast<long double>(record.focus_position_z_pm) -
      static_cast<long double>(record.position_z_pm);

  long double const normal_distance = (delta_x * record.axis_z_x) +
                                      (delta_y * record.axis_z_y) +
                                      (delta_z * record.axis_z_z);

  long double const normal_axis_x =
      (static_cast<long double>(record.axis_z_x) * record.axis_x_x) +
      (static_cast<long double>(record.axis_z_y) * record.axis_x_y) +
      (static_cast<long double>(record.axis_z_z) * record.axis_x_z);

  long double const normal_axis_y =
      (static_cast<long double>(record.axis_z_x) * record.axis_y_x) +
      (static_cast<long double>(record.axis_z_y) * record.axis_y_y) +
      (static_cast<long double>(record.axis_z_z) * record.axis_y_z);

  long double const integer_rounding_band =
      0.5L * (std::abs(static_cast<long double>(record.axis_z_x)) +
              std::abs(static_cast<long double>(record.axis_z_y)) +
              std::abs(static_cast<long double>(record.axis_z_z)));

  long double const plane_band =
      (bounds.half_extent_x_pm * std::abs(normal_axis_x)) +
      (bounds.half_extent_y_pm * std::abs(normal_axis_y)) +
      (static_cast<long double>(bounds.component_radius_pm) *
       k_binary32_sampling_margin) +
      integer_rounding_band;

  GGEMS_CHECK_RECOVERABLE(
      std::abs(normal_distance) > plane_band,
      "A Focused Rectangle or Ellipse source must target a point outside "
      "its emission plane.");
}
} // namespace

// =============================================================================
// =============================================================================

auto HasSignedPicoMetreEnvelope(std::int64_t centre_pm,
                                std::uint64_t radius_pm) noexcept -> bool {
  std::uint64_t const ordinal =
      static_cast<std::uint64_t>(centre_pm) ^ k_signed_ordinal_bias;

  return radius_pm <= ordinal &&
         radius_pm <= std::numeric_limits<std::uint64_t>::max() - ordinal;
}

// =============================================================================
// =============================================================================

auto BuildEmissionBounds(GGEMSSourceRecord const &record)
    -> GGEMSEmissionBounds {
  GGEMSEmissionGeometryType const geometry_type =
      FromKernelEmissionGeometryType(record.emission_geometry_type);

  if (geometry_type == GGEMSEmissionGeometryType::Point) {
    GGEMS_CHECK_RECOVERABLE(
        record.geometry_size_x_pm == 0ULL && record.geometry_size_y_pm == 0ULL,
        "Point emission geometry must have zero dimensions.");

    return {};
  }

  GGEMS_CHECK_RECOVERABLE(
      geometry_type == GGEMSEmissionGeometryType::Rectangle ||
          geometry_type == GGEMSEmissionGeometryType::Ellipse,
      "Unsupported analytic emission geometry type.");

  GGEMS_CHECK_RECOVERABLE(
      record.geometry_size_x_pm > 0ULL && record.geometry_size_y_pm > 0ULL,
      "Rectangle and Ellipse dimensions must be strictly positive.");

  auto const size_x_pm = static_cast<long double>(record.geometry_size_x_pm);
  auto const size_y_pm = static_cast<long double>(record.geometry_size_y_pm);

  long double const mathematical_radius_pm =
      0.5L * std::hypot(size_x_pm, size_y_pm);

  long double const transformed_radius_pm = mathematical_radius_pm *
                                            ComputePlanarBasisScale(record) *
                                            (1.0L + k_binary32_sampling_margin);

  long double const rounded_radius_pm = std::ceil(std::nextafter(
      transformed_radius_pm, std::numeric_limits<long double>::infinity()));

  long double const uint64_upper_exclusive = std::ldexp(1.0L, 64);

  GGEMS_CHECK_RECOVERABLE(
      std::isfinite(rounded_radius_pm) && rounded_radius_pm >= 0.0L &&
          rounded_radius_pm < uint64_upper_exclusive,
      "Emission geometry radius exceeds uint64 picometre storage.");

  return {.component_radius_pm = static_cast<std::uint64_t>(rounded_radius_pm),
          .half_extent_x_pm = 0.5L * size_x_pm,
          .half_extent_y_pm = 0.5L * size_y_pm};
}

// =============================================================================
// =============================================================================

auto ValidateAnalyticSourceRecord(GGEMSSourceRecord const &record) -> void {
  GGEMS_CHECK_RECOVERABLE(
      FromKernelSourceType(record.source_type) == GGEMSSourceType::Analytic,
      "Current source sampling supports only Analytic sources.");

  GGEMS_CHECK_RECOVERABLE(
      IsValidSourceFrame(BuildRecordFrame(record)),
      "Source record must contain a finite, orthonormal, right-handed "
      "binary32 frame.");

  GGEMSEmissionBounds const bounds = BuildEmissionBounds(record);

  constexpr std::array<char const *, 3U> k_axis_names{"X", "Y", "Z"};
  std::array<std::int64_t, 3U> const positions{
      record.position_x_pm, record.position_y_pm, record.position_z_pm};

  for (std::size_t axis = 0U; axis < positions.size(); ++axis) {
    GGEMS_CHECK_RECOVERABLE(
        HasSignedPicoMetreEnvelope(positions[axis], bounds.component_radius_pm),
        std::format("Emission geometry exceeds int64 picometre storage "
                    "around source axis {}.",
                    k_axis_names[axis]));
  }

  GGEMSAngularDistributionType const distribution_type =
      FromKernelAngularDistributionType(record.angular_distribution_type);

  GGEMS_CHECK_RECOVERABLE(distribution_type !=
                              GGEMSAngularDistributionType::Unknown,
                          "Unsupported analytic angular distribution type.");

  if (distribution_type == GGEMSAngularDistributionType::Focused) {
    ValidateFocusedDistribution(
        record, FromKernelEmissionGeometryType(record.emission_geometry_type),
        bounds);
    return;
  }

  GGEMS_CHECK_RECOVERABLE(
      IsZeroFocus(record),
      "Fixed and Isotropic source records must store a zero focus.");
}
} // namespace ggems::core::sources
