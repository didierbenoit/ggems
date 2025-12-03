#pragma once

#include <cstdint>
#include <filesystem>

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {
class GGEMSOpenCLProgram {
public:
  GGEMSOpenCLProgram(GGEMSOpenCLContext &ctx, std::filesystem::path kernel_root,
                     std::string kernel_name, std::string build_options = {});

  ~GGEMSOpenCLProgram() noexcept;

  GGEMSOpenCLProgram(GGEMSOpenCLProgram const &) = delete;
  GGEMSOpenCLProgram &operator=(GGEMSOpenCLProgram const &) = delete;

  GGEMSOpenCLProgram(GGEMSOpenCLProgram &&) noexcept = delete;
  GGEMSOpenCLProgram &operator=(GGEMSOpenCLProgram &&) noexcept = delete;

public:
  cl::Kernel CreateKernel(std::string const &kernel_name);

  [[nodiscard]]
  cl_program const &GetRawProgram() const noexcept {
    return program_();
  }

  cl::Program const &GetProgram() const noexcept { return program_; }

  [[nodiscard]]
  std::string_view GetKernelName() const noexcept {
    return kernel_name_;
  }

  [[nodiscard]]
  std::string_view GetSourcePath() const noexcept {
    return source_path_;
  }

  [[nodiscard]]
  std::string_view GetBuildOptions() const noexcept {
    return build_options_;
  }

  [[nodiscard]] cl_uint GetNumDevices() const;
  [[nodiscard]] std::vector<std::size_t> GetBinarySizes() const;
  [[nodiscard]] auto GetBinaries() const;

private:
  [[nodiscard]]
  static std::string LoadTextFile(std::filesystem::path const &path);

  [[nodiscard]] std::vector<std::string> BuildOptions() const;
  [[nodiscard]] std::string MergeOptions(std::vector<std::string> const &base,
                                         std::string const &extra) const;

  void Initialise();

  void Build();
  void BuildFromSource(std::string const &src);
  void BuildFromBinary(std::vector<std::uint8_t> const &binary);

  std::filesystem::path ComputeCachePath() const;
  void SaveBinaryToCache();
  std::vector<std::uint8_t> LoadBinaryFromCache();

private:
  GGEMSOpenCLContext &context_;
  std::filesystem::path kernel_root_;
  std::string kernel_name_;
  std::string source_path_;
  std::string build_options_;
  std::string build_log_;
  cl::Program program_;
  bool loaded_from_cache_{false};
  std::uint64_t source_hash_;
  std::uint64_t global_hash_;
};
} // namespace ggems::ocl
