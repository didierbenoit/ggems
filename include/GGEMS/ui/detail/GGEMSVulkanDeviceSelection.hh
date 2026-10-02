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
 * \brief Selects suitable Vulkan devices by identity, name, or fallback rank.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <cstdint>

#include <vulkan/vulkan.hpp>

namespace ggems::ui::detail {

/*! \brief Selects the source of a Vulkan device-selection request. */
enum class GGEMSVulkanDeviceSelectorKind : std::uint8_t {
  /*! \brief Prefer a display-adapter match, then use the fallback rank. */
  Automatic = 0U,

  /*! \brief Request one Vulkan enumeration index. */
  EnumerationIndex,

  /*! \brief Request one case-insensitive device-name match. */
  Name,
};

/*! \brief Stores the user request for a rendering device. */
struct GGEMSVulkanDeviceSelector {
  /*! \brief Selection mode determining which request field is used. */
  GGEMSVulkanDeviceSelectorKind kind{GGEMSVulkanDeviceSelectorKind::Automatic};

  /*! \brief Index in the Vulkan physical-device enumeration. */
  std::uint32_t enumeration_index{0U};

  /*! \brief Device-name query used by name selection. */
  std::string name;

  /*!
   * \brief Parses automatic selection or retains a device-name query.
   *
   * \param[in] selection Case-insensitive auto keyword or device-name query.
   * \return Automatic selector for auto, otherwise a name selector.
   */
  [[nodiscard]] static auto FromString(std::string selection)
    -> GGEMSVulkanDeviceSelector;

  /*!
   * \brief Builds an explicit enumeration-index request.
   *
   * \param[in] enumeration_index Vulkan physical-device enumeration index.
   * \return Selector retaining the requested index.
   */
  [[nodiscard]] static auto FromIndex(std::uint32_t enumeration_index)
    -> GGEMSVulkanDeviceSelector;
};

/*! \brief Identifies the platform adapter driving the window display. */
struct GGEMSVulkanDisplayAdapter {
  /*! \brief Platform identity comparable with a Vulkan adapter identity. */
  std::string platform_id;

  /*! \brief Display-adapter name reported by the platform. */
  std::string name;
};

/*! \brief Captures one enumerated device and its rendering suitability. */
struct GGEMSVulkanDeviceCandidate {
  /*! \brief Index in the Vulkan physical-device enumeration. */
  std::uint32_t enumeration_index{0U};

  /*! \brief Borrowed Vulkan physical-device handle. */
  vk::PhysicalDevice physical_device{};

  /*! \brief Physical-device name reported by Vulkan. */
  std::string name;

  /*! \brief Device category used by automatic fallback ranking. */
  vk::PhysicalDeviceType type{vk::PhysicalDeviceType::eOther};

  /*! \brief Vendor identifier reported by Vulkan. */
  std::uint32_t vendor_id{0U};

  /*! \brief Device identifier reported by Vulkan. */
  std::uint32_t device_id{0U};

  /*! \brief Packed supported Vulkan API version. */
  std::uint32_t api_version{0U};

  /*! \brief Implementation-defined packed driver version. */
  std::uint32_t driver_version{0U};

  /*! \brief First queue family supporting graphics, when available. */
  std::optional<std::uint32_t> graphics_queue_family;

  /*! \brief First queue family supporting the window surface. */
  std::optional<std::uint32_t> presentation_queue_family;

  /*! \brief Whether all required device extensions are available. */
  bool required_extensions_available{false};

  /*! \brief Whether required Vulkan 1.1 and 1.3 features are available. */
  bool required_features_available{false};

  /*! \brief Whether the surface exposes formats and presentation modes. */
  bool swapchain_adequate{false};

  /*! \brief Whether the device meets all current GUI requirements. */
  bool suitable{false};

  /*! \brief Diagnostic listing unmet rendering requirements. */
  std::string rejection_reason;

  /*! \brief Platform adapter identity when Vulkan exposes one. */
  std::optional<std::string> platform_adapter_id;
};

/*! \brief Records a chosen device and the reason for selection. */
struct GGEMSVulkanDeviceSelection {
  /*! \brief Index in the Vulkan physical-device enumeration. */
  std::uint32_t enumeration_index{0U};

  /*! \brief Explanation of the successful selection. */
  std::string reason;

  /*! \brief Whether known renderer and display adapter identities differ. */
  bool display_adapter_mismatch{false};
};

/*!
 * \brief Ranks device categories for automatic selection.
 *
 * \param[in] type Vulkan physical-device category.
 * \return Descending preference: integrated, discrete, virtual, CPU, other.
 */
[[nodiscard]] auto
GetVulkanDeviceFallbackScore(vk::PhysicalDeviceType type) noexcept
  -> std::uint32_t;

/*!
 * \brief Compares known rendering and display adapter identities.
 *
 * \param[in] candidate Rendering device candidate.
 * \param[in] display_adapter Resolved window adapter, when available.
 * \return True only when both identities exist and differ.
 */
[[nodiscard]] auto IsVulkanDisplayAdapterMismatch(
  GGEMSVulkanDeviceCandidate const &candidate,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) noexcept
  -> bool;

/*!
 * \brief Selects one suitable device without silently replacing a request.
 *
 * Name matching prefers exact ASCII-insensitive matches before substrings and
 *   rejects ambiguity. Automatic selection prefers the display adapter, then
 *   device-type rank; equal ranks retain the first candidate.
 *
 * \param[in] selector Automatic, index, or name selection request.
 * \param[in] candidates Devices in Vulkan enumeration order.
 * \param[in] display_adapter Resolved window adapter, when available.
 * \return Chosen enumeration index and rationale, or a diagnostic for
 *   rejection.
 */
[[nodiscard]] auto SelectVulkanDevice(
  GGEMSVulkanDeviceSelector const &selector,
  std::span<GGEMSVulkanDeviceCandidate const> candidates,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter)
  -> std::expected<GGEMSVulkanDeviceSelection, std::string>;

} // namespace ggems::ui::detail
