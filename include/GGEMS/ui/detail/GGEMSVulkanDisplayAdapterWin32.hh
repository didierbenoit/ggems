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
 * \brief Matches the window display adapter to Vulkan using Windows LUIDs.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#if defined(_WIN32)

#include <expected>
#include <optional>
#include <string>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMSVulkanDeviceSelection.hh"

struct GLFWwindow;

namespace ggems::ui::detail {

/*!
 * \brief Resolves the adapter driving the monitor nearest the window.
 *
 * \param[in] window GLFW window with a native Win32 handle.
 * \return DXGI adapter identity and name, or an OS-resolution diagnostic.
 */
[[nodiscard]] auto ResolveWin32DisplayAdapter(GLFWwindow *window)
  -> std::expected<GGEMSVulkanDisplayAdapter, std::string>;

/*!
 * \brief Reads a Vulkan device LUID in the shared Windows identity format.
 *
 * \param[in] physical_device Vulkan device supporting the ID-properties query.
 * \return Encoded adapter LUID, or no value when Vulkan reports it invalid.
 */
[[nodiscard]] auto
QueryWin32VulkanAdapterId(vk::raii::PhysicalDevice const &physical_device)
  -> std::optional<std::string>;

} // namespace ggems::ui::detail

#endif
