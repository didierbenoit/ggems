#include <set>
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
#include <functional>
#include <sstream>

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
  for (char &character : text) {
    character =
        static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  }

  return text;
}

// =============================================================================
// =============================================================================

constexpr std::array<std::pair<std::string_view, std::string_view>, 5>
    vendor_aliases{{
        {"intel", "intel(r) corporation"},
        {"nvidia", "nvidia corporation"},
        {"amd", "advanced micro devices, inc."},
        {"apple", "apple"},
        {"arm", "arm"},
    }};
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
  selected_devices_.clear();

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
  selected_devices_ = ParseDeviceFilters(filters, all_devices);

  if (selected_devices_.empty()) {
    throw ggems::core::GGEMSFatal("No matching devices for given filters.");
  }

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
    std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices)
    -> std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> {
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> selected;

  std::vector<std::string> lower_filters;
  lower_filters.reserve(filters.size());
  for (auto const &filter : filters) {
    std::string low_filter = NormalizeDeviceSelectionText(filter);
    lower_filters.push_back(low_filter);
  }

  std::set<std::size_t> numeric_indices;
  for (auto const &low_filter : lower_filters) {
    bool numeric =
        std::ranges::all_of(low_filter, [](unsigned char character) -> bool {
          return std::isdigit(character) || character == ',' ||
                 character == '-';
        });

    if (!numeric) {
      continue;
    }

    std::stringstream lf_stream(low_filter);
    std::string token;
    while (std::getline(lf_stream, token, ',')) {
      auto dash = token.find('-');
      if (dash != std::string::npos) {
        std::size_t start = std::stoul(token.substr(0, dash));
        std::size_t end = std::stoul(token.substr(dash + 1));
        for (std::size_t i = start; i <= end; ++i) {
          numeric_indices.insert(i);
        }
      } else if (!token.empty()) {
        numeric_indices.insert(std::stoul(token));
      }
    }
  }

  if (!numeric_indices.empty()) {
    for (auto const &index : numeric_indices) {
      selected.push_back(all_devices[index]);
    }
    return selected;
  }

  std::vector<std::function<bool(GGEMSOpenCLDevice const &)>> predicates;

  for (auto const &low_filter : lower_filters) {
    if (low_filter == "gpu") {
      predicates.emplace_back([](auto const &device) -> bool {
        return NormalizeDeviceSelectionText(
                   ocl::DeviceTypeToString(device.GetType()))
                   .find("gpu") != std::string::npos;
      });
    } else if (low_filter == "cpu") {
      predicates.emplace_back([](auto const &device) -> bool {
        return NormalizeDeviceSelectionText(
                   ocl::DeviceTypeToString(device.GetType()))
                   .find("cpu") != std::string::npos;
      });
    } else if (auto const vendor_it = std::ranges::find_if(
                   vendor_aliases,
                   [&low_filter](auto const &vendor) -> bool {
                     return vendor.first == low_filter;
                   });
               vendor_it != vendor_aliases.end()) {
      predicates.emplace_back(
          [vendor_name = vendor_it->second](auto const &device) -> bool {
            return NormalizeDeviceSelectionText(device.GetVendor())
                       .find(vendor_name) != std::string::npos;
          });
    }
  }

  for (auto const &device : all_devices) {
    bool match = true;

    for (auto const &predicate : predicates) {
      if (!predicate(device.get())) {
        match = false;
        break;
      }
    }

    if (match) {
      selected.push_back(device);
    }
  }

  return selected;
}

// -----------------------------------------------------------------------------

void GGEMSOpenCL::PrintPlatforms() const {
  GGEMS_INFO("OpenCL", "Available OpenCL platforms:");

  for (auto const &platform : platforms_) {
    platform.Print();
  }
}

// -----------------------------------------------------------------------------

void GGEMSOpenCL::PrintDevices() const {
  GGEMS_INFO("OpenCL", "Available OpenCL devices:");

  for (auto const &platform : platforms_) {
    auto const &devices = platform.GetDevices();
    for (auto const &device : devices) {
      device.Print();
    }
  }
}

// -----------------------------------------------------------------------------

void GGEMSOpenCL::PrintContexts() const {
  GGEMS_INFO("OpenCL", "Active OpenCL contexts:");

  for (auto const &context : contexts_) {
    context.PrintContext();
    context.PrintCommandQueue();
  }
}

// -----------------------------------------------------------------------------

void GGEMSOpenCL::Clean() {
  GGEMS_INFOEX("OpenCL", 3, "Cleaning OpenCL platform resources.");

  for (auto &platform : platforms_) {
    platform.Clean();
  }

  GGEMS_INFOEX("OpenCL", 3, "OpenCL platform resources cleaned.");
}
} // namespace ggems::ocl
