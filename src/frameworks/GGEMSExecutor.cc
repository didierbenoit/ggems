/// \cond
#include <algorithm>
#include <functional>
#include <set>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSExecutor.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"

using ggems::core::GGEMSFatal;
using ggems::core::Throw;
using ggems::ocl::GGEMSOpenCL;
using ggems::ocl::GGEMSOpenCLDevice;

namespace ggems::run {
static std::string lower(std::string s) {
  for (char &c : s)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

// Alias vendors mapping (to normalise searches)
static const std::unordered_map<std::string, std::string> vendor_aliases = {
    {"intel", "intel(r) corporation"},
    {"nvidia", "nvidia corporation"},
    {"amd", "advanced micro devices, inc."},
    {"apple", "apple"},
    {"arm", "arm"},
};

std::vector<GGEMSOpenCLDevice>
ParseDeviceFilters(std::vector<std::string> const &filters,
                   std::vector<GGEMSOpenCLDevice> const &all_devices) {
  std::vector<GGEMSOpenCLDevice> selected;

  // filtre to lower case
  std::vector<std::string> lower_filters;
  lower_filters.reserve(filters.size());
  for (auto const &f : filters) {
    std::string lf = lower(f);
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

  std::vector<std::function<bool(GGEMSOpenCLDevice const &)>> predicates;
  for (auto const &f : lower_filters) {
    if (f == "gpu") {
      predicates.push_back([](auto const &d) {
        return lower(ocl::DeviceTypeToString(d.GetType())).find("gpu") !=
               std::string::npos;
      });
    } else if (f == "cpu") {
      predicates.push_back([](auto const &d) {
        return lower(ocl::DeviceTypeToString(d.GetType())).find("cpu") !=
               std::string::npos;
      });
    } else if (vendor_aliases.contains(f)) {
      std::string vendor_name = vendor_aliases.find(f)->second;
      predicates.push_back([vendor_name](auto const &d) {
        return lower(d.GetVendor()).find(vendor_name) != std::string::npos;
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

    if (numeric_indices.contains(idx))
      match = true;

    if (match)
      selected.push_back(dev);
  }

  return selected;
}

GGEMSExecutor::GGEMSExecutor() {
  GGEMS_INFOEX("GGEMS", 3, "GGEMSExecutor created.");
  Banner();
}

GGEMSExecutor::~GGEMSExecutor() {
  GGEMS_INFOEX("GGEMS", 3, "GGEMSExecutor destroyed.");
}

void GGEMSExecutor::Banner() const {
  constexpr std::string_view GGEMS_BANNER = R"(

╭──────────────────────────────────────────────────╮
|   ██████╗  ██████╗ ███████╗███╗   ███╗███████╗   |
|  ██╔════╝ ██╔════╝ ██╔════╝████╗ ████║██╔════╝   |
|  ██║  ███╗██║  ███╗█████╗  ██╔████╔██║███████╗   |
|  ██║   ██║██║   ██║██╔══╝  ██║╚██╔╝██║╚════██║   |
|  ╚██████╔╝╚██████╔╝███████╗██║ ╚═╝ ██║███████║   |
|   ╚═════╝  ╚═════╝ ╚══════╝╚═╝     ╚═╝╚══════╝   |
|                                                  |
|    GPU Geant4-based Monte Carlo Simulations      |
|   Version 2.0 • GGEMS Team • https://ggems.fr    |
|     Authors: Julien Bert  &  Didier Benoit       |
|  Copyright © 2025  Licensed under GNU GPL v3.0   |
╰──────────────────────────────────────────────────╯
)";
  GGEMS_INFO("Run", "{}", GGEMS_BANNER);
}

void GGEMSExecutor::SelectDevices(std::vector<std::string> const &filters) {
  GGEMS_INFO("Run", "Selecting OpenCL devices...");

  auto &opencl = GGEMSOpenCL::GetInstance();
  auto const &platforms = opencl.GetPlatforms();

  std::vector<GGEMSOpenCLDevice> all_devices;

  // --- Collect all devices from all platforms ----------------------------
  for (auto const &platform : platforms) {
    for (auto const &dev : platform.GetDevices()) {
      all_devices.push_back(*dev);
    }
  }

  if (all_devices.empty())
    Throw<GGEMSFatal>("No OpenCL devices found.");

  // --- Default behaviour --------------------------------------------------
  if (filters.empty()) {
    auto it_gpu =
        std::find_if(all_devices.begin(), all_devices.end(), [](auto const &d) {
          return (d.GetType() & CL_DEVICE_TYPE_GPU) != 0;
        });
    if (it_gpu != all_devices.end()) {
      devices_.push_back(*it_gpu);
      GGEMS_INFO("Run", "No filter specified — using first GPU device: {}",
                 it_gpu->GetName());
    } else {
      devices_.push_back(all_devices.front());
      GGEMS_INFO("Run", "No GPU found — using first available device: {}",
                 all_devices.front().GetName());
    }
    return;
  }
  devices_ = ParseDeviceFilters(filters, all_devices);

  if (devices_.empty())
    Throw<GGEMSFatal>("No matching devices for given filters.");

  GGEMS_INFO("Run", "Total devices selected: {}", devices_.size());
  for (std::size_t i = 0; i < devices_.size(); ++i) {
    auto const &d = devices_[i];
    GGEMS_INFO("Run", "[{}] {}  ({} / {})", i, d.GetName(), d.GetVendor(),
               ocl::DeviceTypeToString(d.GetType()));
  }
}

void GGEMSExecutor::CreateContexts() {
  contexts_.clear();
  contexts_.reserve(devices_.size());

  for (auto const &dev : devices_)
    contexts_.emplace_back(dev);

  GGEMS_INFO("Run", "Created {} OpenCL contexts.", contexts_.size());
}

void GGEMSExecutor::Initialize() {
  GGEMS_INFOEX("Run", 2, "Initialising GGEMSExecutor...");

  try {
    CreateContexts();
    GGEMS_INFOEX("OpenCL", 1, "GGEMSOpenCL successfully constructed!");
  } catch (ggems::core::GGEMSExceptionBase &) {
    std::terminate();
  } catch (std::exception const &) {
    std::terminate();
  } catch (...) {
    std::terminate();
  }

  GGEMS_INFOEX("Run", 2, "GGEMSExecutor initialized.");
}

void GGEMSExecutor::Run() { ; }

} // namespace ggems::run
