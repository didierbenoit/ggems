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
 * \brief Checks rendering-device selection and actionable failures.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <vulkan/vulkan.hpp>

#include "GGEMS/ui/detail/GGEMSVulkanDeviceSelection.hh"

namespace {

using ggems::ui::detail::GGEMSVulkanDeviceCandidate;
using ggems::ui::detail::GGEMSVulkanDeviceSelector;
using ggems::ui::detail::GGEMSVulkanDisplayAdapter;
using ggems::ui::detail::SelectVulkanDevice;

// =============================================================================
// =============================================================================

[[nodiscard]] auto
MakeCandidate(std::uint32_t index, std::string name,
              vk::PhysicalDeviceType type, bool suitable = true,
              std::optional<std::string> platform_adapter_id = std::nullopt,
              std::string rejection_reason = {}) -> GGEMSVulkanDeviceCandidate {
  GGEMSVulkanDeviceCandidate candidate{};
  candidate.enumeration_index = index;
  candidate.name = std::move(name);
  candidate.type = type;
  candidate.required_extensions_available = suitable;
  candidate.required_features_available = suitable;
  candidate.swapchain_adequate = suitable;
  candidate.suitable = suitable;
  candidate.platform_adapter_id = std::move(platform_adapter_id);
  candidate.rejection_reason = suitable || !rejection_reason.empty()
                                 ? std::move(rejection_reason)
                                 : "missing presentation queue";
  return candidate;
}

// =============================================================================
// =============================================================================

// Automatic rendering selection must prioritize the actual display adapter when
// identified.
TEST(GGEMSVulkanDeviceSelection, AutoPrefersDisplayMatchOverDiscreteDevice) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(5U, "Display Vulkan Device",
                  vk::PhysicalDeviceType::eVirtualGpu, true,
                  "win32-luid:display"),
    MakeCandidate(17U, "Discrete Vulkan Device",
                  vk::PhysicalDeviceType::eDiscreteGpu, true,
                  "win32-luid:other"),
  };

  GGEMSVulkanDisplayAdapter display_adapter{
    .platform_id = "win32-luid:display",
    .name = "Display apapter",
  };

  auto result =
    SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromString("auto"),
                       std::span{candidates}, display_adapter);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->enumeration_index, 5U);
  EXPECT_EQ(result->reason, "display adapter match");
  EXPECT_FALSE(result->display_adapter_mismatch);
}

// =============================================================================
// =============================================================================

// Missing display identity must use the admitted rendering preference and
// explain its choice.
TEST(GGEMSVulkanDeviceSelection,
     AutoUsesCurrentFallbackWhenDisplayIdentityIsUnavailable) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(3U, "Discrete", vk::PhysicalDeviceType::eDiscreteGpu),
    MakeCandidate(19U, "Integrated", vk::PhysicalDeviceType::eIntegratedGpu),
  };

  auto result =
    SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromString("auto"),
                       std::span{candidates}, std::nullopt);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->enumeration_index, 19U);
  EXPECT_EQ(result->reason,
            "automatic fallback: display adapter identity unavailable");
}

// =============================================================================
// =============================================================================

// Explicit rendering selection must use enumeration IDs rather than vector
// positions.
TEST(GGEMSVulkanDeviceSelection,
     ExplicitIndexSupportsNonContiguousEnumerationIndices) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(4U, "First", vk::PhysicalDeviceType::eIntegratedGpu),
    MakeCandidate(27U, "Second", vk::PhysicalDeviceType::eDiscreteGpu),
  };

  auto result = SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromIndex(27U),
                                   std::span{candidates}, std::nullopt);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->enumeration_index, 27U);
}

// =============================================================================
// =============================================================================

// An unknown rendering index must report the requested value without choosing
// another device.
TEST(GGEMSVulkanDeviceSelection, InvalidExplicitIndexFails) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(11U, "Only device", vk::PhysicalDeviceType::eIntegratedGpu),
  };

  auto result = SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromIndex(3U),
                                   std::span{candidates}, std::nullopt);

  ASSERT_FALSE(result.has_value());
  EXPECT_NE(result.error().find("index 3"), std::string::npos);
}

