#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {
class GGEMSOpenCL;

class GGEMSOpenCLProgram {
  friend class GGEMSOpenCL;

public:
  ~GGEMSOpenCLProgram() = default;

  GGEMSOpenCLProgram(GGEMSOpenCLProgram const &) = delete;
  auto operator=(GGEMSOpenCLProgram const &) -> GGEMSOpenCLProgram & = delete;
  GGEMSOpenCLProgram(GGEMSOpenCLProgram &&) noexcept = delete;
  auto operator=(GGEMSOpenCLProgram &&) noexcept
      -> GGEMSOpenCLProgram & = delete;

  [[nodiscard]] auto CreateKernel(std::string const &kernel_name) const
      -> cl::Kernel;

  [[nodiscard]] auto GetProgramNative() const noexcept -> cl::Program const & {
    return program_;
  }

  [[nodiscard]] auto GetKernelName() const noexcept -> std::string_view {
    return kernel_name_;
  }

  [[nodiscard]] auto GetSourcePath() const noexcept -> std::string_view {
    return source_path_;
  }

  [[nodiscard]] auto GetBuildOptions() const noexcept -> std::string_view {
    return build_options_;
  }

  [[nodiscard]] auto GetNumDevices() const -> cl_uint;

  [[nodiscard]] auto GetBinarySizes() const -> std::vector<std::size_t>;

  [[nodiscard]] auto GetBinaries() const
      -> std::vector<std::vector<unsigned char>>;

  [[nodiscard]] auto Matches(GGEMSOpenCLContext const &context,
                             std::filesystem::path const &kernel_root,
                             std::string_view kernel_name,
                             std::string_view user_build_options) const -> bool;

private:
  GGEMSOpenCLProgram(GGEMSOpenCLContext const &context,
                     std::filesystem::path kernel_root, std::string kernel_name,
                     std::string build_options = {});

  [[nodiscard]]
  static auto LoadTextFile(std::filesystem::path const &path) -> std::string;

  [[nodiscard]] auto BuildOptions() const -> std::vector<std::string>;

  [[nodiscard]] static auto MergeOptions(std::vector<std::string> const &base,
                                         std::string const &extra)
      -> std::string;

  auto Initialize() -> void;

  auto Build() -> void;

  auto BuildFromSource(std::string const &source) -> void;

  auto BuildFromBinary(std::vector<std::uint8_t> const &binary) -> void;

  [[nodiscard]] auto ComputeCachePath() const -> std::filesystem::path;

  auto SaveBinaryToCache() -> void;

  auto LoadBinaryFromCache() -> std::vector<std::uint8_t>;

  [[nodiscard]] auto BuildIncludeSearchRoots() const
      -> std::vector<std::filesystem::path>;

  [[nodiscard]] auto
  BuildSourceFingerprintText(std::filesystem::path const &source_path) const
      -> std::string;

  auto AppendSourceFingerprintText(
      std::filesystem::path const &source_path,
      std::vector<std::filesystem::path> const &include_roots,
      std::unordered_set<std::string> &visited_sources,
      std::string &fingerprint_text) const -> void;

  GGEMSOpenCLContext const &context_;
  std::filesystem::path kernel_root_;
  std::string kernel_name_;
  std::string source_path_;
  std::string user_build_options_;
  std::string build_options_;
  std::string build_log_;
  cl::Program program_;
  std::uint64_t source_hash_;
  std::uint64_t global_hash_;
};
} // namespace ggems::ocl
