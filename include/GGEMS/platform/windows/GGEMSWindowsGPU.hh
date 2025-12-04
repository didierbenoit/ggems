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
 * \file GGEMSWindowsGPU.hh
 * \brief Windows GPU interfaces and driver-level APIs used by GGEMS.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright GNU GPL v3
 *
 * This header exposes DXGI and WDDM structures used by GGEMS to retrieve
 * GPU device topology, PCI descriptors, memory layout and capability flags
 * from NVIDIA or AMD drivers under Microsoft Windows.
 *
 * It isolates these low-level interfaces to avoid systemic leakage of Windows
 * identifiers, and keeps the rest of the codebase fully portable across Linux.
 */

#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"

/// \cond
#include <d3dkmthk.h>
#include <dxgi.h>
#include <dxgi1_4.h>
/// \endcond
