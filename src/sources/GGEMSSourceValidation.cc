#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <cstddef>
#include <algorithm>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/sources/GGEMSSourceFrame.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/sources/GGEMSSourceValidation.hh"

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

[[nodiscard]] auto
ComputeSpatialComponentScale(GGEMSSourceRecord const &record) noexcept
    -> long double {
  auto const row_norm = [](float x, float y, float z) noexcept -> long double {
    return std::hypot(static_cast<long double>(x), static_cast<long double>(y),
                      static_cast<long double>(z));
  };

  return std::max(
      {row_norm(record.axis_x_x, record.axis_y_x, record.axis_z_x),
       row_norm(record.axis_x_y, record.axis_y_y, record.axis_z_y),
       row_norm(record.axis_x_z, record.axis_y_z, record.axis_z_z)});
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto SignedDifferenceToLongDouble(std::int64_t lhs,
                                                std::int64_t rhs) noexcept
    -> long double {
  auto const lhs_ordinal =
      static_cast<std::uint64_t>(lhs) ^ k_signed_ordinal_bias;
  auto const rhs_ordinal =
      static_cast<std::uint64_t>(rhs) ^ k_signed_ordinal_bias;

  if (lhs_ordinal >= rhs_ordinal) {
    return static_cast<long double>(lhs_ordinal - rhs_ordinal);
  }

  return -static_cast<long double>(rhs_ordinal - lhs_ordinal);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto IntegerRoundingBand(float x, float y, float z) noexcept
    -> long double {
  return 0.5L * (std::abs(static_cast<long double>(x)) +
                 std::abs(static_cast<long double>(y)) +
                 std::abs(static_cast<long double>(z)));
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

[[nodiscard]] auto
IsVolumeGeometry(GGEMSEmissionGeometryType geometry_type) noexcept -> bool {
  return geometry_type == GGEMSEmissionGeometryType::Box ||
         geometry_type == GGEMSEmissionGeometryType::Sphere ||
         geometry_type == GGEMSEmissionGeometryType::Cylinder;
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

  if (geometry_type == GGEMSEmissionGeometryType::Point) {
    if (focus_equals_origin) {
      throw ggems::core::GGEMSRecoverable(
          "A Focused Point source must not target its emission position.");
    }
    return;
  }

  long double const delta_x = SignedDifferenceToLongDouble(
      record.focus_position_x_pm, record.position_x_pm);
  long double const delta_y = SignedDifferenceToLongDouble(
      record.focus_position_y_pm, record.position_y_pm);
  long double const delta_z = SignedDifferenceToLongDouble(
      record.focus_position_z_pm, record.position_z_pm);

  if (IsVolumeGeometry(geometry_type)) {
    long double const local_x = (delta_x * record.axis_x_x) +
                                (delta_y * record.axis_x_y) +
                                (delta_z * record.axis_x_z);
    long double const local_y = (delta_x * record.axis_y_x) +
                                (delta_y * record.axis_y_y) +
                                (delta_z * record.axis_y_z);
    long double const local_z = (delta_x * record.axis_z_x) +
                                (delta_y * record.axis_z_y) +
                                (delta_z * record.axis_z_z);

    long double const transform_band =
        static_cast<long double>(bounds.component_radius_pm) *
        k_binary32_sampling_margin;
    long double const band_x =
        transform_band +
        IntegerRoundingBand(record.axis_x_x, record.axis_x_y, record.axis_x_z);
    long double const band_y =
        transform_band +
        IntegerRoundingBand(record.axis_y_x, record.axis_y_y, record.axis_y_z);
    long double const band_z =
        transform_band +
        IntegerRoundingBand(record.axis_z_x, record.axis_z_y, record.axis_z_z);

    bool focus_in_reachable_support{false};

    if (geometry_type == GGEMSEmissionGeometryType::Box) {
      focus_in_reachable_support =
          std::abs(local_x) <= bounds.half_extent_x_pm + band_x &&
          std::abs(local_y) <= bounds.half_extent_y_pm + band_y &&
          std::abs(local_z) <= bounds.half_extent_z_pm + band_z;
    } else if (geometry_type == GGEMSEmissionGeometryType::Sphere) {
      long double const radial_band = std::hypot(band_x, band_y, band_z);
      focus_in_reachable_support = std::hypot(local_x, local_y, local_z) <=
                                   bounds.half_extent_x_pm + radial_band;
    } else {
      long double const radial_band = std::hypot(band_x, band_y);
      focus_in_reachable_support =
          std::hypot(local_x, local_y) <=
              bounds.half_extent_x_pm + radial_band &&
          std::abs(local_z) <= bounds.half_extent_z_pm + band_z;
    }

    if (focus_in_reachable_support) {
      throw ggems::core::GGEMSRecoverable(
          "A Focused volume source must target a point outside its reachable "
        "emission support.");
    }
    return;
  }

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

  if (!(std::abs(normal_distance) > plane_band)) {
    throw ggems::core::GGEMSRecoverable(
        "A Focused Rectangle or Ellipse source must target a point outside "
      "its emission plane.");
  }
}
} // namespace

// =============================================================================
// =============================================================================

auto HasSignedPicoMeterEnvelope(std::int64_t center_pm,
                                std::uint64_t radius_pm) noexcept -> bool {
  std::uint64_t const ordinal =
      static_cast<std::uint64_t>(center_pm) ^ k_signed_ordinal_bias;

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
    if (!(record.geometry_size_x_pm == 0ULL &&
            record.geometry_size_y_pm == 0ULL &&
            record.geometry_size_z_pm == 0ULL)) {
      throw ggems::core::GGEMSRecoverable("Point emission geometry must have zero dimensions.");
    }
    return {};
  }

  bool const planar = geometry_type == GGEMSEmissionGeometryType::Rectangle ||
                      geometry_type == GGEMSEmissionGeometryType::Ellipse;
  bool const volume = IsVolumeGeometry(geometry_type);
  if (!(planar || volume)) {
    throw ggems::core::GGEMSRecoverable("Unsupported analytic emission geometry type.");
  }

  auto const size_x_pm = static_cast<long double>(record.geometry_size_x_pm);
  auto const size_y_pm = static_cast<long double>(record.geometry_size_y_pm);
  auto const size_z_pm = static_cast<long double>(record.geometry_size_z_pm);

  long double half_x_pm{0.0L};
  long double half_y_pm{0.0L};
  long double half_z_pm{0.0L};
  long double mathematical_radius_pm{0.0L};
  long double basis_scale{1.0L};

  if (planar) {
    if (!(record.geometry_size_x_pm > 0ULL && record.geometry_size_y_pm > 0ULL &&
            record.geometry_size_z_pm == 0ULL)) {
      throw ggems::core::GGEMSRecoverable(
          "Rectangle and Ellipse dimensions must be strictly positive in X/Y "
        "and zero in Z.");
    }
    half_x_pm = 0.5L * size_x_pm;
    half_y_pm = 0.5L * size_y_pm;
    mathematical_radius_pm = 0.5L * std::hypot(size_x_pm, size_y_pm);
    basis_scale = ComputePlanarBasisScale(record);
  } else if (geometry_type == GGEMSEmissionGeometryType::Box) {
    if (!(record.geometry_size_x_pm > 0ULL &&
                                record.geometry_size_y_pm > 0ULL &&
                                record.geometry_size_z_pm > 0ULL)) {
      throw ggems::core::GGEMSRecoverable("Box dimensions must be strictly positive.");
    }
    half_x_pm = 0.5L * size_x_pm;
    half_y_pm = 0.5L * size_y_pm;
    half_z_pm = 0.5L * size_z_pm;
    mathematical_radius_pm = 0.5L * std::hypot(size_x_pm, size_y_pm, size_z_pm);
    basis_scale = ComputeSpatialComponentScale(record);
  } else if (geometry_type == GGEMSEmissionGeometryType::Sphere) {
    if (!(record.geometry_size_x_pm > 0ULL &&
            record.geometry_size_y_pm == record.geometry_size_x_pm &&
            record.geometry_size_z_pm == record.geometry_size_x_pm)) {
      throw ggems::core::GGEMSRecoverable(
          "Sphere dimensions must contain one repeated strictly positive "
        "diameter.");
    }
    half_x_pm = 0.5L * size_x_pm;
    half_y_pm = half_x_pm;
    half_z_pm = half_x_pm;
    mathematical_radius_pm = half_x_pm;
    basis_scale = ComputeSpatialComponentScale(record);
  } else {
    if (!(record.geometry_size_x_pm > 0ULL &&
            record.geometry_size_y_pm == record.geometry_size_x_pm &&
            record.geometry_size_z_pm > 0ULL)) {
      throw ggems::core::GGEMSRecoverable(
          "Cylinder dimensions must contain a repeated positive diameter and "
        "a positive height.");
    }
    half_x_pm = 0.5L * size_x_pm;
    half_y_pm = half_x_pm;
    half_z_pm = 0.5L * size_z_pm;
    mathematical_radius_pm = 0.5L * std::hypot(size_x_pm, size_z_pm);
    basis_scale = ComputeSpatialComponentScale(record);
  }

  long double const transformed_radius_pm =
      mathematical_radius_pm * basis_scale * ComputePlanarBasisScale(record) *
      (1.0L + k_binary32_sampling_margin);

  long double const rounded_radius_pm = std::ceil(std::nextafter(
      transformed_radius_pm, std::numeric_limits<long double>::infinity()));

  long double const uint64_upper_exclusive = std::ldexp(1.0L, 64);

  if (!(std::isfinite(rounded_radius_pm) && rounded_radius_pm >= 0.0L &&
          rounded_radius_pm < uint64_upper_exclusive)) {
    throw ggems::core::GGEMSRecoverable(
        "Emission geometry radius exceeds uint64 picometer storage.");
  }

  return {.component_radius_pm = static_cast<std::uint64_t>(rounded_radius_pm),
          .half_extent_x_pm = half_x_pm,
          .half_extent_y_pm = half_y_pm,
          .half_extent_z_pm = half_z_pm};
}

// =============================================================================
// =============================================================================

auto IsDefaultFullSphereIsotropicDomain(
    GGEMSSourceRecord const &record) noexcept -> bool {
  return record.isotropic_cos_theta_lower ==
             k_isotropic_full_sphere_cos_theta_lower &&
         record.isotropic_cos_theta_upper ==
             k_isotropic_full_sphere_cos_theta_upper &&
         record.isotropic_phi_min_rad == k_isotropic_full_sphere_phi_min_rad &&
         record.isotropic_phi_max_rad == k_isotropic_full_sphere_phi_max_rad;
}

// =============================================================================
// =============================================================================

auto ValidateAnalyticSourceRecord(GGEMSSourceRecord const &record) -> void {
  if (!(FromKernelSourceType(record.source_type) == GGEMSSourceType::Analytic)) {
    throw ggems::core::GGEMSRecoverable("Current source sampling supports only Analytic sources.");
  }

  if (!(IsValidSourceFrame(BuildRecordFrame(record)))) {
    throw ggems::core::GGEMSRecoverable(
        "Source record must contain a finite, orthonormal, right-handed "
      "binary32 frame.");
  }

  GGEMSEmissionBounds const bounds = BuildEmissionBounds(record);

  constexpr std::array<char const *, 3U> k_axis_names{"X", "Y", "Z"};
  std::array<std::int64_t, 3U> const positions{
      record.position_x_pm, record.position_y_pm, record.position_z_pm};

  for (std::size_t axis = 0U; axis < positions.size(); ++axis) {
    if (!(HasSignedPicoMeterEnvelope(positions[axis], bounds.component_radius_pm))) {
      throw ggems::core::GGEMSRecoverable(
          std::format("Emission geometry exceeds int64 picometer storage "
                    "around source axis {}.",
                    k_axis_names[axis]));
    }
  }

  GGEMSAngularDistributionType const distribution_type =
      FromKernelAngularDistributionType(record.angular_distribution_type);

  if (!(distribution_type !=
                              GGEMSAngularDistributionType::Unknown)) {
    throw ggems::core::GGEMSRecoverable("Unsupported analytic angular distribution type.");
  }

  bool const finite_domain = std::isfinite(record.isotropic_cos_theta_lower) &&
                             std::isfinite(record.isotropic_cos_theta_upper) &&
                             std::isfinite(record.isotropic_phi_min_rad) &&
                             std::isfinite(record.isotropic_phi_max_rad);
  long double const phi_width =
      static_cast<long double>(record.isotropic_phi_max_rad) -
      static_cast<long double>(record.isotropic_phi_min_rad);

  if (!(finite_domain && record.isotropic_cos_theta_lower >= -1.0F &&
          record.isotropic_cos_theta_lower < record.isotropic_cos_theta_upper &&
          record.isotropic_cos_theta_upper <= 1.0F &&
          record.isotropic_phi_min_rad < record.isotropic_phi_max_rad &&
          std::isfinite(phi_width) &&
          phi_width <=
              static_cast<long double>(k_isotropic_full_sphere_phi_max_rad))) {
    throw ggems::core::GGEMSRecoverable("Invalid binary32 Isotropic angular domain.");
  }

  if (distribution_type != GGEMSAngularDistributionType::Isotropic) {
    if (!(IsDefaultFullSphereIsotropicDomain(record))) {
      throw ggems::core::GGEMSRecoverable(
          "Fixed and Focused source records must store the canonical unused "
        "Isotropic domain.");
    }
  }

  if (distribution_type == GGEMSAngularDistributionType::Focused) {
    ValidateFocusedDistribution(
        record, FromKernelEmissionGeometryType(record.emission_geometry_type),
        bounds);
    return;
  }

  if (!(IsZeroFocus(record))) {
    throw ggems::core::GGEMSRecoverable(
        "Fixed and Isotropic source records must store a zero focus.");
  }
}
} // namespace ggems::core::sources
