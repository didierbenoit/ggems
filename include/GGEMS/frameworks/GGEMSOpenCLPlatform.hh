#pragma once

// ************************************************************************
// ************************************************************************


#include <string>
#include <unordered_set>
#include <vector>

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {
// Forward declaration to decouple headers (the destructor is out-of-line).
class GGEMSOpenCLDevice;

class GGEMSOpenCLPlatform {
public:
  explicit GGEMSOpenCLPlatform(cl::Platform const &platform,
                               std::size_t platform_index);

  GGEMSOpenCLPlatform() = delete;

  ~GGEMSOpenCLPlatform();

  // Non-copyable, non-movable — preserves ownership and index stability.
  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform const &) = delete;
  GGEMSOpenCLPlatform &operator=(GGEMSOpenCLPlatform const &) = delete;

  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform &&) = default;

  GGEMSOpenCLPlatform &operator=(GGEMSOpenCLPlatform &&) = delete;

public:
  // -------------------- High-level inspection API --------------------

  [[nodiscard]] bool CheckExtension(std::string_view extension_name) const;

  [[nodiscard]] std::string GetName() const;

  [[nodiscard]] std::string GetProfile() const;

  [[nodiscard]] std::string GetVersion() const;

  [[nodiscard]] std::string GetVendor() const;

  [[nodiscard]] std::string GetExtensions() const;

  [[nodiscard]] cl_version GetNumericVersion() const;

  [[nodiscard]] cl_ulong GetHostTimerResolution() const;

  [[nodiscard]] std::vector<cl_name_version> GetExtensionsWithVersion() const;

  void Print() const;

  void Clean();

  // -------------------- Accessors for orchestration layers
  // --------------------

  [[nodiscard]] std::size_t GetPlatformIndex() const noexcept {
    return platform_index_;
  }

  [[nodiscard]] cl::Platform const &GetPlatformNative() const noexcept {
    return platform_;
  }

  [[nodiscard]] std::vector<GGEMSOpenCLDevice> const &
  GetDevices() const noexcept {
    return devices_;
  }

private:
  void PrintIdentity() const;

  void PrintExtension() const;

private:
  // -------------------- Internal discovery --------------------

  void DiscoverDevices();

private:
  cl::Platform platform_;
  std::size_t platform_index_;
  std::unordered_set<std::string>
      extensions_;
  std::vector<GGEMSOpenCLDevice> devices_;
};
} // namespace ggems::ocl
