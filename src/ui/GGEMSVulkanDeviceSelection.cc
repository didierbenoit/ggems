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
 * \brief Resolves automatic and explicit Vulkan device selection requests.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <optional>
#include <format>
#include <string_view>
#include <utility>
#include <vector>
#include <cstddef>
#include <string>
#include <expected>
#include <cstdint>
#include <span>

#include "GGEMS/ui/detail/GGEMSVulkanDeviceSelection.hh"

namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Maps an uppercase ASCII character to lowercase.
 *
 * \param[in] character Character to normalize.
 * \return Lowercase ASCII counterpart, or the unchanged character.
 */
[[nodiscard]] auto ToLowerAscii(char character) noexcept -> char {
  if (character >= 'A' && character <= 'Z') {
    return static_cast<char>(character + ('a' - 'A'));
  }
  return character;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Compares two strings with ASCII-only case folding.
 *
 * \param[in] left First string view.
 * \param[in] right Second string view.
 * \return True when lengths and folded characters match.
 */
[[nodiscard]] auto EqualAsciiInsensitive(std::string_view left,
                                         std::string_view right) noexcept
  -> bool {
  if (left.size() != right.size()) {
    return false;
  }

  for (std::size_t index = 0; index < left.size(); ++index) {
    if (ToLowerAscii(left[index]) != ToLowerAscii(right[index])) {
      return false;
    }
  }

  return true;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Searches for a nonempty query using ASCII-only case folding.
 *
 * \param[in] text Device name to search.
 * \param[in] query Nonempty name fragment.
 * \return True when a folded substring matches; false for an empty query.
 */
[[nodiscard]] auto ContainsAsciiInsensitive(std::string_view text,
                                            std::string_view query) noexcept
  -> bool {
  if (query.empty() || query.size() > text.size()) {
    return false;
  }

  for (std::size_t offset = 0; offset + query.size() <= text.size(); ++offset) {
    if (EqualAsciiInsensitive(text.substr(offset, query.size()), query)) {
      return true;
    }
  }

  return false;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats device indices and names for a selection diagnostic.
 *
 * \param[in] candidates Borrowed candidate pointers to list.
 * \return Comma-separated device labels.
 */
[[nodiscard]] auto FormatCandidates(
  std::vector<ggems::ui::detail::GGEMSVulkanDeviceCandidate const *> const
    &candidates) -> std::string {
  std::string result{};

  for (auto const *candidate : candidates) {
    if (!result.empty()) {
      result += ", ";
    }
    result +=
      std::format("[{}] {}", candidate->enumeration_index, candidate->name);
  }

  return result;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Records a chosen device with its display-adapter relationship.
 *
 * \param[in] candidate Chosen suitable device.
 * \param[in] reason Selection rationale to retain.
 * \param[in] display_adapter Resolved monitor adapter, when available.
 * \return Device index, rationale, and known adapter mismatch.
 */
[[nodiscard]] auto
MakeSelection(ggems::ui::detail::GGEMSVulkanDeviceCandidate const &candidate,
              std::string reason,
              std::optional<ggems::ui::detail::GGEMSVulkanDisplayAdapter> const
                &display_adapter)
  -> ggems::ui::detail::GGEMSVulkanDeviceSelection {
  return ggems::ui::detail::GGEMSVulkanDeviceSelection{
    .enumeration_index = candidate.enumeration_index,
    .reason = std::move(reason),
    .display_adapter_mismatch =
      ggems::ui::detail::IsVulkanDisplayAdapterMismatch(candidate,
                                                        display_adapter),
  };
}

// =============================================================================
// =============================================================================

/*!
 * \brief Accepts an explicit device only when it is suitable.
 *
 * \param[in] candidate Explicitly matched device.
 * \param[in] reason Selection rationale to retain on success.
 * \param[in] display_adapter Resolved monitor adapter, when available.
 * \return Accepted device selection, or its incompatibility diagnostic.
 */
[[nodiscard]] auto SelectExplicitCandidate(
  ggems::ui::detail::GGEMSVulkanDeviceCandidate const &candidate,
  std::string reason,
  std::optional<ggems::ui::detail::GGEMSVulkanDisplayAdapter> const
    &display_adapter)
  -> std::expected<ggems::ui::detail::GGEMSVulkanDeviceSelection, std::string> {
  if (!candidate.suitable) {
    std::string_view rejection_reason =
      candidate.rejection_reason.empty()
        ? std::string_view{"unspecified incompatibility"}
        : std::string_view{candidate.rejection_reason};

    return std::unexpected<std::string>{std::format(
      "Vulkan physical device [{}] '{}' was explicitly selected but is "
      "incompatible: {}.",
      candidate.enumeration_index, candidate.name, rejection_reason)};
  }

  return MakeSelection(candidate, std::move(reason), display_adapter);
}
} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

auto GGEMSVulkanDeviceSelector::FromString(std::string selection)
  -> GGEMSVulkanDeviceSelector {
  if (EqualAsciiInsensitive(selection, "auto")) {
    return GGEMSVulkanDeviceSelector{};
  }

  return GGEMSVulkanDeviceSelector{
    .kind = GGEMSVulkanDeviceSelectorKind::Name,
    .name = std::move(selection),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDeviceSelector::FromIndex(std::uint32_t enumeration_index)
  -> GGEMSVulkanDeviceSelector {
  return GGEMSVulkanDeviceSelector{
    .kind = GGEMSVulkanDeviceSelectorKind::EnumerationIndex,
    .enumeration_index = enumeration_index,
    .name = {},
  };
}
// =============================================================================

auto GetVulkanDeviceFallbackScore(vk::PhysicalDeviceType type) noexcept
  -> std::uint32_t {
  switch (type) {
  case vk::PhysicalDeviceType::eIntegratedGpu:
    return 400U;
  case vk::PhysicalDeviceType::eDiscreteGpu:
    return 300U;
  case vk::PhysicalDeviceType::eVirtualGpu:
    return 200U;
  case vk::PhysicalDeviceType::eCpu:
    return 100U;
  case vk::PhysicalDeviceType::eOther:
  default:
    return 0U;
  }
}

// =============================================================================
// =============================================================================

auto IsVulkanDisplayAdapterMismatch(
  GGEMSVulkanDeviceCandidate const &candidate,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) noexcept
  -> bool {
  return display_adapter.has_value() &&
         candidate.platform_adapter_id.has_value() &&
         candidate.platform_adapter_id.value() != display_adapter->platform_id;
}

// =============================================================================
// =============================================================================

auto SelectVulkanDevice(
  GGEMSVulkanDeviceSelector const &selector,
  std::span<GGEMSVulkanDeviceCandidate const> candidates,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter)
  -> std::expected<GGEMSVulkanDeviceSelection, std::string> {
  if (selector.kind == GGEMSVulkanDeviceSelectorKind::EnumerationIndex) {
    auto candidate = std::ranges::find_if(
      candidates, [&selector](GGEMSVulkanDeviceCandidate const &entry) -> bool {
        return entry.enumeration_index == selector.enumeration_index;
      });

    if (candidate == candidates.end()) {
      return std::unexpected<std::string>{std::format(
        "Vulkan physical device enumeration index {} is unavailable.",
        selector.enumeration_index)};
    }

    return SelectExplicitCandidate(*candidate, "explicit enumeration index",
                                   display_adapter);
  }

  if (selector.kind == GGEMSVulkanDeviceSelectorKind::Name) {
    if (selector.name.empty()) {
      return std::unexpected<std::string>{
        "Vulkan device name selection cannot be empty."};
    }

    std::vector<GGEMSVulkanDeviceCandidate const *> matches{};

    for (GGEMSVulkanDeviceCandidate const &candidate : candidates) {
      if (EqualAsciiInsensitive(candidate.name, selector.name)) {
        matches.push_back(&candidate);
      }
    }

    if (matches.empty()) {
      for (GGEMSVulkanDeviceCandidate const &candidate : candidates) {
        if (ContainsAsciiInsensitive(candidate.name, selector.name)) {
          matches.push_back(&candidate);
        }
      }
    }

    if (matches.empty()) {
      return std::unexpected<std::string>{std::format(
        "No Vulkan physical device name matches '{}'.", selector.name)};
    }

    if (matches.size() != 1U) {
      return std::unexpected<std::string>{
        std::format("Vulkan device name '{}' is ambiguous. Candidates: {}.",
                    selector.name, FormatCandidates(matches))};
    }

    return SelectExplicitCandidate(*matches.front(),
                                   "explicit case-insensitive name match",
                                   display_adapter);
  }

  GGEMSVulkanDeviceCandidate const *display_match{nullptr};

  if (display_adapter.has_value()) {
    for (GGEMSVulkanDeviceCandidate const &candidate : candidates) {
      if (!candidate.suitable || !candidate.platform_adapter_id.has_value() ||
          candidate.platform_adapter_id.value() !=
            display_adapter->platform_id) {
        continue;
      }

      if (display_match == nullptr ||
          GetVulkanDeviceFallbackScore(candidate.type) >
            GetVulkanDeviceFallbackScore(display_match->type)) {
        display_match = &candidate;
      }
    }
  }

  if (display_match != nullptr) {
    return MakeSelection(*display_match, "display adapter match",
                         display_adapter);
  }

  GGEMSVulkanDeviceCandidate const *fallback{nullptr};

  for (GGEMSVulkanDeviceCandidate const &candidate : candidates) {
    if (!candidate.suitable) {
      continue;
    }

    if (fallback == nullptr || GetVulkanDeviceFallbackScore(candidate.type) >
                                 GetVulkanDeviceFallbackScore(fallback->type)) {
      fallback = &candidate;
    }
  }

  if (fallback == nullptr) {
    return std::unexpected<std::string>{
      "No Vulkan physical device satisfies the GGEMS GuiMode requirements."};
  }

  std::string reason =
    display_adapter.has_value()
      ? "automatic fallback: no suitable Vulkan device matches the "
        "display adapter identity"
      : "automatic fallback: display adapter identity unavailable";

  return MakeSelection(*fallback, std::move(reason), display_adapter);
}

} // namespace ggems::ui::detail
