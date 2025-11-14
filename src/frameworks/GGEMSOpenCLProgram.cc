#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

/// \cond
#include <fstream>
#include <utility>
/// \endcond

namespace ggems::ocl {
using core::GGEMSFatal;
using core::Throw;

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

GGEMSOpenCLProgram::GGEMSOpenCLProgram(GGEMSOpenCLContext &ctx,
                                       std::filesystem::path kernel_root,
                                       std::string kernel_name,
                                       std::string build_options)
    : context_(ctx), kernel_root_(std::move(kernel_root)),
      kernel_name_(std::move(kernel_name)),
      build_options_(std::move(build_options)) {
  auto cl_path = kernel_root_ / (kernel_name_ + ".cl");
  auto spirv_path = kernel_root_ / (kernel_name_ + ".spv");

  source_path_ = cl_path.string();
  spirv_path_ = spirv_path.string();

  can_use_il_ = context_.SupportILProgram();

  GGEMS_INFO("OpenCL", "Initialising program '{}' (root: '{}')", kernel_name_,
             kernel_root_.string());

  Build();

  GGEMS_INFO("OpenCL", "Program '{}' built.", kernel_name_);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

GGEMSOpenCLProgram::~GGEMSOpenCLProgram() noexcept {
  GGEMS_INFOEX("OpenCL", 2, "Destroying program '{}'.", source_path_);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

void GGEMSOpenCLProgram::Build() {
  namespace fs = std::filesystem;

  fs::path spv{spirv_path_};
  fs::path cl{source_path_};

  bool has_spirv = fs::exists(spv);

  if (can_use_il_ && has_spirv) {
    GGEMS_INFO("OpenCL", "Trying SPIR-V for '{}': '{}'", kernel_name_,
               spirv_path_);
    auto il = LoadBinaryFile(spv);
    BuildFromSPIRV(il);
    return;
  }

  GGEMS_INFO("OpenCL", "Falling back to source for '{}': '{}'", kernel_name_,
             source_path_);

  auto src = LoadTextFile(cl);
  BuildFromSource(src);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

std::string
GGEMSOpenCLProgram::LoadTextFile(std::filesystem::path const &path) {
  std::ifstream ifs(path, std::ios::binary);
  GGEMS_CHECK(
      ifs.good(),
      std::format("Failed to open program source file '{}'.", path.string()));

  std::ostringstream oss;
  oss << ifs.rdbuf();
  return oss.str();
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

std::vector<std::uint8_t>
GGEMSOpenCLProgram::LoadBinaryFile(std::filesystem::path const &path) {
  std::ifstream ifs(path, std::ios::binary);
  GGEMS_CHECK(ifs.good(),
              std::format("Failed to open SPIR-V file '{}'.", path.string()));

  ifs.seekg(0, std::ios::end);
  std::streamsize size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
  if (!ifs.read(reinterpret_cast<char *>(data.data()), size)) {
    Throw<GGEMSFatal>(
        std::format("Failed to read SPIR-V file '{}'.", path.string()));
  }
  return data;
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

void GGEMSOpenCLProgram::BuildFromSource(std::string const &src) {
  auto &ctx = context_.GetNative();
  auto &device = context_.GetDevice().GetNative();

  cl::Program::Sources sources;
  sources.push_back({src.c_str(), src.size()});

  program_ = cl::Program(ctx, sources);

  cl_int err = program_.build({device}, build_options_.c_str());
  if (err != CL_SUCCESS) {
    build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    GGEMS_ERROR("OpenCL", "Build log (Source) for '{}':\n{}", kernel_name_,
                build_log_);
    GGEMS_OCL_CHECK(
        err, std::format("Failed to build OpenCL program '{}'", kernel_name_));
  }

  build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
  if (!build_log_.empty())
    GGEMS_INFOEX("OpenCL", 2, "(Source) '{}':\n{}", kernel_name_, build_log_);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

void GGEMSOpenCLProgram::BuildFromSPIRV(std::vector<std::uint8_t> const &il) {
  auto raw_ctx = context_.GetRawContext();
  auto &device = context_.GetDevice().GetNative();

  cl_int err = CL_SUCCESS;
  cl_program prog_il =
      clCreateProgramWithIL(raw_ctx, il.data(), il.size(), &err);
  GGEMS_OCL_CHECK(err, std::format("Failed to create program with IL for '{}'.",
                                   kernel_name_));

  program_ = cl::Program(prog_il, true);

  err = program_.build({device}, build_options_.c_str());
  if (err != CL_SUCCESS) {
    build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    GGEMS_ERROR("OpenCL", "Build log (SPIR-V) for '{}':\n{}", kernel_name_,
                build_log_);
    GGEMS_OCL_CHECK(
        err, std::format("Failed to build OpenCL program '{}' from SPIR-V.",
                         kernel_name_));
  }

  build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
  if (!build_log_.empty()) {
    GGEMS_INFOEX("OpenCL", 2, "(SPIR-V) '{}':\n{}", kernel_name_, build_log_);
  }
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

cl::Kernel GGEMSOpenCLProgram::CreateKernel(std::string const &kernel_name) {
  GGEMS_INFOEX("OpenCL", 2, "Creating kernel '{}'", kernel_name);

  cl_int err = 0;
  cl::Kernel kernel(program_, kernel_name.c_str(), &err);
  GGEMS_OCL_CHECK(err,
                  std::format("Failed to create kernel '{}'", kernel_name));

  return kernel;
}
} // namespace ggems::ocl
