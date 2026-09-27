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
 * \brief Defines the shared analytic-source pose and emission record.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <numbers>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

/*! \brief Cosine lower bound for full-sphere isotropic emission. */
inline constexpr float k_isotropic_full_sphere_cos_theta_lower{-1.0F};

/*! \brief Cosine upper bound for full-sphere isotropic emission. */
inline constexpr float k_isotropic_full_sphere_cos_theta_upper{1.0F};

/*! \brief Full-sphere azimuth start in radians. */
inline constexpr float k_isotropic_full_sphere_phi_min_rad{0.0F};

/*! \brief Full-sphere azimuth stop of two pi radians. */
inline constexpr float k_isotropic_full_sphere_phi_max_rad{
  6.28318530717958647692F};

/*!
 * \brief Packs source pose and analytic birth laws for host/device execution.
 *
 * The orthonormal axes map local source coordinates into the global frame.
 * Spatial extents are full widths or diameters centered on the local origin.
 * Focused directions aim at a global focus; integer positions are stored in pm.
 * Energy tables and population laws are carried by separate descriptors.
 * Default values describe a point source at the origin emitting 511 keV Gamma
 * particles along global +Z.
 */
struct GGEMSSourceRecord {
  /*! \brief Source identifier carried into generated particle state. */
  std::uint64_t source_id{0ULL};

  /*! \brief Inclusive birth-window start in absolute simulation ps. */
  std::uint64_t time_start_ps{0ULL};

  /*! \brief Exclusive birth-window stop in absolute simulation ps. */
  std::uint64_t time_stop_ps{0ULL};

  /*!
   * \brief Count-driven mono energy in micro-eV; zero for other energy laws.
   */
  std::uint64_t energy_micro_eV{511'000'000'000ULL};

  /*! \brief Global x coordinate of the source origin in signed pm. */
  std::int64_t position_x_pm{0LL};

  /*! \brief Global y coordinate of the source origin in signed pm. */
  std::int64_t position_y_pm{0LL};

  /*! \brief Global z coordinate of the source origin in signed pm. */
  std::int64_t position_z_pm{0LL};

  /*!
   * \brief Device source-kind encoding; current execution supports Analytic.
   */
  std::uint32_t source_type{ToKernelSourceType(GGEMSSourceType::Analytic)};

  /*! \brief Count-driven particle kind; Unknown for activity-driven sources. */
  std::uint32_t emitted_particle_type{
    particles::ToKernelParticleType(particles::GGEMSParticleType::Gamma)};

  /*! \brief Reserved source flags, initialized to zero. */
  std::uint32_t flags{0U};

  /*! \brief Reserved host/device layout word, initialized to zero. */
  std::uint32_t reserved_0{0U};

  /*! \brief Global x component of the unit local x axis. */
  float axis_x_x{1.0F};

  /*! \brief Global y component of the unit local x axis. */
  float axis_x_y{0.0F};

  /*! \brief Global z component of the unit local x axis. */
  float axis_x_z{0.0F};

  /*! \brief Global x component of the unit local y axis. */
  float axis_y_x{0.0F};

  /*! \brief Global y component of the unit local y axis. */
  float axis_y_y{1.0F};

  /*! \brief Global z component of the unit local y axis. */
  float axis_y_z{0.0F};

  /*! \brief Global x component of the unit local z axis. */
  float axis_z_x{0.0F};

  /*! \brief Global y component of the unit local z axis. */
  float axis_z_y{0.0F};

  /*! \brief Global z component of the unit local z axis. */
  float axis_z_z{1.0F};

  /*! \brief Device encoding of the local spatial emission shape. */
  std::uint32_t emission_geometry_type{
    ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Point)};

  /*! \brief Device encoding of the local emission direction law. */
  std::uint32_t angular_distribution_type{
    ToKernelAngularDistributionType(GGEMSAngularDistributionType::Fixed)};

  /*! \brief Full local x size or diameter in pm, according to the shape. */
  std::uint64_t geometry_size_x_pm{0ULL};

  /*! \brief Full local y size or diameter in pm, according to the shape. */
  std::uint64_t geometry_size_y_pm{0ULL};

  /*! \brief Global x coordinate of the focus in signed pm. */
  std::int64_t focus_position_x_pm{0LL};

  /*! \brief Global y coordinate of the focus in signed pm. */
  std::int64_t focus_position_y_pm{0LL};

  /*! \brief Global z coordinate of the focus in signed pm. */
  std::int64_t focus_position_z_pm{0LL};

  /*! \brief Full local z size in pm; cylinder height or sphere diameter. */
  std::uint64_t geometry_size_z_pm{0ULL};

  /*! \brief Lower cosine bound, corresponding to the larger polar angle. */
  float isotropic_cos_theta_lower{-1.0F};

  /*! \brief Upper cosine bound, corresponding to the smaller polar angle. */
  float isotropic_cos_theta_upper{1.0F};

  /*! \brief Lower azimuth bound in radians in the local source frame. */
  float isotropic_phi_min_rad{0.0F};

  /*! \brief Upper azimuth bound in radians in the local source frame. */
  float isotropic_phi_max_rad{2.0F * std::numbers::pi_v<float>};
};

} // namespace ggems::core::sources
