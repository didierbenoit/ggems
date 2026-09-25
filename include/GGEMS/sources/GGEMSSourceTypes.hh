#pragma once

#include <cstdint>
#include <string_view>

namespace ggems::core::sources {

enum class GGEMSSourceType : std::uint8_t {
  Unknown = 0U,
  Analytic = 1U,
  Voxelized = 2U,
  PhaseSpace = 3U,
};

enum class GGEMSEmissionGeometryType : std::uint8_t {
  Unknown = 0U,
  Point = 1U,
  Rectangle = 2U,
  Ellipse = 3U,
  Box = 4U,
  Sphere = 5U,
  Cylinder = 6U,
};

enum class GGEMSAngularDistributionType : std::uint8_t {
  Unknown = 0U,
  Fixed = 1U,
  Isotropic = 2U,
  Focused = 3U,
};

enum class GGEMSEnergyDistributionType : std::uint8_t {
  Unknown = 0U,
  Mono = 1U,
  DiscreteLines = 2U,
  RegularSpectrum = 3U,
};

[[nodiscard]] constexpr auto
ToKernelSourceType(GGEMSSourceType source_type) noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(source_type);
}

[[nodiscard]] constexpr auto
FromKernelSourceType(std::uint32_t source_type) noexcept -> GGEMSSourceType {
  return static_cast<GGEMSSourceType>(source_type);
}

[[nodiscard]] constexpr auto ToLongName(GGEMSSourceType source_type) noexcept
  -> std::string_view {
  switch (source_type) {
  case GGEMSSourceType::Unknown:
    return "Unknown";
  case GGEMSSourceType::Analytic:
    return "Analytic";
  case GGEMSSourceType::Voxelized:
    return "Voxelized";
  case GGEMSSourceType::PhaseSpace:
    return "PhaseSpace";
  }

  return "Unknown";
}

[[nodiscard]] constexpr auto ToKernelEnergyDistributionType(
  GGEMSEnergyDistributionType distribution_type) noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(distribution_type);
}

[[nodiscard]] constexpr auto
FromKernelEnergyDistributionType(std::uint32_t distribution_type) noexcept
  -> GGEMSEnergyDistributionType {
  return static_cast<GGEMSEnergyDistributionType>(distribution_type);
}

[[nodiscard]] constexpr auto
ToLongName(GGEMSEnergyDistributionType distribution_type) noexcept
  -> std::string_view {
  switch (distribution_type) {
  case GGEMSEnergyDistributionType::Mono:
    return "Mono";
  case GGEMSEnergyDistributionType::DiscreteLines:
    return "Discrete lines";
  case GGEMSEnergyDistributionType::RegularSpectrum:
    return "Regular spectrum";
  case GGEMSEnergyDistributionType::Unknown:
    return "Unknown";
  }

  return "Unknown";
}

[[nodiscard]] constexpr auto
ToKernelEmissionGeometryType(GGEMSEmissionGeometryType geometry_type) noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(geometry_type);
}

[[nodiscard]] constexpr auto
FromKernelEmissionGeometryType(std::uint32_t geometry_type) noexcept
  -> GGEMSEmissionGeometryType {
  return static_cast<GGEMSEmissionGeometryType>(geometry_type);
}

[[nodiscard]] constexpr auto
ToLongName(GGEMSEmissionGeometryType geometry_type) noexcept
  -> std::string_view {
  switch (geometry_type) {
  case GGEMSEmissionGeometryType::Point:
    return "Point";
  case GGEMSEmissionGeometryType::Rectangle:
    return "Rectangle";
  case GGEMSEmissionGeometryType::Ellipse:
    return "Ellipse";
  case GGEMSEmissionGeometryType::Box:
    return "Box";
  case GGEMSEmissionGeometryType::Sphere:
    return "Sphere";
  case GGEMSEmissionGeometryType::Cylinder:
    return "Cylinder";
  case GGEMSEmissionGeometryType::Unknown:
    return "Unknown";
  }

  return "Unknown";
}

[[nodiscard]] constexpr auto ToKernelAngularDistributionType(
  GGEMSAngularDistributionType distribution_type) noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(distribution_type);
}

[[nodiscard]] constexpr auto
FromKernelAngularDistributionType(std::uint32_t distribution_type) noexcept
  -> GGEMSAngularDistributionType {
  return static_cast<GGEMSAngularDistributionType>(distribution_type);
}

[[nodiscard]] constexpr auto
ToLongName(GGEMSAngularDistributionType distribution_type) noexcept
  -> std::string_view {
  switch (distribution_type) {
  case GGEMSAngularDistributionType::Fixed:
    return "Fixed";
  case GGEMSAngularDistributionType::Isotropic:
    return "Isotropic";
  case GGEMSAngularDistributionType::Focused:
    return "Focused";
  case GGEMSAngularDistributionType::Unknown:
    return "Unknown";
  }

  return "Unknown";
}

} // namespace ggems::core::sources
