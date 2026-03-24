// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSOpenCLProgram.cc
 * \brief Declaration of GGEMSOpenCLProgram, wrapping OpenCL program creation,
 *        binary caching, and rebuild logic.
 *
 * This class manages the lifecycle of a compiled OpenCL program:
 * - loading source code,
 * - compiling with given build options,
 * - generating and reading cached binaries,
 * - producing human-readable build logs,
 * - exposing the final cl::Program to GGEMS.
 *
 * GGEMSOpenCLProgram instances cannot be created directly; only
 * GGEMSOpenCL::GetOrCreateProgram() is authorised to construct them through
 * internal caching. This ensures program reuse and prevents uncontrolled
 * recompilation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"
#include "GGEMS/core/GGEMSCoreUtils.hh"

/// \cond
#include <fstream>
/// \endcond

namespace ggems::ocl {

namespace {
/*!
 * \brief Replace unsafe filename characters by underscores.
 *
 * Internal helper used to normalise vendor name, device name, and kernel names
 * prior to constructing cache filenames. Characters such as whitespace,
 * slashes, Windows separators and delimiters are converted to '\_' to ensure
 * the generated path is portable and valid across platforms.
 *
 * \param s Input string to sanitise.
 * \return Sanitised string safe to use in filesystem paths.
 */
std::string Sanitise(std::string s) {
  for (char &c : s) {
    if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == ';' || c == '\t')
      c = '_';
  }
  return s;
}

/*!
 * \brief Determine the root directory used for storing OpenCL binary caches.
 *
 * Resolves the platform-specific base directory for caching compiled OpenCL
 * program binaries:
 *  - On Windows: uses LOCALAPPDATA or APPDATA.
 *  - On POSIX: uses the HOME directory (creating ~/.ggems/opencl_cache).
 *
 * If no environment variable is available, falls back to a local directory
 * ("ggems_opencl_cache") relative to the working directory.
 *
 * \return Filesystem path to the root cache directory.
 */
std::filesystem::path GetCacheRootDirectory() {
#ifdef _WIN32
  if (char const *local = std::getenv("LOCALAPPDATA")) {
    return std::filesystem::path(local) / "GGEMS" / "opencl_cache";
  }
  if (char const *app = std::getenv("APPDATA")) {
    return std::filesystem::path(app) / "GGEMS" / "opencl_cache";
  }
#else
  if (char const *home = std::getenv("HOME")) {
    return std::filesystem::path(home) / ".ggems" / "opencl_cache";
  }
#endif
  return std::filesystem::path("ggems_opencl_cache");
}
} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSOpenCLProgram::GGEMSOpenCLProgram(GGEMSOpenCLContext &ctx,
                                       std::filesystem::path kernel_root,
                                       std::string kernel_name,
                                       std::string build_options)
    : context_{ctx}, kernel_root_{std::move(kernel_root)},
      kernel_name_{std::move(kernel_name)}, build_options_{""},
      source_hash_{0LL}, global_hash_{0LL} {
  GGEMS_INFO("OpenCL", "Initialising program '{}' (path: '{}')", kernel_name_,
             kernel_root_.string());

  std::string extra_options = build_options;
  auto default_options = BuildOptions();
  build_options_ = MergeOptions(default_options, extra_options);

  Initialise();
  Build();

  GGEMS_INFO("OpenCL", "Program '{}' built.", kernel_name_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::string> GGEMSOpenCLProgram::BuildOptions() const {
  std::vector<std::string> opts;

#ifdef GGEMS_DEBUG_MODE
  opts.emplace_back("-g");
  opts.emplace_back("-cl-opt-disable");
#else
  opts.emplace_back("-cl-fast-relaxed-math");
  opts.emplace_back("-cl-mad-enable");
  opts.emplace_back("-cl-no-signed-zeros");
#endif

  return opts;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string
GGEMSOpenCLProgram::MergeOptions(std::vector<std::string> const &base,
                                 std::string const &extra) const {
  std::string merged;

  for (auto const &s : base)
    merged += s + " ";

  if (!extra.empty())
    merged += extra;

  return merged;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCLProgram::Initialise() {
  auto cl_path = kernel_root_ / (kernel_name_ + ".cl");
  source_path_ = cl_path.string();

  GGEMS_INFO("OpenCL", "Initialising program '{}' (path: '{}', source: '{}')",
             kernel_name_, kernel_root_.string(), source_path_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCLProgram::Build() {
  auto src = LoadTextFile(std::filesystem::path{source_path_});
  source_hash_ = core::HashFNV1a(source_path_);

  auto &dev = context_.GetDevice();
  std::string concat;
  concat.reserve(1024);

  concat += Sanitise(dev.GetVendor());
  concat += Sanitise(dev.GetName());
  concat += dev.GetDriverVersion();
  concat += kernel_name_;
  concat += src;
  concat += build_options_;

  global_hash_ = core::HashFNV1a(concat);

  auto binary = LoadBinaryFromCache();
  if (!binary.empty()) {
    try {
      BuildFromBinary(binary);
      loaded_from_cache_ = true;
      GGEMS_INFO("OpenCL", "Loaded program '{}' from cache.", kernel_name_);
      return;
    } catch (...) {
      GGEMS_WARN(
          "OpenCL",
          "Failed to use cached binary for '{}', falling back to source.",
          kernel_name_);
    }
  }

  GGEMS_INFO("OpenCL", "Building program '{}' from source '{}'.", kernel_name_,
             source_path_);

  BuildFromSource(src);
  try {
    SaveBinaryToCache();
  } catch (...) {
    GGEMS_WARN("OpenCL", "Failed to save binary cache for '{}'.", kernel_name_);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string
GGEMSOpenCLProgram::LoadTextFile(std::filesystem::path const &path) {
  std::ifstream ifs(path, std::ios::binary);
  GGEMS_CHECK_FATAL(
      ifs.good(),
      std::format("Failed to open program source file '{}'.", path.string()));

  std::ostringstream oss;
  oss << ifs.rdbuf();
  return oss.str();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCLProgram::BuildFromSource(std::string const &src) {
  auto &ctx = context_.GetContextNative();
  auto &device = context_.GetDevice().GetDeviceNative();

  cl::Program::Sources sources;
  sources.push_back({src.c_str(), src.size()});

  program_ = cl::Program(ctx, sources);

  cl_int err = program_.build({device}, build_options_.c_str());
  if (err != CL_SUCCESS) {
    build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    GGEMS_ERROR("OpenCL", "Build log for program '{}' (file='{}'): {}",
                kernel_name_, source_path_, build_log_);
    GGEMS_OCL_CHECK(
        err, std::format("Failed to build OpenCL program '{}' from source.",
                         kernel_name_));
  }

  build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
  if (!build_log_.empty())
    GGEMS_INFO("OpenCL", "Build log for '{}' (file='{}'): {}", kernel_name_,
               source_path_, build_log_);

  GGEMS_INFO("OpenCL", "Program '{}' built from source '{}' and options '{}'.",
             kernel_name_, source_path_, build_options_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCLProgram::BuildFromBinary(
    std::vector<std::uint8_t> const &binary) {
  auto ctx = context_.GetContextNative();
  auto &device = context_.GetDevice().GetDeviceNative();
  cl_device_id dev_id = device();

  cl_int binary_status{CL_SUCCESS};
  cl_int err{CL_SUCCESS};

  size_t length = binary.size();
  unsigned char const *ptr =
      reinterpret_cast<unsigned char const *>(binary.data());

  cl_program prog = clCreateProgramWithBinary(ctx(), 1, &dev_id, &length, &ptr,
                                              &binary_status, &err);

  GGEMS_OCL_CHECK(
      err, std::format(
               "Failed to create program with binary for '{}' (source='{}').",
               kernel_name_, source_path_));

  GGEMS_OCL_CHECK(
      binary_status,
      std::format("Binary status error for program '{}' (source='{}').",
                  kernel_name_, source_path_));

  program_ = cl::Program(prog, false);

  err = program_.build({device}, build_options_.c_str());
  if (err != CL_SUCCESS) {
    build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    GGEMS_ERROR("OpenCL", "Build log for '{}' (file='{}'): {}", kernel_name_,
                source_path_, build_log_);
    core::Throw<core::GGEMSFatal>(std::format(
        "Failed to build OpenCL program '{}' from binary.", kernel_name_));
  }

  build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
  if (!build_log_.empty()) {
    GGEMS_INFO("OpenCL", "Build log for '{}' (file='{}'): {}", kernel_name_,
               source_path_, build_log_);
  }

  GGEMS_INFO("OpenCL",
             "Program '{}' built from cached binary and options '{}'.",
             kernel_name_, build_options_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::filesystem::path GGEMSOpenCLProgram::ComputeCachePath() const {
  auto &dev = context_.GetDevice();
  std::string vendor = Sanitise(dev.GetVendor());
  std::string name = Sanitise(dev.GetName());

  std::string fname = std::format("{}__{}__{}__{:016x}.bin", vendor, name,
                                  kernel_name_, global_hash_);

  auto root = GetCacheRootDirectory();
  auto device_dir = root / (vendor + "__" + name);

  std::error_code ec;
  std::filesystem::create_directories(device_dir, ec);
  if (ec) {
    GGEMS_WARN("OpenCL", "Failed to create cache directory '{}': {}",
               device_dir.string(), ec.message());
  }

  return device_dir / fname;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

[[nodiscard]] cl_uint GGEMSOpenCLProgram::GetNumDevices() const {
  return GetInfo<CL_PROGRAM_NUM_DEVICES>(program_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

[[nodiscard]] std::vector<std::size_t>
GGEMSOpenCLProgram::GetBinarySizes() const {
  return GetInfo<CL_PROGRAM_BINARY_SIZES>(program_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

[[nodiscard]] auto GGEMSOpenCLProgram::GetBinaries() const {
  return GetInfo<CL_PROGRAM_BINARIES>(program_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCLProgram::SaveBinaryToCache() {
  auto num = GetNumDevices();
  auto sizes = GetBinarySizes();
  auto bins = GetBinaries();

  if (num == 0 || sizes.empty() || bins.empty() || sizes[0] == 0) {
    GGEMS_WARN("OpenCL", "No binary available for '{}'.", kernel_name_);
    return;
  }

  auto cache_path = ComputeCachePath();
  std::ofstream ofs(cache_path, std::ios::binary);

  if (!ofs.good()) {
    GGEMS_WARN("OpenCL", "Failed to open '{}' for writing.",
               cache_path.string());
    return;
  }

  ofs.write(reinterpret_cast<char const *>(bins[0].data()),
            static_cast<std::streamsize>(bins[0].size()));

  GGEMS_INFO("OpenCL", "Saved binary cache for '{}' → '{}'", kernel_name_,
             cache_path.string());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::uint8_t> GGEMSOpenCLProgram::LoadBinaryFromCache() {
  auto cache_path = ComputeCachePath();
  std::ifstream ifs(cache_path, std::ios::binary);
  if (!ifs.good()) {
    return {};
  }

  ifs.seekg(0, std::ios::end);
  auto size = ifs.tellg();
  if (size <= 0) {
    return {};
  }
  ifs.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
  if (!ifs.read(reinterpret_cast<char *>(data.data()), size)) {
    GGEMS_WARN("OpenCL", "Failed to read cache file '{}'.",
               cache_path.string());
    return {};
  }

  GGEMS_INFO("OpenCL", "Loaded binary cache for program '{}' from '{}'.",
             kernel_name_, cache_path.string());

  return data;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

cl::Kernel GGEMSOpenCLProgram::CreateKernel(std::string const &kernel_name) {
  GGEMS_INFOEX("OpenCL", 2, "Creating kernel '{}'", kernel_name);

  cl_int err{CL_SUCCESS};
  cl::Kernel kernel(program_, kernel_name.c_str(), &err);
  GGEMS_OCL_CHECK(err,
                  std::format("Failed to create kernel '{}'", kernel_name));

  return kernel;
}
} // namespace ggems::ocl
