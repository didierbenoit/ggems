// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements OpenCL program building, fingerprinting, and caching.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <string>
#include <filesystem>
#include <vector>
#include <system_error>
#include <ios>
#include <cstdint>
#include <iosfwd>
#include <cstddef>
/// \endcond

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLCacheFingerprint.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"

namespace ggems::ocl {

namespace {

#if defined(__APPLE__)
constexpr std::string_view k_opencl_standard_option{"-cl-std=CL1.2"};
#else
constexpr std::string_view k_opencl_standard_option{"-cl-std=CL2.0"};
#endif

// =============================================================================
// =============================================================================

/*!
 * \brief Schema identifier stored in GGEMS OpenCL cache entries.
 */
constexpr std::string_view k_opencl_cache_schema{"GGEMS_OPENCL_CACHE"};

// =============================================================================
// =============================================================================

/*!
 * \brief Replaces path-unsafe separator characters in a cache-name component.
 *
 * \param[in] value Text to sanitize.
 * \return Sanitized cache-name component.
 */
auto Sanitize(std::string value) -> std::string {
  for (char &character : value) {
    if (character == ' ' || character == '/' || character == '\\' ||
        character == ':' || character == ';' || character == '\t') {
      character = '_';
    }
  }
  return value;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Returns the platform-appropriate GGEMS OpenCL cache directory.
 *
 * \return OpenCL cache root directory.
 */
auto GetCacheRootDirectory() -> std::filesystem::path {
#ifdef _WIN32
  if (char const *local = std::getenv("LOCALAPPDATA")) {
    return std::filesystem::path(local) / "GGEMS" / "opencl_cache";
  }
  if (char const *application_data = std::getenv("APPDATA")) {
    return std::filesystem::path(application_data) / "GGEMS" / "opencl_cache";
  }
#else
  if (char const *home = std::getenv("HOME")) {
    return std::filesystem::path(home) / ".ggems" / "opencl_cache";
  }
#endif
  return {"ggems_opencl_cache"};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Extracts a quoted local include from one source line.
 *
 * \param[in] line Source line to inspect.
 * \return Included relative path when a quoted include is found, otherwise an
 * empty optional.
 */
auto ExtractQuotedInclude(std::string_view line) -> std::optional<std::string> {
  auto include_pos = line.find("#include");

  if (include_pos == std::string_view::npos) {
    return std::nullopt;
  }

  auto first_quote = line.find('"', include_pos);

  if (first_quote == std::string_view::npos) {
    return std::nullopt;
  }

  auto second_quote = line.find('"', first_quote + 1U);

  if (second_quote == std::string_view::npos) {
    return std::nullopt;
  }

  return std::string{
      line.substr(first_quote + 1U, second_quote - first_quote - 1U)};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Extracts -I include roots from an OpenCL build-option string.
 *
 * \param[in] build_options OpenCL build options.
 * \return Include-root paths in option order.
 */
auto ExtractIncludeRoots(std::string_view build_options)
    -> std::vector<std::filesystem::path> {
  std::vector<std::filesystem::path> roots;

  for (std::size_t index = 0U; index < build_options.size(); ++index) {
    if (build_options[index] != '-' || index + 1U >= build_options.size() ||
        build_options[index + 1U] != 'I') {
      continue;
    }

    index += 2U;

    while (index < build_options.size() &&
           std::isspace(static_cast<unsigned char>(build_options[index])) !=
               0) {
      ++index;
    }

    std::string include_path;

    if (index < build_options.size() && build_options[index] == '"') {
      ++index;

      while (index < build_options.size() && build_options[index] != '"') {
        include_path.push_back(build_options[index]);
        ++index;
      }
    } else {
      while (index < build_options.size() &&
             std::isspace(static_cast<unsigned char>(build_options[index])) ==
                 0) {
        include_path.push_back(build_options[index]);
        ++index;
      }
    }

    if (!include_path.empty()) {
      roots.emplace_back(include_path);
    }
  }

  return roots;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Normalizes a filesystem path, preferring weak canonicalization when
 * available.
 *
 * \param[in] path Path to normalize.
 * \return Normalized filesystem path.
 */
auto NormalizePath(std::filesystem::path const &path) -> std::filesystem::path {
  std::error_code error_code;
  auto canonical = std::filesystem::weakly_canonical(path, error_code);

  if (!error_code) {
    return canonical;
  }

  return path.lexically_normal();
}

// =============================================================================
// =============================================================================

/*!
 * \brief Resolves a quoted include against the including directory and
 * configured roots.
 *
 * \param[in] include_name Quoted include path.
 * \param[in] including_dir Directory of the including source file.
 * \param[in] include_roots Configured include-search roots.
 * \return Resolved normalized source path, or an empty optional if no candidate
 * exists.
 */
auto ResolveLocalInclude(
    std::string const &include_name, std::filesystem::path const &including_dir,
    std::vector<std::filesystem::path> const &include_roots)
    -> std::optional<std::filesystem::path> {
  std::vector<std::filesystem::path> candidates;
  candidates.reserve(include_roots.size() + 1U);

  candidates.emplace_back(including_dir / include_name);

  for (std::filesystem::path const &root : include_roots) {
    candidates.emplace_back(root / include_name);
  }

  for (std::filesystem::path const &candidate : candidates) {
    std::error_code error_code;

    if (std::filesystem::exists(candidate, error_code) && !error_code) {
      return NormalizePath(candidate);
    }
  }

  return std::nullopt;
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSOpenCLProgram::GGEMSOpenCLProgram(GGEMSOpenCLContext const &context,
                                       std::filesystem::path kernel_root,
                                       std::string kernel_name,
                                       std::string build_options)
    : context_{context.GetContextNative()},
      device_{context.GetDevice().GetDeviceNative()},
      kernel_root_{std::move(kernel_root)},
      kernel_name_{std::move(kernel_name)},
      user_build_options_{std::move(build_options)}, source_hash_{0LL},
      global_hash_{0LL} {
  GGEMS_INFOEX("OpenCL", 2, "Initializing OpenCL program '{}'.", kernel_name_);
  GGEMS_INFOEX("OpenCL", 3, "OpenCL program source root: '{}'.",
               kernel_root_.string());

  auto default_options = BuildOptions();
  build_options_ = MergeOptions(default_options, user_build_options_);

  Initialize();
  Build();

  GGEMS_INFOEX("OpenCL", 2, "OpenCL program '{}' built.", kernel_name_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::BuildOptions() const -> std::vector<std::string> {
  std::vector<std::string> opts;

  opts.emplace_back(k_opencl_standard_option);
  opts.emplace_back("-I\"" + kernel_root_.generic_string() + "\"");

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

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::MergeOptions(std::vector<std::string> const &base,
                                      std::string const &extra) -> std::string {
  std::string merged;

  for (auto const &option : base) {
    merged += option + " ";
  }

  if (!extra.empty()) {
    merged += extra;
  }

  return merged;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::Initialize() -> void {
  auto cl_path = kernel_root_ / (kernel_name_ + ".cl");
  source_path_ = cl_path.string();

  GGEMS_INFO("OpenCL", "Initializing program '{}' (path: '{}', source: '{}')",
             kernel_name_, kernel_root_.string(), source_path_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::BuildIncludeSearchRoots() const
    -> std::vector<std::filesystem::path> {
  std::vector<std::filesystem::path> roots;

  std::filesystem::path source_dir =
      std::filesystem::path{source_path_}.parent_path();

  roots.emplace_back(source_dir);
  roots.emplace_back(kernel_root_);

  if (kernel_root_.has_parent_path()) {
    roots.emplace_back(kernel_root_.parent_path());
  }

  std::vector<std::filesystem::path> option_roots =
      ExtractIncludeRoots(build_options_);

  roots.insert(roots.end(), option_roots.begin(), option_roots.end());

  for (std::filesystem::path &root : roots) {
    root = NormalizePath(root);
  }

  std::ranges::sort(roots);
  roots.erase(std::ranges::unique(roots).begin(), roots.end());

  return roots;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::BuildSourceFingerprintText(
    std::filesystem::path const &source_path) const -> std::string {
  std::vector<std::filesystem::path> include_roots = BuildIncludeSearchRoots();

  std::unordered_set<std::string> visited_sources;
  std::string fingerprint_text;

  AppendSourceFingerprintText(NormalizePath(source_path), include_roots,
                              visited_sources, fingerprint_text);

  return fingerprint_text;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::AppendSourceFingerprintText(
    std::filesystem::path const &source_path,
    std::vector<std::filesystem::path> const &include_roots,
    std::unordered_set<std::string> &visited_sources,
    std::string &fingerprint_text) const -> void {
  std::filesystem::path normalized_source = NormalizePath(source_path);
  std::string source_key = normalized_source.generic_string();

  if (visited_sources.contains(source_key)) {
    return;
  }

  visited_sources.insert(source_key);

  std::string source = LoadTextFile(normalized_source);

  fingerprint_text += "\n/* BEGIN GGEMS SOURCE: ";
  fingerprint_text += source_key;
  fingerprint_text += " */\n";
  fingerprint_text += source;
  fingerprint_text += "\n/* END GGEMS SOURCE: ";
  fingerprint_text += source_key;
  fingerprint_text += " */\n";

  std::istringstream stream{source};
  std::string line;

  while (std::getline(stream, line)) {
    std::optional<std::string> include_name = ExtractQuotedInclude(line);

    if (!include_name.has_value()) {
      continue;
    }

    std::optional<std::filesystem::path> include_path = ResolveLocalInclude(
        *include_name, normalized_source.parent_path(), include_roots);

    if (!include_path.has_value()) {
      fingerprint_text += "\n/* UNRESOLVED GGEMS INCLUDE: ";
      fingerprint_text += *include_name;
      fingerprint_text += " */\n";
      continue;
    }

    AppendSourceFingerprintText(*include_path, include_roots, visited_sources,
                                fingerprint_text);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::Matches(GGEMSOpenCLContext const &context,
                                 std::filesystem::path const &kernel_root,
                                 std::string_view kernel_name,
                                 std::string_view user_build_options) const
    -> bool {
  if (context_() != context.GetContextNative()()) {
    return false;
  }

  if (device_() != context.GetDevice().GetDeviceNative()()) {
    return false;
  }

  if (kernel_name_ != kernel_name) {
    return false;
  }

  if (user_build_options_ != user_build_options) {
    return false;
  }

  std::filesystem::path expected_source_path =
      NormalizePath(kernel_root / (std::string{kernel_name} + ".cl"));

  std::filesystem::path current_source_path =
      NormalizePath(std::filesystem::path{source_path_});

  return current_source_path == expected_source_path;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::Build() -> void {
  std::filesystem::path source_path{source_path_};

  auto source = LoadTextFile(source_path);

  std::string source_fingerprint_text = BuildSourceFingerprintText(source_path);

  source_hash_ = detail::HashFNV1a64(source_fingerprint_text);

  std::string concat;
  concat.reserve(1024);

  concat += k_opencl_cache_schema;
  concat += "\nVendor=" + Sanitize(GetInfo<CL_DEVICE_VENDOR>(device_));
  concat += "\nDevice=" + Sanitize(GetInfo<CL_DEVICE_NAME>(device_));
  concat += "\nDeviceVersion=" + GetInfo<CL_DEVICE_VERSION>(device_);
  concat += "\nOpenCLCVersion=" + GetInfo<CL_DEVICE_OPENCL_C_VERSION>(device_);
  concat += "\nDriverVersion=" + GetInfo<CL_DRIVER_VERSION>(device_);
  concat += "\nKernelName=" + kernel_name_;
  concat += "\nBuildOptions=" + build_options_;
  concat += "\nSourceHash=" + std::format("{:016x}", source_hash_);

  global_hash_ = detail::HashFNV1a64(concat);

  auto binary = LoadBinaryFromCache();
  if (!binary.empty()) {
    try {
      BuildFromBinary(binary);
      GGEMS_INFOEX("OpenCL", 2, "OpenCL program '{}' from cache.",
                   kernel_name_);
      return;
    } catch (...) {
      GGEMS_INFOEX(
          "OpenCL", 2,
          "Cached binary for '{}' could not be used; falling back to source.",
          kernel_name_);
    }
  }

  GGEMS_INFOEX("OpenCL", 2, "Building OpenCL program '{}' from source.",
               kernel_name_);

  GGEMS_INFOEX("OpenCL", 3, "OpenCL program '{}' source path: '{}'.",
               kernel_name_, source_path_);

  GGEMS_INFOEX("OpenCL", 3,
               "OpenCL program '{}' source dependency hash: {:016x}.",
               kernel_name_, source_hash_);

  BuildFromSource(source);

  try {
    SaveBinaryToCache();
  } catch (...) {
    GGEMS_WARN("OpenCL", "Failed to save binary cache for '{}'.", kernel_name_);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::LoadTextFile(std::filesystem::path const &path)
    -> std::string {
  std::ifstream input_stream(path, std::ios::binary);
  if (!input_stream.good()) {
    throw ggems::core::GGEMSFatal(
        std::format("Failed to open program source file '{}'.", path.string()));
  }

  std::ostringstream output_stream;
  output_stream << input_stream.rdbuf();
  return output_stream.str();
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::BuildFromSource(std::string const &source) -> void {
  auto const &context = context_;
  auto const &device = device_;

  cl::Program::Sources sources;
  sources.emplace_back(source.c_str(), source.size());

  program_ = cl::Program(context, sources);

  cl_int error = program_.build({device}, build_options_.c_str());
  if (error != CL_SUCCESS) {
    build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    GGEMS_ERROR("OpenCL", "Build log for program '{}' (file='{}'): {}",
                kernel_name_, source_path_, build_log_);
    CheckCLError(
        error,
        std::format(
            "Failed to build OpenCL program '{}' from source. Build log : {}",
            kernel_name_, build_log_));
  }

  build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
  if (!build_log_.empty()) {
    GGEMS_INFOEX("OpenCL", 3, "Build log for '{}' (file='{}'): {}",
                 kernel_name_, source_path_, build_log_);
  }
  GGEMS_INFOEX("OpenCL", 2, "OpenCL program '{}' built from source.",
               kernel_name_);

  GGEMS_INFOEX("OpenCL", 3, "OpenCL program '{}' source='{}', options='{}'.",
               kernel_name_, source_path_, build_options_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::BuildFromBinary(
    std::vector<std::uint8_t> const &binary) -> void {
  auto const &context = context_;
  auto const &device = device_;
  cl_device_id device_id = device();

  cl_int binary_status{CL_SUCCESS};
  cl_int error{CL_SUCCESS};

  std::size_t length = binary.size();
  auto const *binary_pointer =
      reinterpret_cast<unsigned char const *>(binary.data());

  cl_program native_program =
      clCreateProgramWithBinary(context(), 1, &device_id, &length,
                                &binary_pointer, &binary_status, &error);

  CheckCLError(
      error, std::format(
                 "Failed to create program with binary for '{}' (source='{}').",
                 kernel_name_, source_path_));

  CheckCLError(
      binary_status,
      std::format("Binary status error for program '{}' (source='{}').",
                  kernel_name_, source_path_));

  program_ = cl::Program(native_program, false);

  error = program_.build({device}, build_options_.c_str());
  if (error != CL_SUCCESS) {
    build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

    if (!build_log_.empty()) {
      GGEMS_INFOEX("OpenCL", 3,
                   "Cached binary build log for '{}' (file='{}'): {}",
                   kernel_name_, source_path_, build_log_);
    }

    throw core::GGEMSFatal(std::format(
        "Failed to build OpenCL program '{}' from binary.", kernel_name_));
  }

  build_log_ = program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
  if (!build_log_.empty()) {
    GGEMS_INFOEX("OpenCL", 3, "Build log for '{}' (file='{}'): {}",
                 kernel_name_, source_path_, build_log_);
  }
  GGEMS_INFOEX("OpenCL", 2, "OpenCL program '{}' built from cached binary.",
               kernel_name_);

  GGEMS_INFOEX("OpenCL", 3, "OpenCL program '{}' binary options='{}'.",
               kernel_name_, build_options_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::ComputeCachePath() const -> std::filesystem::path {
  std::string vendor = Sanitize(GetInfo<CL_DEVICE_VENDOR>(device_));
  std::string name = Sanitize(GetInfo<CL_DEVICE_NAME>(device_));

  std::string filename = std::format("{}__{}__{}__{:016x}.bin", vendor, name,
                                     kernel_name_, global_hash_);

  auto root = GetCacheRootDirectory();
  auto device_dir = root / (vendor + "__" + name);

  std::error_code error_code;
  std::filesystem::create_directories(device_dir, error_code);
  if (error_code) {
    GGEMS_INFOEX("OpenCL", 2,
                 "Could not create OpenCL binary cache directory.");

    GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache directory: '{}' ({})",
                 device_dir.string(), error_code.message());
  }

  return device_dir / filename;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLProgram::GetNumDevices() const -> cl_uint {
  return GetInfo<CL_PROGRAM_NUM_DEVICES>(program_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLProgram::GetBinarySizes() const
    -> std::vector<std::size_t> {
  return GetInfo<CL_PROGRAM_BINARY_SIZES>(program_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLProgram::GetBinaries() const
    -> std::vector<std::vector<unsigned char>> {
  return GetInfo<CL_PROGRAM_BINARIES>(program_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::SaveBinaryToCache() -> void {
  auto const device_count = GetNumDevices();
  auto const binary_sizes = GetBinarySizes();
  auto const binaries = GetBinaries();

  if (device_count == 0 || binary_sizes.empty() || binaries.empty() ||
      binary_sizes[0] == 0) {
    GGEMS_INFOEX("OpenCL", 3,
                 "No OpenCL binary available for '{}'; cache not written.",
                 kernel_name_);
    return;
  }

  auto cache_path = ComputeCachePath();
  std::ofstream output_stream(cache_path, std::ios::binary);

  if (!output_stream.good()) {
    GGEMS_INFOEX("OpenCL", 2, "Could not write OpenCL binary cache for '{}'.",
                 kernel_name_);

    GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache path: '{}'.",
                 cache_path.string());

    return;
  }

  output_stream.write(reinterpret_cast<char const *>(binaries[0].data()),
                      static_cast<std::streamsize>(binaries[0].size()));

  GGEMS_INFOEX("OpenCL", 2, "OpenCL binary cache saved for '{}'.",
               kernel_name_);

  GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache path: '{}'.",
               cache_path.string());
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::LoadBinaryFromCache() -> std::vector<std::uint8_t> {
  auto cache_path = ComputeCachePath();
  std::ifstream input_stream(cache_path, std::ios::binary);
  if (!input_stream.good()) {
    return {};
  }

  input_stream.seekg(0, std::ios::end);
  auto end_pos = input_stream.tellg();

  if (end_pos <= std::streampos{0}) {
    return {};
  }

  auto byte_count = static_cast<std::streamoff>(end_pos);
  input_stream.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> data(static_cast<std::size_t>(byte_count));

  if (!input_stream.read(reinterpret_cast<char *>(data.data()),
                         static_cast<std::streamsize>(byte_count))) {
    GGEMS_INFOEX("OpenCL", 2, "Could not read OpenCL binary cache for '{}'.",
                 kernel_name_);

    GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache path: '{}'.",
                 cache_path.string());
    return {};
  }

  GGEMS_INFOEX("OpenCL", 2, "OpenCL binary cache loaded for '{}'.",
               kernel_name_);

  GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache path: '{}'.",
               cache_path.string());

  return data;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProgram::CreateKernel(std::string const &kernel_name) const
    -> cl::Kernel {
  GGEMS_INFOEX("OpenCL", 2, "Creating kernel '{}'", kernel_name);

  cl_int error{CL_SUCCESS};
  cl::Kernel kernel(program_, kernel_name.c_str(), &error);
  CheckCLError(error, std::format("Failed to create kernel '{}'", kernel_name));

  return kernel;
}
} // namespace ggems::ocl
