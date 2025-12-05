#pragma once

#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"

namespace ggems::ocl {

class GGEMSOpenCLPlatform;
class GGEMSOpenCLContext;
class GGEMSOpenCLDevice;

class GGEMSOpenCL {
private:
  GGEMSOpenCL();

  GGEMSOpenCL(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL(GGEMSOpenCL const &&openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL const &&openCL) = delete;

public:
  [[nodiscard]] static GGEMSOpenCL &GetInstance() {
    static GGEMSOpenCL *instance = []() {
      GGEMS_INFOEX("OpenCL", 2, "First Instance of GGEMSOpenCL singleton...");
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
  void Initialise();

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
  void DisableKernelCache() const;

private:
  std::vector<GGEMSOpenCLPlatform> platforms_;
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
      selected_devices_;
  std::vector<GGEMSOpenCLContext> contexts_;
  std::vector<std::unique_ptr<GGEMSOpenCLProgram>> program_cache_;
};
} // namespace ggems::ocl
