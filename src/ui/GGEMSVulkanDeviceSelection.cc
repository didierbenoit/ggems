#include <algorithm>
#include <format>
#include <string_view>
#include <utility>
#include <vector>

#include "GGEMS/ui/GGEMSVulkanDeviceSelection.hh"

namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] char ToLowerAscii(char character) noexcept {
  if (character >= 'A' && character <= 'Z') {
    return static_cast<char>(character + ('a' - 'A'));
  }
  return character;
}

// =============================================================================
// =============================================================================

[[nodiscard]] bool EqualAsciiInsensitive(std::string_view left,
                                         std::string_view right) noexcept {
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

[[nodiscard]] bool ContainsAsciiInsensitive(std::string_view text,
                                            std::string_view query) noexcept {
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

[[nodiscard]] std::string FormatCandidates(
    std::vector<ggems::ui::detail::GGEMSVulkanDeviceCandidate const *> const
        &candidates) {
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

[[nodiscard]] ggems::ui::detail::GGEMSVulkanDeviceSelection
MakeSelection(ggems::ui::detail::GGEMSVulkanDeviceCandidate const &candidate,
              std::string reason,
              std::optional<ggems::ui::detail::GGEMSVulkanDisplayAdapter> const
                  &display_adapter) {
  return ggems::ui::detail::GGEMSVulkanDeviceSelection{
      .enumeration_index = candidate.enumeration_index,
      .reason = std::move(reason),
      .display_adapter_mismatch =
          ggems::ui::detail::IsVulkanDisplayAdapterMismatch(candidate,
                                                            display_adapter)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] std::expected<ggems::ui::detail::GGEMSVulkanDeviceSelection,
                            std::string>
SelectExplicitCandidate(
    ggems::ui::detail::GGEMSVulkanDeviceCandidate const &candidate,
    std::string reason,
    std::optional<ggems::ui::detail::GGEMSVulkanDisplayAdapter> const
        &display_adapter) {
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

GGEMSVulkanDeviceSelector
GGEMSVulkanDeviceSelector::FromString(std::string selection) {
  if (EqualAsciiInsensitive(selection, "auto")) {
    return GGEMSVulkanDeviceSelector{};
  }

  return GGEMSVulkanDeviceSelector{.kind = GGEMSVulkanDeviceSelectorKind::Name,
                                   .name = std::move(selection)};
}

// -----------------------------------------------------------------------------

GGEMSVulkanDeviceSelector
GGEMSVulkanDeviceSelector::FromIndex(std::uint32_t enumeration_index) {
  return GGEMSVulkanDeviceSelector{
      .kind = GGEMSVulkanDeviceSelectorKind::EnumerationIndex,
      .enumeration_index = enumeration_index};
}
// =============================================================================

std::uint32_t
GetVulkanDeviceFallbackScore(vk::PhysicalDeviceType type) noexcept {
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

bool IsVulkanDisplayAdapterMismatch(
    GGEMSVulkanDeviceCandidate const &candidate,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) noexcept {
  return display_adapter.has_value() &&
         candidate.platform_adapter_id.has_value() &&
         candidate.platform_adapter_id.value() != display_adapter->platform_id;
}

// =============================================================================
// =============================================================================

std::expected<GGEMSVulkanDeviceSelection, std::string> SelectVulkanDevice(
    GGEMSVulkanDeviceSelector const &selector,
    std::span<GGEMSVulkanDeviceCandidate const> candidates,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) {
  if (selector.kind == GGEMSVulkanDeviceSelectorKind::EnumerationIndex) {
    auto candidate = std::ranges::find_if(
        candidates, [&selector](GGEMSVulkanDeviceCandidate const &entry) {
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

    if (fallback == nullptr ||
        GetVulkanDeviceFallbackScore(candidate.type) >
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
