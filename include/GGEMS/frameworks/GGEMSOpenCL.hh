#pragma once

#include <mutex>

#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"

namespace ggems::ocl {

class GGEMSOpenCLPlatform;
class GGEMSOpenCLContext;
class GGEMSOpenCLDevice;

class GGEMSOpenCL {
private:
  GGEMSOpenCL();

  GGEMSOpenCL(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL(GGEMSOpenCL &&openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL &&openCL) = delete;

public:
  [[nodiscard]] static GGEMSOpenCL &GetInstance() {
    static GGEMSOpenCL *instance = []() {
      GGEMS_INFOEX("OpenCL", 3, "Creating GGEMSOpenCL singleton instance.");
      return new GGEMSOpenCL();
    }();
    return *instance;
  }

  GGEMSOpenCLProgram &GetOrCreateProgram(
      GGEMSOpenCLContext &ctx, std::filesystem::path const &kernel_root,
      std::string const &kernel_name, std::string const &build_options = "");

  ~GGEMSOpenCL();

  void Clean() noexcept;

  void PrintPlatforms() const noexcept;

  void PrintDevices() const noexcept;

  void PrintContexts() const noexcept;

  [[nodiscard]] std::vector<GGEMSOpenCLPlatform> const &
  GetPlatforms() const noexcept {
    return platforms_;
  }

  void SelectDevices(std::vector<std::string> const &filters);

  void Initialize();

  [[nodiscard]]
  std::vector<GGEMSOpenCLContext> &GetContext() noexcept {
    return contexts_;
  }

private:
  void InitPlatformsAndDevices();

  [[nodiscard]]
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
  ParseDeviceFilters(
      std::vector<std::string> const &filters,
      std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices);

  void CreateContexts();

  void DisableNvidiaDriverKernelCache() const;

private:
  std::vector<GGEMSOpenCLPlatform> platforms_;
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
      selected_devices_;
  std::vector<GGEMSOpenCLContext> contexts_;
  std::vector<std::unique_ptr<GGEMSOpenCLProgram>> program_cache_;
  std::mutex program_cache_mutex_;
};
} // namespace ggems::ocl
