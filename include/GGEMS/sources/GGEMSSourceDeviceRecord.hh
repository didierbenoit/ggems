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
 * \brief Defines the FP32 spatial Source upload record.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <numbers>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

/*!
 * \brief Packs source configuration for the OpenCL FP32 spatial pipeline.
 *
 * Mirrors the device GGEMSSourceRecord. Spatial members are binary32 values
 * in pm; absolute time and energy retain their canonical integer units.
 * Canonical host authoring remains in GGEMSSourceRecord.
 */
struct GGEMSSourceDeviceRecord {
  /*!
   * \brief Source identity, distinct from the generated global particle
   * identity.
   */
  std::uint64_t source_id{0ULL};

  /*! \brief Inclusive birth-window start in absolute simulation ps. */
  std::uint64_t time_start_ps{0ULL};

  /*! \brief Exclusive birth-window stop in absolute simulation ps. */
  std::uint64_t time_stop_ps{0ULL};

  /*!
   * \brief Count-driven mono energy in micro-eV; zero for other energy laws.
   */
  std::uint64_t energy_micro_eV{511'000'000'000ULL};

  /*! \brief Global x coordinate of the source origin in binary32 pm. */
  float position_x_pm{0.0F};

  /*! \brief Global y coordinate of the source origin in binary32 pm. */
  float position_y_pm{0.0F};

  /*! \brief Global z coordinate of the source origin in binary32 pm. */
  float position_z_pm{0.0F};

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

  /*! \brief Full local x size or diameter in binary32 pm, according to the
   * shape. */
  float geometry_size_x_pm{0.0F};

  /*! \brief Full local y size or diameter in binary32 pm, according to the
   * shape. */
  float geometry_size_y_pm{0.0F};

  /*! \brief Global x coordinate of the focus in binary32 pm. */
  float focus_position_x_pm{0.0F};

  /*! \brief Global y coordinate of the focus in binary32 pm. */
  float focus_position_y_pm{0.0F};

  /*! \brief Global z coordinate of the focus in binary32 pm. */
  float focus_position_z_pm{0.0F};

  /*! \brief Full local z size in binary32 pm; cylinder height or sphere
   * diameter. */
  float geometry_size_z_pm{0.0F};

  /*! \brief Lower cosine bound, corresponding to the larger polar angle. */
  float isotropic_cos_theta_lower{-1.0F};

  /*! \brief Upper cosine bound, corresponding to the smaller polar angle. */
  float isotropic_cos_theta_upper{1.0F};

  /*! \brief Lower azimuth bound in radians in the local source frame. */
  float isotropic_phi_min_rad{0.0F};

  /*! \brief Upper azimuth bound in radians in the local source frame. */
  float isotropic_phi_max_rad{2.0F * std::numbers::pi_v<float>};
};

/*!
 * \brief Converts validated canonical source configuration for device upload.
 *
 * \param[in] record Host-validated source configuration.
 * \return Device record with spatial fields rounded once to binary32 pm.
 * \throws GGEMSRecoverable If a focused point's distinct canonical positions
 * collapse to the same binary32 position.
 */
[[nodiscard]] auto BuildSourceDeviceRecord(GGEMSSourceRecord const &record)
  -> GGEMSSourceDeviceRecord;

} // namespace ggems::core::sources
