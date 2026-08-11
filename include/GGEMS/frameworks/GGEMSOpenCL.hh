#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <memory>

#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"

namespace ggems::ocl {

class GGEMSOpenCLPlatform;
class GGEMSOpenCLContext;
class GGEMSOpenCLDevice;

class GGEMSOpenCL {
public:
  ~GGEMSOpenCL();

  GGEMSOpenCL(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL(GGEMSOpenCL &&openCL) = delete;
  auto operator=(GGEMSOpenCL const &openCL) -> GGEMSOpenCL & = delete;
  auto operator=(GGEMSOpenCL &&openCL) -> GGEMSOpenCL & = delete;

  [[nodiscard]] static auto GetInstance() -> GGEMSOpenCL & {
    static GGEMSOpenCL *instance = []() -> GGEMSOpenCL * {
      GGEMS_INFOEX("OpenCL", 3, "Creating GGEMSOpenCL singleton instance.");
      return new GGEMSOpenCL();
    }();
    return *instance;
  }

  auto GetOrCreateProgram(GGEMSOpenCLContext const &ctx,
                          std::filesystem::path const &kernel_root,
                          std::string const &kernel_name,
                          std::string const &build_options = "")
      -> GGEMSOpenCLProgram const &;

  void Clean();

  void PrintPlatforms() const;

  void PrintDevices() const;

  void PrintContexts() const;

  [[nodiscard]] auto GetPlatforms() const noexcept
      -> std::vector<GGEMSOpenCLPlatform> const & {
    return platforms_;
  }

  auto SelectDevices(std::vector<std::string> const &filters) -> void;

  auto Initialize() -> void;

  [[nodiscard]]
  auto GetContext() noexcept -> std::vector<GGEMSOpenCLContext> & {
    return contexts_;
  }

private:
  GGEMSOpenCL();
  void InitPlatformsAndDevices();

  [[nodiscard]]
  static auto ParseDeviceFilters(
      std::vector<std::string> const &filters,
      std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices)
      -> std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>;

  auto CreateContexts() -> void;

  static auto DisableNvidiaDriverKernelCache() -> void;

  std::vector<GGEMSOpenCLPlatform> platforms_;
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
      selected_devices_;
  std::vector<GGEMSOpenCLContext> contexts_;
  std::vector<std::unique_ptr<GGEMSOpenCLProgram>> program_cache_;
  std::mutex program_cache_mutex_;
};
} // namespace ggems::ocl
