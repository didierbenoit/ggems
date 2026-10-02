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
 * \brief Describes renderer and compute devices for the status panel.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/*! \brief Provides UI presentation state and platform integration. */
namespace ggems::ui::detail {
/*! \brief Describes the selected Vulkan rendering device. */
struct GGEMSVulkanDeviceStatus {
  /*! \brief Whether the device subsystem is initialized. */
  bool initialized{false};

  /*! \brief Index in the Vulkan physical-device enumeration. */
  std::uint32_t enumeration_index{0U};

  /*! \brief Device name reported by its runtime. */
  std::string name;

  /*! \brief Human-readable device category. */
  std::string type;

  /*! \brief Explanation of the Vulkan device selection. */
  std::string selection_reason;
};

/*! \brief Describes one active OpenCL context for display. */
struct GGEMSComputeDeviceStatus {
  /*! \brief Index in the active OpenCL context list. */
  std::size_t context_index{0U};

  /*! \brief Device name reported by its runtime. */
  std::string name;

  /*! \brief Human-readable device category. */
  std::string type;

  /*! \brief OpenCL platform name used to distinguish devices. */
  std::string platform;

  /*! \brief Whether equal device names require platform labels. */
  bool show_platform{false};
};

/*! \brief Collects the active OpenCL devices for display. */
struct GGEMSComputeStatus {
  /*! \brief Whether the device subsystem is initialized. */
  bool initialized{false};

  /*! \brief Active OpenCL devices in context order. */
  std::vector<GGEMSComputeDeviceStatus> devices;
};

/*! \brief Groups renderer and compute status shown by the UI. */
struct GGEMSDeviceStatusSnapshot {
  /*! \brief Vulkan device information captured at initialization. */
  GGEMSVulkanDeviceStatus renderer{};

  /*! \brief OpenCL device information captured at initialization. */
  GGEMSComputeStatus compute{};
};
} // namespace ggems::ui::detail
