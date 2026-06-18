#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSUnits.hh
 * \brief Central aggregator for GGEMS physical units and dimensional types.
 *
 * This header serves as a unified inclusion point for all physical unit
 * definitions used throughout GGEMS. It aggregates specialised modules
 * providing typed quantities for time, length, speed, frequency, byte
 * storage, bit counts and bandwidth measures.
 *
 * All underlying components are defined in dedicated headers under the
 * \c GGEMS/core/units/ directory. This file introduces no additional
 * types, constants or logic; it exists purely for include convenience
 * and clarity in simulation code.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/units/GGEMSDoseUnits.hh"
#include "GGEMS/core/units/GGEMSCrossSectionUnits.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSDensityUnits.hh"
#include "GGEMS/core/units/GGEMSAreaUnits.hh"
#include "GGEMS/core/units/GGEMSVolumeUnits.hh"
#include "GGEMS/core/units/GGEMSMassUnits.hh"
#include "GGEMS/core/units/GGEMSBandwidthUnits.hh"
#include "GGEMS/core/units/GGEMSBitsUnits.hh"
#include "GGEMS/core/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/core/units/GGEMSSpeedUnits.hh"
