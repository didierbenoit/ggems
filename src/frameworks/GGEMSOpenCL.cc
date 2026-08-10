// ************************************************************************
// ************************************************************************


#include <set>
#include <algorithm>
#include <cctype>
#include <exception>
#include <string>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static const std::unordered_map<std::string, std::string> vendor_aliases = {
    {"intel", "intel(r) corporation"},
    {"nvidia", "nvidia corporation"},
    {"amd", "advanced micro devices, inc."},
    {"apple", "apple"},
    {"arm", "arm"},
};

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSOpenCL::~GGEMSOpenCL() {
  GGEMS_INFOEX(
      "OpenCL", 3,
      "GGEMSOpenCL singleton destroyed; memory intentionally retained.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

auto GGEMSOpenCL::GetOrCreateProgram(
    GGEMSOpenCLContext const &ctx,
    std::filesystem::path const &kernel_root,
    std::string const &kernel_name, std::string const &build_options)
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::DisableNvidiaDriverKernelCache() const {
#ifdef _MSC_VER
  static char env_var[] = "CUDA_CACHE_DISABLE=1";
  _putenv(env_var);
#else
  setenv("CUDA_CACHE_DISABLE", "1", 1);
#endif

  GGEMS_INFOEX("OpenCL", 3, "NVIDIA kernel cache disabled.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::InitPlatformsAndDevices() {
  GGEMS_INFOEX("OpenCL", 2, "Enumerating OpenCL platforms.");

  std::vector<cl::Platform> platforms;
  {
    auto const opencl_error_code = (cl::Platform::get(&platforms));
    ggems::ocl::CheckCLError(opencl_error_code, "No OpenCL platforms detected on this system.");
  }

  platforms_.clear();
  platforms_.reserve(platforms.size());
  std::size_t plat_index{0};
  for (auto const &p : platforms) {
    platforms_.emplace_back(p, plat_index++);
  }

  GGEMS_INFOEX("OpenCL", 1, "{} OpenCL platform(s) detected.",
               platforms_.size());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::SelectDevices(std::vector<std::string> const &filters) {
  selected_devices_.clear();

  GGEMS_INFOEX("OpenCL", 1, "Selecting OpenCL devices.");

  // --- Collect all devices from all platforms ----------------------------
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices;
  for (auto const &platform : platforms_) {
    for (auto const &dev : platform.GetDevices()) {
      all_devices.push_back(dev);
    }
  }

  if (all_devices.empty()) {
    throw ggems::core::GGEMSFatal("No OpenCL devices found.");
  }

  // --- Default behavior --------------------------------------------------
  if (filters.empty()) {
    auto it_gpu =
        std::find_if(all_devices.begin(), all_devices.end(), [](auto const &d) {
          return (d.get().GetType() & CL_DEVICE_TYPE_GPU) != 0;
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
    auto const &d = selected_devices_[i];
    GGEMS_INFO("OpenCL", "[{}] {}  ({} / {})", i, d.get().GetName(),
               d.get().GetVendor(), ocl::DeviceTypeToString(d.get().GetType()));
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::Initialize() {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::CreateContexts() {
  program_cache_.clear();

  contexts_.clear();
  contexts_.reserve(selected_devices_.size());

  for (auto const &dev : selected_devices_)
    contexts_.emplace_back(dev);

  GGEMS_INFOEX("OpenCL", 2, "{} OpenCL context(s) created.", contexts_.size());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
GGEMSOpenCL::ParseDeviceFilters(
    std::vector<std::string> const &filters,
    std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices) {
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> selected;

  // filtre to lower case
  std::vector<std::string> lower_filters;
  lower_filters.reserve(filters.size());
  for (auto const &f : filters) {
    std::string lf = NormalizeDeviceSelectionText(f);
    lower_filters.push_back(lf);
  }

  // Filtre expansion: "1,3-5" → {1,3,4,5}
  std::set<std::size_t> numeric_indices;
  for (auto const &f : lower_filters) {
    bool numeric = std::ranges::all_of(f, [](unsigned char c) {
      return std::isdigit(c) || c == ',' || c == '-';
    });

    if (!numeric)
      continue;

    std::stringstream ss(f);
    std::string token;
    while (std::getline(ss, token, ',')) {
      auto dash = token.find('-');
      if (dash != std::string::npos) {
        std::size_t start = std::stoul(token.substr(0, dash));
        std::size_t end = std::stoul(token.substr(dash + 1));
        for (std::size_t i = start; i <= end; ++i)
          numeric_indices.insert(i);
      } else if (!token.empty()) {
        numeric_indices.insert(std::stoul(token));
      }
    }
  }

  if (!numeric_indices.empty()) {
    for (auto const &i : numeric_indices) {
      selected.push_back(all_devices[i]);
    }
    return selected;
  }

  std::vector<std::function<bool(GGEMSOpenCLDevice const &)>> predicates;
  for (auto const &f : lower_filters) {
    if (f == "gpu") {
      predicates.push_back([](auto const &d) {
        return NormalizeDeviceSelectionText(
                   ocl::DeviceTypeToString(d.GetType()))
                   .find("gpu") != std::string::npos;
      });
    } else if (f == "cpu") {
      predicates.push_back([](auto const &d) {
        return NormalizeDeviceSelectionText(
                   ocl::DeviceTypeToString(d.GetType()))
                   .find("cpu") != std::string::npos;
      });
    } else if (vendor_aliases.contains(f)) {
      std::string vendor_name = vendor_aliases.find(f)->second;
      predicates.push_back([vendor_name](auto const &d) {
        return NormalizeDeviceSelectionText(d.GetVendor()).find(vendor_name) !=
               std::string::npos;
      });
    }
  }

  // Filtres
  for (std::size_t idx = 0; idx < all_devices.size(); ++idx) {
    auto const &dev = all_devices[idx];

    bool match = true;

    for (auto const &pred : predicates) {
      if (!pred(dev)) {
        match = false;
        break;
      }
    }

    if (match)
      selected.push_back(dev);
  }

  return selected;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::PrintPlatforms() const {
  GGEMS_INFO("OpenCL", "Available OpenCL platforms:");

  for (auto const &p : platforms_) {
    p.Print();
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::PrintDevices() const {
  GGEMS_INFO("OpenCL", "Available OpenCL devices:");

  for (auto const &p : platforms_) {
    auto const &devices = p.GetDevices();
    for (auto const &d : devices)
      d.Print();
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::PrintContexts() const {
  GGEMS_INFO("OpenCL", "Active OpenCL contexts:");

  for (auto const &c : contexts_) {
    c.PrintContext();
    c.PrintCommandQueue();
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCL::Clean() {
  GGEMS_INFOEX("OpenCL", 3, "Cleaning OpenCL platform resources.");

  for (auto &p : platforms_) {
    p.Clean();
  }

  GGEMS_INFOEX("OpenCL", 3, "OpenCL platform resources cleaned.");
}
} // namespace ggems::ocl
