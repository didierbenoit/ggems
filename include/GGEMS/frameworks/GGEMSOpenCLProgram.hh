#pragma once

/// \cond
#include <cstdint>
#include <filesystem>
/// \encond

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {
class GGEMSOpenCLProgram {
public:
  GGEMSOpenCLProgram(GGEMSOpenCLContext &ctx, std::filesystem::path kernel_root,
                     std::string kernel_name, std::string build_options = "");
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

  [[nodiscard]]
  cl::Program const &GetProgram() const noexcept {
    return program_;
  }

  [[nodiscard]]
  std::string_view GetKernelName() const noexcept {
    return kernel_name_;
  }

  [[nodiscard]]
  std::string_view GetSourcePath() const noexcept {
    return source_path_;
  }

  [[nodiscard]]
  std::string_view GetSpirVPath() const noexcept {
    return spirv_path_;
  }

  [[nodiscard]]
  std::string_view GetBuildOptions() const noexcept {
    return build_options_;
  }

  [[nodiscard]]
  std::string_view GetBuildLog() const noexcept {
    return build_log_;
  }

private:
  [[nodiscard]]
  std::string LoadTextFile(std::filesystem::path const &path);
  std::vector<std::uint8_t> LoadBinaryFile(std::filesystem::path const &path);

  void Build();
  void BuildFromSPIRV(std::vector<std::uint8_t> const &il);
  void BuildFromSource(std::string const &src);

private:
  GGEMSOpenCLContext &context_;
  std::filesystem::path kernel_root_;
  std::string kernel_name_;
  std::string source_path_;
  std::string spirv_path_;
  std::string build_options_;
  std::string build_log_;
  cl::Program program_;
  bool can_use_il_{false};
};
} // namespace ggems::ocl
