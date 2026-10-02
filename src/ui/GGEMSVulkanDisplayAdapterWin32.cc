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
 * \brief Resolves Windows display and Vulkan adapter identities through LUIDs.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#if defined(_WIN32)

#include <windows.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <expected>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

/*! \brief Keeps GLFW from selecting a graphics API include boundary. */
#define GLFW_INCLUDE_NONE
/*! \brief Enables the GLFW native Win32 window-handle API. */
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "GGEMS/ui/detail/GGEMSVulkanDisplayAdapterWin32.hh"
#include "GGEMS/ui/detail/GGEMSVulkanDeviceSelection.hh"

namespace {

using Microsoft::WRL::ComPtr;

// =============================================================================
// =============================================================================

/*!
 * \brief Encodes adapter identity bytes in the shared Windows LUID format.
 *
 * \param[in] bytes Platform LUID bytes in their native stored order.
 * \return Lowercase hexadecimal identity with the win32-luid prefix.
 */
[[nodiscard]] auto EncodeLuid(std::span<std::uint8_t const> bytes)
  -> std::string {
  constexpr std::string_view k_hex_digits{"0123456789abcdef"};

  std::string identity{"win32-luid:"};
  identity.reserve(identity.size() + (bytes.size() * 2U));

  for (std::uint8_t byte : bytes) {
    auto value = static_cast<std::uint32_t>(byte);

    identity.push_back(k_hex_digits[(value >> 4U) & 0x0FU]);
    identity.push_back(k_hex_digits[value & 0x0FU]);
  }

  return identity;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Converts a Windows adapter label to UTF-8.
 *
 * \param[in] text Borrowed UTF-16 label.
 * \return UTF-8 text, or an empty string on conversion failure or empty input.
 */
[[nodiscard]] auto WideToUtf8(std::wstring_view text) -> std::string {
  if (text.empty()) {
    return {};
  }

  int source_length = static_cast<int>(text.size());
  int destination_length = WideCharToMultiByte(
    CP_UTF8, 0, text.data(), source_length, nullptr, 0, nullptr, nullptr);

  if (destination_length <= 0) {
    return {};
  }

  std::string result(static_cast<std::size_t>(destination_length), '\0');

  int converted_length =
    WideCharToMultiByte(CP_UTF8, 0, text.data(), source_length, result.data(),
                        destination_length, nullptr, nullptr);

  if (converted_length != destination_length) {
    return {};
  }

  return result;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Encodes a DXGI adapter LUID for comparison with Vulkan IDs.
 *
 * \param[in] luid DXGI adapter identifier.
 * \return Identity in the shared Windows LUID byte format.
 */
[[nodiscard]] auto EncodeDxgiLuid(LUID const &luid) -> std::string {
  static_assert(sizeof(LUID) == VK_LUID_SIZE,
                "Windows and Vulkan LUID widths must match.");

  std::array<std::uint8_t, VK_LUID_SIZE> bytes{};
  std::memcpy(bytes.data(), &luid, bytes.size());

  return EncodeLuid(std::span<std::uint8_t const>{bytes.data(), bytes.size()});
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats a failed Windows adapter query as an expected error.
 *
 * \param[in] operation Name of the failed DXGI operation.
 * \param[in] result HRESULT returned by the operation.
 * \return Unexpected result containing the operation and hexadecimal error
 *   code.
 */
[[nodiscard]] auto MakeDxgiFailure(std::string_view operation, HRESULT result)
  -> std::unexpected<std::string> {
  return std::unexpected<std::string>{
    std::format("{} failed with HRESULT 0x{:08x}.", operation,
                static_cast<std::uint32_t>(result))};
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

auto ResolveWin32DisplayAdapter(GLFWwindow *window)
  -> std::expected<GGEMSVulkanDisplayAdapter, std::string> {
  if (window == nullptr) {
    return std::unexpected<std::string>{"A valid GLFW window is required to "
                                        "resolve the Win32 display adapter."};
  }

  HWND window_handle = glfwGetWin32Window(window);

  if (window_handle == nullptr) {
    return std::unexpected<std::string>{
      "GLFW did not expose a Win32 HWND for the GuiMode window."};
  }

  HMONITOR monitor = MonitorFromWindow(window_handle, MONITOR_DEFAULTTONEAREST);

  if (monitor == nullptr) {
    return std::unexpected<std::string>{
      "MonitorFromWindow did not identify a monitor for the GuiMode window."};
  }

  ComPtr<IDXGIFactory1> factory{};
  HRESULT result = CreateDXGIFactory1(
    IID_IDXGIFactory1,
    reinterpret_cast<void **>(factory.ReleaseAndGetAddressOf()));

  if (FAILED(result)) {
    return MakeDxgiFailure("CreateDXGIFactory1", result);
  }

  std::optional<GGEMSVulkanDisplayAdapter> matched_adapter{};

  for (UINT adapter_index = 0U;; ++adapter_index) {
    ComPtr<IDXGIAdapter1> adapter{};
    result =
      factory->EnumAdapters1(adapter_index, adapter.ReleaseAndGetAddressOf());

    if (result == DXGI_ERROR_NOT_FOUND) {
      break;
    }

    if (FAILED(result)) {
      return MakeDxgiFailure("IDXGIFactory1::EnumAdapters1", result);
    }

    DXGI_ADAPTER_DESC1 adapter_description{};
    result = adapter->GetDesc1(&adapter_description);

    if (FAILED(result)) {
      return MakeDxgiFailure("IDXGIAdapter1::GetDesc1", result);
    }

    for (UINT output_index = 0U;; ++output_index) {
      ComPtr<IDXGIOutput> output{};
      result =
        adapter->EnumOutputs(output_index, output.ReleaseAndGetAddressOf());

      if (result == DXGI_ERROR_NOT_FOUND) {
        break;
      }

      if (FAILED(result)) {
        return MakeDxgiFailure("IDXGIAdapter1::EnumOutputs", result);
      }

      DXGI_OUTPUT_DESC output_description{};
      result = output->GetDesc(&output_description);

      if (FAILED(result)) {
        return MakeDxgiFailure("IDXGIOutput::GetDesc", result);
      }

      if (output_description.Monitor != monitor) {
        continue;
      }

      std::string name = WideToUtf8(adapter_description.Description);

      if (name.empty()) {
        name = std::format("DXGI adapter {}", adapter_index);
      }

      GGEMSVulkanDisplayAdapter candidate{
        .platform_id = EncodeDxgiLuid(adapter_description.AdapterLuid),
        .name = std::move(name),
      };

      if (matched_adapter.has_value() &&
          matched_adapter->platform_id != candidate.platform_id) {
        return std::unexpected<std::string>{
          "Multiple DXGI adapters were associated with the selected "
          "HMONITOR."};
      }

      matched_adapter = std::move(candidate);
    }
  }

  if (!matched_adapter.has_value()) {
    return std::unexpected<std::string>{
      "No DXGI output matches the HMONITOR containing the GuiMode window."};
  }

  return std::move(matched_adapter).value();
}

// =============================================================================
// =============================================================================

auto QueryWin32VulkanAdapterId(vk::raii::PhysicalDevice const &physical_device)
  -> std::optional<std::string> {
  auto properties =
    physical_device.getProperties2<vk::PhysicalDeviceProperties2,
                                   vk::PhysicalDeviceIDProperties>();

  vk::PhysicalDeviceIDProperties const &device_id =
    properties.get<vk::PhysicalDeviceIDProperties>();

  if (device_id.deviceLUIDValid != VK_TRUE) {
    return std::nullopt;
  }

  return EncodeLuid(
    std::span<std::uint8_t const>{device_id.deviceLUID.data(), VK_LUID_SIZE});
}

} // namespace ggems::ui::detail

#endif
