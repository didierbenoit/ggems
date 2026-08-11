#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include <cstddef>
#include <string_view>

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

class GGEMSOpenCLDevice;

class GGEMSOpenCLPlatform {
public:
  explicit GGEMSOpenCLPlatform(cl::Platform platform,
                               std::size_t platform_index);

  GGEMSOpenCLPlatform() = delete;

  ~GGEMSOpenCLPlatform();

  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform const &) = delete;
  auto operator=(GGEMSOpenCLPlatform const &) -> GGEMSOpenCLPlatform & = delete;
  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform &&) noexcept = default;
  auto operator=(GGEMSOpenCLPlatform &&) -> GGEMSOpenCLPlatform & = delete;

  [[nodiscard]] auto CheckExtension(std::string_view extension_name) const
      -> bool;
  [[nodiscard]] auto GetName() const -> std::string;

  [[nodiscard]] auto GetProfile() const -> std::string;

  [[nodiscard]] auto GetVersion() const -> std::string;

  [[nodiscard]] auto GetVendor() const -> std::string;

  [[nodiscard]] auto GetExtensions() const -> std::string;

  [[nodiscard]] auto GetNumericVersion() const -> cl_version;

  [[nodiscard]] auto GetHostTimerResolution() const -> cl_ulong;

  [[nodiscard]] auto GetExtensionsWithVersion() const
      -> std::vector<cl_name_version>;

  auto Print() const -> void;
  auto Clean() -> void;

  [[nodiscard]] auto GetPlatformIndex() const noexcept -> std::size_t {
    return platform_index_;
  }

  [[nodiscard]] auto GetPlatformNative() const noexcept
      -> cl::Platform const & {
    return platform_;
  }

  [[nodiscard]] auto GetDevices() const noexcept
      -> std::vector<GGEMSOpenCLDevice> const & {
    return devices_;
  }

private:
  auto PrintIdentity() const -> void;
  auto PrintExtension() const -> void;
  auto DiscoverDevices() -> void;

  cl::Platform platform_;
  std::size_t platform_index_;
  std::unordered_set<std::string> extensions_;
  std::vector<GGEMSOpenCLDevice> devices_;
};
} // namespace ggems::ocl
