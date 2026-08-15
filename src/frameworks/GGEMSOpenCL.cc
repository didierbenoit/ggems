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
 * \brief Implements the GGEMS OpenCL runtime manager.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <cctype>
#include <exception>
#include <string>
#include <array>
#include <string_view>
#include <filesystem>
#include <memory>
#include <vector>
#include <mutex>
#include <cstddef>
#include <utility>
#include <cstdlib>
#include <charconv>
#include <system_error>
#include <functional>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"

namespace ggems::ocl {

namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Trims and lowercases one device-selector token.
 *
 * \param[in] text Selector text to normalize.
 * \return Normalized selector text.
 */
[[nodiscard]] auto NormalizeDeviceSelectionText(std::string text)
    -> std::string {
  auto const first = text.find_first_not_of(" \t\n\r\f\v");
  if (first == std::string::npos) {
    return {};
  }

  auto const last = text.find_last_not_of(" \t\n\r\f\v");
  text = text.substr(first, last - first + 1U);

  for (char &character : text) {
    character =
        static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  }

  return text;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Maps accepted vendor aliases to normalized OpenCL vendor names.
 */
constexpr std::array<std::pair<std::string_view, std::string_view>, 3>
    vendor_aliases{{{"intel", "intel(r) corporation"},
                    {"nvidia", "nvidia corporation"},
                    {"amd", "advanced micro devices, inc."}}};

// =============================================================================
// =============================================================================

/*!
 * \brief Splits device selector expressions into normalized semicolon-delimited tokens.
 *
 * \param[in] filters Device selector expressions.
 * \return Normalized selector tokens.
 */
[[nodiscard]] auto
TokenizeDeviceSelection(std::vector<std::string> const &filters)
    -> std::vector<std::string> {
  std::vector<std::string> tokens;

  for (auto const &filter : filters) {
    std::size_t begin{0U};

    while (begin <= filter.size()) {
      auto const separator = filter.find(';', begin);
      auto const count = separator == std::string::npos ? std::string::npos
                                                        : separator - begin;

      auto token = NormalizeDeviceSelectionText(filter.substr(begin, count));

      if (token.empty()) {
        throw ggems::core::GGEMSFatal(
            "OpenCL device selector contains an empty token.");
      }

      tokens.push_back(std::move(token));

      if (separator == std::string::npos) {
        break;
      }

      begin = separator + 1U;
    }
  }

  return tokens;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Parses a decimal OpenCL device index.
 *
 * \param[in] token Decimal index token.
 * \return Parsed device index.
 */
[[nodiscard]] auto ParseDeviceIndex(std::string_view token) -> std::size_t {
  std::size_t index{0U};

  auto const result =
      std::from_chars(token.data(), token.data() + token.size(), index);

  if (result.ec != std::errc{} || result.ptr != token.data() + token.size()) {
    throw ggems::core::GGEMSFatal("Invalid OpenCL device index '" +
                                  std::string{token} + "'.");
  }

  return index;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Stores the normalized criteria produced by OpenCL device-selector parsing.
 */
struct DeviceSelectionCriteria {
  /*!
   * \brief Explicit device indices in selector order.
   */
  std::vector<std::size_t> numeric_indices;
  /*!
   * \brief Requested CPU/GPU device type, or zero when unspecified.
   */
  cl_device_type requested_type{0};
  /*!
   * \brief Requested normalized vendor name, or an empty view when unspecified.
   */
  std::string_view requested_vendor;
  /*!
   * \brief Whether any numeric selector was parsed.
   */
  bool has_numeric_selector{false};
  /*!
   * \brief Whether any textual selector was parsed.
   */
  bool has_textual_selector{false};
};

// =============================================================================
// =============================================================================

/*!
 * \brief Checks whether a token begins with a numeric device selector.
 *
 * \param[in] token Normalized selector token.
 * \return True if the token begins with a decimal digit.
 */
[[nodiscard]] auto IsNumericDeviceSelector(std::string_view token) -> bool {
  return !token.empty() &&
         std::isdigit(static_cast<unsigned char>(token.front())) != 0;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Appends a validated device index if it is not already selected.
 *
 * \param[in,out] criteria Selection criteria being built.
 * \param[in] index Device index to append.
 * \param[in] selector Original normalized selector used for diagnostics.
 * \param[in] device_count Number of discovered devices.
 */
auto AppendDeviceIndex(DeviceSelectionCriteria &criteria, std::size_t index,
                       std::string_view selector, std::size_t device_count)
    -> void {
  if (index >= device_count) {
    throw ggems::core::GGEMSFatal(
        "OpenCL device index is out of range in selector '" +
        std::string{selector} + "'.");
  }

  if (std::ranges::find(criteria.numeric_indices, index) ==
      criteria.numeric_indices.end()) {
    criteria.numeric_indices.push_back(index);
  }
}

// =============================================================================
// =============================================================================

/*!
 * \brief Parses one device index or inclusive device-index range.
 *
 * \param[in] token Normalized numeric selector token.
 * \param[in] device_count Number of discovered devices.
 * \param[in,out] criteria Selection criteria being built.
 */
auto ParseNumericDeviceSelector(std::string const &token,
                                std::size_t device_count,
                                DeviceSelectionCriteria &criteria) -> void {
  criteria.has_numeric_selector = true;

  auto const dash = token.find('-');

  if (dash == std::string::npos) {
    AppendDeviceIndex(criteria, ParseDeviceIndex(token), token, device_count);
    return;
  }

  if (dash == 0U || dash + 1U >= token.size() ||
      token.find('-', dash + 1U) != std::string::npos) {
    throw ggems::core::GGEMSFatal("Invalid OpenCL device range '" + token +
                                  "'.");
  }

  auto const start = ParseDeviceIndex(std::string_view{token}.substr(0U, dash));
  auto const end = ParseDeviceIndex(std::string_view{token}.substr(dash + 1U));

  if (start > end) {
    throw ggems::core::GGEMSFatal(
        "OpenCL device range start exceeds range end in selector '" + token +
        "'.");
  }

  if (end >= device_count) {
    throw ggems::core::GGEMSFatal(
        "OpenCL device index is out of range in selector '" + token + "'.");
  }

  for (std::size_t index = start; index <= end; ++index) {
    AppendDeviceIndex(criteria, index, token, device_count);
  }
}

// =============================================================================
// =============================================================================

/*!
 * \brief Parses a CPU/GPU or vendor selector token.
 *
 * \param[in] token Normalized textual selector token.
 * \param[in,out] criteria Selection criteria being built.
 * \return True if the token is a recognized textual selector.
 */
[[nodiscard]] auto ParseTextualDeviceSelector(std::string const &token,
                                              DeviceSelectionCriteria &criteria)
    -> bool {
  if (token == "cpu" || token == "gpu") {
    criteria.has_textual_selector = true;

    cl_device_type const type =
        token == "cpu" ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU;

    if (criteria.requested_type != 0 && criteria.requested_type != type) {
      throw ggems::core::GGEMSFatal(
          "OpenCL device selector cannot combine 'cpu' and 'gpu'.");
    }

    criteria.requested_type = type;
    return true;
  }

  auto const vendor_it = std::ranges::find_if(
      vendor_aliases,
      [&token](auto const &vendor) -> bool { return vendor.first == token; });

  if (vendor_it == vendor_aliases.end()) {
    return false;
  }

  criteria.has_textual_selector = true;

  if (!criteria.requested_vendor.empty() &&
      criteria.requested_vendor != vendor_it->second) {
    throw ggems::core::GGEMSFatal(
        "OpenCL device selector cannot combine multiple vendors.");
  }

  criteria.requested_vendor = vendor_it->second;
  return true;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Builds validated selection criteria from normalized selector tokens.
 *
 * \param[in] tokens Normalized selector tokens.
 * \param[in] device_count Number of discovered devices.
 * \return Validated device-selection criteria.
 */
[[nodiscard]] auto
ParseDeviceSelectionCriteria(std::vector<std::string> const &tokens,
                             std::size_t device_count)
    -> DeviceSelectionCriteria {
  DeviceSelectionCriteria criteria;

  for (auto const &token : tokens) {
    if (ParseTextualDeviceSelector(token, criteria)) {
      continue;
    }

    if (IsNumericDeviceSelector(token)) {
      ParseNumericDeviceSelector(token, device_count, criteria);
      continue;
    }

    throw ggems::core::GGEMSFatal("Unknown OpenCL device selector '" + token +
                                  "'.");
  }

  if (criteria.has_numeric_selector && criteria.has_textual_selector) {
    throw ggems::core::GGEMSFatal(
        "OpenCL device selector cannot combine numeric and textual "
        "selectors.");
  }

  return criteria;
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSOpenCL::GGEMSOpenCL() {
  GGEMS_INFOEX("OpenCL", 3, "Constructing GGEMSOpenCL singleton.");
  std::set_terminate(ggems::core::TerminateHandler);
  DisableNvidiaDriverKernelCache();

  try {
    InitPlatformsAndDevices();
    GGEMS_INFOEX("OpenCL", 2, "OpenCL backend initialized.");
  } catch (core::GGEMSExceptionBase &) {
    std::terminate();
  } catch (std::exception const &) {
    std::terminate();
  } catch (...) {
    std::terminate();
  }
}

// -----------------------------------------------------------------------------

GGEMSOpenCL::~GGEMSOpenCL() {
  GGEMS_INFOEX(
      "OpenCL", 3,
      "GGEMSOpenCL singleton destroyed; memory intentionally retained.");
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::GetOrCreateProgram(GGEMSOpenCLContext const &ctx,
                                     std::filesystem::path const &kernel_root,
                                     std::string const &kernel_name,
                                     std::string const &build_options)
    -> GGEMSOpenCLProgram const & {
  std::scoped_lock lock{program_cache_mutex_};

  for (auto const &program : program_cache_) {
    if (program->Matches(ctx, kernel_root, kernel_name, build_options)) {
      GGEMS_INFOEX("OpenCL", 3,
                   "Reusing OpenCL program '{}' from memory cache.",
                   kernel_name);
      return *program;
    }
  }

  GGEMS_INFOEX("OpenCL", 2, "Creating OpenCL program '{}'.", kernel_name);

  auto program = std::make_unique<GGEMSOpenCLProgram>(
      ctx, kernel_root, kernel_name, build_options);

  GGEMSOpenCLProgram const &reference = *program;
  program_cache_.push_back(std::move(program));

  return reference;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::DisableNvidiaDriverKernelCache() -> void {
#ifdef _MSC_VER
  _putenv_s("CUDA_CACHE_DISABLE", "1");
#else
  setenv("CUDA_CACHE_DISABLE", "1", 1);
#endif

  GGEMS_INFOEX("OpenCL", 3, "NVIDIA kernel cache disabled.");
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::InitPlatformsAndDevices() -> void {
  GGEMS_INFOEX("OpenCL", 2, "Enumerating OpenCL platforms.");

  std::vector<cl::Platform> platforms;
  {
    auto const opencl_error_code = (cl::Platform::get(&platforms));
    ggems::ocl::CheckCLError(opencl_error_code,
                             "No OpenCL platforms detected on this system.");
  }

  platforms_.clear();
  platforms_.reserve(platforms.size());
  std::size_t plat_index{0};
  for (auto const &platform : platforms) {
    platforms_.emplace_back(platform, plat_index++);
  }

  GGEMS_INFOEX("OpenCL", 1, "{} OpenCL platform(s) detected.",
               platforms_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::SelectDevices(std::vector<std::string> const &filters)
    -> void {
  GGEMS_INFOEX("OpenCL", 1, "Selecting OpenCL devices.");

  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices;
  for (auto const &platform : platforms_) {
    for (auto const &device : platform.GetDevices()) {
      all_devices.emplace_back(device);
    }
  }

  if (all_devices.empty()) {
    throw ggems::core::GGEMSFatal("No OpenCL devices found.");
  }

  if (filters.empty()) {
    auto it_gpu =
        std::ranges::find_if(all_devices, [](auto const &device) -> bool {
          return (device.get().GetType() & CL_DEVICE_TYPE_GPU) != 0;
        });

    if (it_gpu != all_devices.end()) {
      selected_devices_ = {*it_gpu};

      GGEMS_INFO("OpenCL", "No filter specified; using first GPU device: {}",
                 it_gpu->get().GetName());
    } else {
      selected_devices_ = {all_devices.front()};

      GGEMS_INFO("OpenCL", "No GPU found; using first available device: {}",
                 all_devices.front().get().GetName());
    }
    return;
  }

  auto selected_devices = ParseDeviceFilters(filters, all_devices);

  if (selected_devices.empty()) {
    throw core::GGEMSFatal("No matching devices for given filters.");
  }

  selected_devices_ = std::move(selected_devices);

  GGEMS_INFO("OpenCL", "{} OpenCL device(s) selected.",
             selected_devices_.size());

  for (std::size_t i = 0; i < selected_devices_.size(); ++i) {
    auto const &device = selected_devices_[i];
    GGEMS_INFO("OpenCL", "[{}] {}  ({} / {})", i, device.get().GetName(),
               device.get().GetVendor(),
               ocl::DeviceTypeToString(device.get().GetType()));
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::Initialize() -> void {
  try {
    CreateContexts();
    GGEMS_INFOEX("OpenCL", 2, "OpenCL backend ready.");
  } catch (ggems::core::GGEMSExceptionBase &) {
    std::terminate();
  } catch (std::exception const &) {
    std::terminate();
  } catch (...) {
    std::terminate();
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::CreateContexts() -> void {
  {
    std::scoped_lock lock{program_cache_mutex_};
    program_cache_.clear();
  }

  contexts_.clear();
  contexts_.reserve(selected_devices_.size());

  for (auto const &dev : selected_devices_) {
    contexts_.emplace_back(dev);
  }

  GGEMS_INFOEX("OpenCL", 2, "{} OpenCL context(s) created.", contexts_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::ParseDeviceFilters(
    std::vector<std::string> const &filters,
    std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> const
        &all_devices)
    -> std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> {
  auto const tokens = TokenizeDeviceSelection(filters);

  if (std::ranges::find(tokens, "all") != tokens.end()) {
    if (tokens.size() != 1U) {
      throw core::GGEMSFatal(
          "OpenCL device selector 'all' cannot be combined with other "
          "selectors.");
    }

    return all_devices;
  }

  auto const criteria =
      ParseDeviceSelectionCriteria(tokens, all_devices.size());

  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> selected;

  if (criteria.has_numeric_selector) {
    selected.reserve(criteria.numeric_indices.size());

    for (auto const index : criteria.numeric_indices) {
      selected.push_back(all_devices[index]);
    }

    return selected;
  }

  for (auto const &device_ref : all_devices) {
    auto const &device = device_ref.get();

    if (criteria.requested_type != 0 &&
        (device.GetType() & criteria.requested_type) == 0) {
      continue;
    }

    if (!criteria.requested_vendor.empty() &&
        !NormalizeDeviceSelectionText(device.GetVendor())
             .contains(criteria.requested_vendor)) {
      continue;
    }

    selected.push_back(device_ref);
  }

  return selected;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::PrintPlatforms() const -> void {
  GGEMS_INFO("OpenCL", "Available OpenCL platforms:");

  for (auto const &platform : platforms_) {
    platform.Print();
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::PrintDevices() const -> void {
  GGEMS_INFO("OpenCL", "Available OpenCL devices:");

  for (auto const &platform : platforms_) {
    auto const &devices = platform.GetDevices();
    for (auto const &device : devices) {
      device.Print();
    }
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::PrintContexts() const -> void {
  GGEMS_INFO("OpenCL", "Active OpenCL contexts:");

  for (auto const &context : contexts_) {
    context.PrintContext();
    context.PrintCommandQueue();
  }
}

} // namespace ggems::ocl
