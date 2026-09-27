// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Defines source-law kinds and their host/device encodings.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <string_view>

/*!
 * \namespace ggems::core::sources
 * \brief Configures source birth laws and prepares immutable run inputs.
 */
namespace ggems::core::sources {

/*! \brief Identifies the source representation. */
enum class GGEMSSourceType : std::uint8_t {
  /*! \brief Unspecified source kind. */
  Unknown = 0U,

  /*! \brief Analytic spatial and angular birth laws. */
  Analytic = 1U,

  /*! \brief Reserved voxelized-source kind; not currently executed. */
  Voxelized = 2U,

  /*! \brief Reserved phase-space-source kind; not currently executed. */
  PhaseSpace = 3U,
};

/*! \brief Identifies the local spatial emission support. */
enum class GGEMSEmissionGeometryType : std::uint8_t {
  /*! \brief Unspecified spatial law. */
  Unknown = 0U,

  /*! \brief Single local origin. */
  Point = 1U,

  /*! \brief Uniform local xy rectangle. */
  Rectangle = 2U,

  /*! \brief Uniform local xy ellipse or disk. */
  Ellipse = 3U,

  /*! \brief Uniform centered local box volume. */
  Box = 4U,

  /*! \brief Uniform centered sphere volume. */
  Sphere = 5U,

  /*! \brief Uniform centered cylinder along local z. */
  Cylinder = 6U,
};

/*! \brief Identifies the direction law in the source frame. */
enum class GGEMSAngularDistributionType : std::uint8_t {
  /*! \brief Unspecified direction law. */
  Unknown = 0U,

  /*! \brief Local +Z direction. */
  Fixed = 1U,

  /*! \brief Uniform solid angle within configured cosine and azimuth bounds. */
  Isotropic = 2U,

  /*! \brief Direction from the sampled birth point to a global focus. */
  Focused = 3U,
};

/*! \brief Identifies the conditional source energy law. */
enum class GGEMSEnergyDistributionType : std::uint8_t {
  /*! \brief Unspecified energy law. */
  Unknown = 0U,

  /*! \brief One strictly positive canonical energy. */
  Mono = 1U,

  /*! \brief Weighted selection among discrete canonical energies. */
  DiscreteLines = 2U,

  /*! \brief Weighted regular bins with within-bin sampling. */
  RegularSpectrum = 3U,
};

/*!
 * \brief Encodes a host source kind for shared records.
 *
 * \param[in] source_type Host enumeration value.
 * \return Underlying unsigned encoding; no validity check is performed.
 */
[[nodiscard]] constexpr auto
ToKernelSourceType(GGEMSSourceType source_type) noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(source_type);
}

/*!
 * \brief Interprets a shared encoding as a host source kind.
 *
 * \param[in] source_type Raw unsigned device encoding.
 * \return Enumeration value without validation; unknown integers remain
 * representable.
 */
[[nodiscard]] constexpr auto
FromKernelSourceType(std::uint32_t source_type) noexcept -> GGEMSSourceType {
  return static_cast<GGEMSSourceType>(source_type);
}

/*!
 * \brief Names a source kind for diagnostics.
 *
 * \param[in] source_type Enumeration value to describe.
 * \return Static label, or Unknown for an unrecognized value.
 */
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

/*!
 * \brief Encodes a host energy-law kind for shared records.
 *
 * \param[in] distribution_type Host enumeration value.
 * \return Underlying unsigned encoding; no validity check is performed.
 */
[[nodiscard]] constexpr auto ToKernelEnergyDistributionType(
  GGEMSEnergyDistributionType distribution_type) noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(distribution_type);
}

/*!
 * \brief Interprets a shared encoding as a host energy-law kind.
 *
 * \param[in] distribution_type Raw unsigned device encoding.
 * \return Enumeration value without validation; unknown integers remain
 * representable.
 */
[[nodiscard]] constexpr auto
FromKernelEnergyDistributionType(std::uint32_t distribution_type) noexcept
  -> GGEMSEnergyDistributionType {
  return static_cast<GGEMSEnergyDistributionType>(distribution_type);
}

/*!
 * \brief Names a energy-law kind for diagnostics.
 *
 * \param[in] distribution_type Enumeration value to describe.
 * \return Static label, or Unknown for an unrecognized value.
 */
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

/*!
 * \brief Encodes a host spatial-law kind for shared records.
 *
 * \param[in] geometry_type Host enumeration value.
 * \return Underlying unsigned encoding; no validity check is performed.
 */
[[nodiscard]] constexpr auto
ToKernelEmissionGeometryType(GGEMSEmissionGeometryType geometry_type) noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(geometry_type);
}

/*!
 * \brief Interprets a shared encoding as a host spatial-law kind.
 *
 * \param[in] geometry_type Raw unsigned device encoding.
 * \return Enumeration value without validation; unknown integers remain
 * representable.
 */
[[nodiscard]] constexpr auto
FromKernelEmissionGeometryType(std::uint32_t geometry_type) noexcept
  -> GGEMSEmissionGeometryType {
  return static_cast<GGEMSEmissionGeometryType>(geometry_type);
}

/*!
 * \brief Names a spatial-law kind for diagnostics.
 *
 * \param[in] geometry_type Enumeration value to describe.
 * \return Static label, or Unknown for an unrecognized value.
 */
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

/*!
 * \brief Encodes a host direction-law kind for shared records.
 *
 * \param[in] distribution_type Host enumeration value.
 * \return Underlying unsigned encoding; no validity check is performed.
 */
[[nodiscard]] constexpr auto ToKernelAngularDistributionType(
  GGEMSAngularDistributionType distribution_type) noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(distribution_type);
}

/*!
 * \brief Interprets a shared encoding as a host direction-law kind.
 *
 * \param[in] distribution_type Raw unsigned device encoding.
 * \return Enumeration value without validation; unknown integers remain
 * representable.
 */
[[nodiscard]] constexpr auto
FromKernelAngularDistributionType(std::uint32_t distribution_type) noexcept
  -> GGEMSAngularDistributionType {
  return static_cast<GGEMSAngularDistributionType>(distribution_type);
}

/*!
 * \brief Names a direction-law kind for diagnostics.
 *
 * \param[in] distribution_type Enumeration value to describe.
 * \return Static label, or Unknown for an unrecognized value.
 */
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
