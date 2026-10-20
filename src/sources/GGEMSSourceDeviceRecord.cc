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
 * \brief Converts canonical Source configuration at the device boundary.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

auto BuildSourceDeviceRecord(GGEMSSourceRecord const &record)
  -> GGEMSSourceDeviceRecord {
  GGEMSSourceDeviceRecord result{
    .source_id = record.source_id,
    .time_start_ps = record.time_start_ps,
    .time_stop_ps = record.time_stop_ps,
    .energy_micro_eV = record.energy_micro_eV,
    .position_x_pm = static_cast<float>(record.position_x_pm),
    .position_y_pm = static_cast<float>(record.position_y_pm),
    .position_z_pm = static_cast<float>(record.position_z_pm),
    .source_type = record.source_type,
    .emitted_particle_type = record.emitted_particle_type,
    .flags = record.flags,
    .reserved_0 = record.reserved_0,
    .axis_x_x = record.axis_x_x,
    .axis_x_y = record.axis_x_y,
    .axis_x_z = record.axis_x_z,
    .axis_y_x = record.axis_y_x,
    .axis_y_y = record.axis_y_y,
    .axis_y_z = record.axis_y_z,
    .axis_z_x = record.axis_z_x,
    .axis_z_y = record.axis_z_y,
    .axis_z_z = record.axis_z_z,
    .emission_geometry_type = record.emission_geometry_type,
    .angular_distribution_type = record.angular_distribution_type,
    .geometry_size_x_pm = static_cast<float>(record.geometry_size_x_pm),
    .geometry_size_y_pm = static_cast<float>(record.geometry_size_y_pm),
    .focus_position_x_pm = static_cast<float>(record.focus_position_x_pm),
    .focus_position_y_pm = static_cast<float>(record.focus_position_y_pm),
    .focus_position_z_pm = static_cast<float>(record.focus_position_z_pm),
    .geometry_size_z_pm = static_cast<float>(record.geometry_size_z_pm),
    .isotropic_cos_theta_lower = record.isotropic_cos_theta_lower,
    .isotropic_cos_theta_upper = record.isotropic_cos_theta_upper,
    .isotropic_phi_min_rad = record.isotropic_phi_min_rad,
    .isotropic_phi_max_rad = record.isotropic_phi_max_rad,
  };

  if (record.emission_geometry_type ==
        ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Point) &&
      record.angular_distribution_type ==
        ToKernelAngularDistributionType(
          GGEMSAngularDistributionType::Focused) &&
      result.focus_position_x_pm == result.position_x_pm &&
      result.focus_position_y_pm == result.position_y_pm &&
      result.focus_position_z_pm == result.position_z_pm) {
    throw GGEMSRecoverable(
      "Focused Point origin and focus coincide after FP32 device conversion.");
  }

  return result;
}

} // namespace ggems::core::sources