// =============================================================================
// =============================================================================

// Rendering device names must be selectable without matching vendor
// capitalization.
TEST(GGEMSVulkanDeviceSelection, NameSearchIsCaseInsensitive) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(8U, "AMD Radeon RX 7900 XTX",
                  vk::PhysicalDeviceType::eDiscreteGpu),
  };

  auto result =
    SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromString("amd radeon"),
                       std::span{candidates}, std::nullopt);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->enumeration_index, 8U);
}

// =============================================================================
// =============================================================================

// An exact device name must resolve uniquely even when other names contain it.
TEST(GGEMSVulkanDeviceSelection,
     ExactNameMatchTakesPriorityOverPartialMatches) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(7U, "AMD Radeon", vk::PhysicalDeviceType::eIntegratedGpu),
    MakeCandidate(42U, "AMD Radeon RX 7900 XTX",
                  vk::PhysicalDeviceType::eDiscreteGpu),
    MakeCandidate(91U, "Mobile AMD Radeon",
                  vk::PhysicalDeviceType::eIntegratedGpu),
  };

  auto result =
    SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromString("aMd RaDeOn"),
                       std::span{candidates}, std::nullopt);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->enumeration_index, 7U);
}

// =============================================================================
// =============================================================================

// Ambiguous rendering selection must expose candidates instead of silently
// choosing one.
TEST(GGEMSVulkanDeviceSelection, AmbiguousNameFailsAndListsCandidates) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(2U, "AMD Radeon RX 7800 XT",
                  vk::PhysicalDeviceType::eDiscreteGpu),
    MakeCandidate(13U, "AMD Radeon RX 7900 XTX",
                  vk::PhysicalDeviceType::eDiscreteGpu),
  };

  auto result =
    SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromString("radeon"),
                       std::span{candidates}, std::nullopt);
  ASSERT_FALSE(result.has_value());
  EXPECT_NE(result.error().find("[2] AMD Radeon RX 7800 XT"),
            +std::string::npos);
  EXPECT_NE(result.error().find("[13] AMD Radeon RX 7900 XTX"),
            +std::string::npos);
}

// =============================================================================
// =============================================================================

// Explicit rendering selection must report the device's missing capability.
TEST(GGEMSVulkanDeviceSelection, ExplicitlyIncompatibleDeviceFailsWithReason) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(31U, "Incompatible device",
                  vk::PhysicalDeviceType::eDiscreteGpu, false, std::nullopt,
                  "no presentation queue"),
  };
  auto result = SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromIndex(31U),
                                   std::span{candidates}, std::nullopt);
  ASSERT_FALSE(result.has_value());
  EXPECT_NE(result.error().find("no presentation queue"), +std::string::npos);
}

// =============================================================================
// =============================================================================

// An inventory with no usable rendering device must produce an actionable
// failure.
TEST(GGEMSVulkanDeviceSelection, NoCompatibleDeviceFailsClearly) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(1U, "First", vk::PhysicalDeviceType::eIntegratedGpu, false),
    MakeCandidate(23U, "Second", vk::PhysicalDeviceType::eDiscreteGpu, false),
  };
  auto result =
    SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromString("auto"),
                       std::span{candidates}, std::nullopt);
  ASSERT_FALSE(result.has_value());
  EXPECT_NE(result.error().find("No Vulkan physical device satisfies"),
            std::string::npos);
}

// =============================================================================
// =============================================================================

// A reliable display/render adapter mismatch must remain visible to the caller.
TEST(GGEMSVulkanDeviceSelection, ReliableCrossAdapterMismatchIsReported) {
  std::vector<GGEMSVulkanDeviceCandidate> candidates{
    MakeCandidate(29U, "Selected renderer",
                  vk::PhysicalDeviceType::eDiscreteGpu, true,
                  "win32-luid:renderer"),
  };

  GGEMSVulkanDisplayAdapter display_adapter{
    .platform_id = "win32-luid:display",
    .name = "Display adapter",
  };

  auto result = SelectVulkanDevice(GGEMSVulkanDeviceSelector::FromIndex(29U),
                                   std::span{candidates}, display_adapter);

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->display_adapter_mismatch);
}

} // namespace
