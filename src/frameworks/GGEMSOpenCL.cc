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

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"

namespace ggems::ocl {

namespace {

// =============================================================================
// =============================================================================

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

constexpr std::array<std::pair<std::string_view, std::string_view>, 5>
    vendor_aliases{{{"intel", "intel(r) corporation"},
                    {"nvidia", "nvidia corporation"},
                    {"amd", "advanced micro devices, inc."}}};

// =============================================================================
// =============================================================================

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

  auto prog = std::unique_ptr<GGEMSOpenCLProgram>(
      new GGEMSOpenCLProgram(ctx, kernel_root, kernel_name, build_options));

  GGEMSOpenCLProgram const &ref = *prog;
  program_cache_.push_back(std::move(prog));

  return ref;
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
      selected_devices_.push_back(*it_gpu);
      GGEMS_INFO("OpenCL", "No filter specified; using first GPU device: {}",
                 it_gpu->get().GetName());
    } else {
      selected_devices_.push_back(all_devices.front());
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
  program_cache_.clear();

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

  std::vector<std::size_t> numeric_indices;
  cl_device_type requested_type{0};
  std::string_view requested_vendor;

  bool has_numeric_selector{false};
  bool has_textual_selector{false};

  auto append_index = [&](std::size_t index,
                          std::string_view selector) -> void {
    if (index >= all_devices.size()) {
      throw core::GGEMSFatal(
          "OpenCL device index is out of range in selector '" +
          std::string{selector} + "'.");
    }

    if (std::ranges::find(numeric_indices, index) == numeric_indices.end()) {
      numeric_indices.push_back(index);
    }
  };

  for (auto const &token : tokens) {
    if (token == "cpu" || token == "gpu") {
      has_textual_selector = true;

      cl_device_type const type =
          token == "cpu" ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU;

      if (requested_type != 0 && requested_type != type) {
        throw core::GGEMSFatal(
            "OpenCL device selector cannot combine 'cpu' and 'gpu'.");
      }

      requested_type = type;
      continue;
    }

    auto const vendor_it = std::ranges::find_if(
        vendor_aliases,
        [&token](auto const &vendor) -> bool { return vendor.first == token; });

    if (vendor_it != vendor_aliases.end()) {
      has_textual_selector = true;

      if (!requested_vendor.empty() && requested_vendor != vendor_it->second) {
        throw core::GGEMSFatal(
            "OpenCL device selector cannot combine multiple vendors.");
      }

      requested_vendor = vendor_it->second;
      continue;
    }

    if (!token.empty() &&
        std::isdigit(static_cast<unsigned char>(token.front())) != 0) {
      has_numeric_selector = true;

      auto const dash = token.find('-');

      if (dash == std::string::npos) {
        append_index(ParseDeviceIndex(token), token);
        continue;
      }

      if (dash == 0U || dash + 1U >= token.size() ||
          token.find('-', dash + 1U) != std::string::npos) {
        throw core::GGEMSFatal("Invalid OpenCL device range '" + token + "'.");
      }

      auto const start =
          ParseDeviceIndex(std::string_view{token}.substr(0U, dash));
      auto const end =
          ParseDeviceIndex(std::string_view{token}.substr(dash + 1U));

      if (start > end) {
        throw core::GGEMSFatal(
            "OpenCL device range start exceeds range end in selector '" +
            token + "'.");
      }

      if (end >= all_devices.size()) {
        throw core::GGEMSFatal(
            "OpenCL device index is out of range in selector '" + token + "'.");
      }

      for (std::size_t index = start; index <= end; ++index) {
        append_index(index, token);
      }

      continue;
    }

    throw core::GGEMSFatal("Unknown OpenCL device selector '" + token + "'.");
  }

  if (has_numeric_selector && has_textual_selector) {
    throw core::GGEMSFatal(
        "OpenCL device selector cannot combine numeric and textual "
        "selectors.");
  }

  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> selected;

  if (has_numeric_selector) {
    selected.reserve(numeric_indices.size());

    for (auto const index : numeric_indices) {
      selected.push_back(all_devices[index]);
    }

    return selected;
  }

  for (auto const &device_ref : all_devices) {
    auto const &device = device_ref.get();

    if (requested_type != 0 && (device.GetType() & requested_type) == 0) {
      continue;
    }

    if (!requested_vendor.empty() &&
        NormalizeDeviceSelectionText(device.GetVendor())
                .find(requested_vendor) == std::string::npos) {
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

// -----------------------------------------------------------------------------

auto GGEMSOpenCL::Clean() -> void {
  GGEMS_INFOEX("OpenCL", 3, "Cleaning OpenCL platform resources.");

  for (auto &platform : platforms_) {
    platform.Clean();
  }

  GGEMS_INFOEX("OpenCL", 3, "OpenCL platform resources cleaned.");
}
} // namespace ggems::ocl
