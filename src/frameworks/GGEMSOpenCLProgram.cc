// ************************************************************************
// ************************************************************************


#include <fstream>
#include <sstream>
#include <optional>
#include <unordered_set>
#include <cctype>
#include <algorithm>
#include <string_view>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"
#include "GGEMS/frameworks/GGEMSOpenCLCacheFingerprint.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

namespace {
constexpr std::string_view k_opencl_cache_schema{"GGEMS_OPENCL_CACHE"};

std::string Sanitize(std::string s) {
  for (char &c : s) {
    if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == ';' || c == '\t')
      c = '_';
  }
  return s;
}

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

std::optional<std::string> ExtractQuotedInclude(std::string_view line) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::filesystem::path>
ExtractIncludeRoots(std::string_view build_options) {
  std::vector<std::filesystem::path> roots;

  for (std::size_t i = 0U; i < build_options.size(); ++i) {
    if (build_options[i] != '-' || i + 1U >= build_options.size() ||
        build_options[i + 1U] != 'I') {
      continue;
    }

    i += 2U;

    while (i < build_options.size() &&
           std::isspace(static_cast<unsigned char>(build_options[i])) != 0) {
      ++i;
    }

    std::string include_path;

    if (i < build_options.size() && build_options[i] == '"') {
      ++i;

      while (i < build_options.size() && build_options[i] != '"') {
        include_path.push_back(build_options[i]);
        ++i;
      }
    } else {
      while (i < build_options.size() &&
             std::isspace(static_cast<unsigned char>(build_options[i])) == 0) {
        include_path.push_back(build_options[i]);
        ++i;
      }
    }

    if (!include_path.empty()) {
      roots.emplace_back(include_path);
    }
  }

  return roots;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::filesystem::path NormalizePath(std::filesystem::path const &path) {
  std::error_code ec;
  std::filesystem::path const canonical =
      std::filesystem::weakly_canonical(path, ec);

  if (!ec) {
    return canonical;
  }

  return path.lexically_normal();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::optional<std::filesystem::path>
ResolveLocalInclude(std::string const &include_name,
                    std::filesystem::path const &including_dir,
                    std::vector<std::filesystem::path> const &include_roots) {
  std::vector<std::filesystem::path> candidates;
  candidates.reserve(include_roots.size() + 1U);

  candidates.emplace_back(including_dir / include_name);

  for (std::filesystem::path const &root : include_roots) {
    candidates.emplace_back(root / include_name);
  }

  for (std::filesystem::path const &candidate : candidates) {
    std::error_code ec;

    if (std::filesystem::exists(candidate, ec) && !ec) {
      return NormalizePath(candidate);
    }
  }

  return std::nullopt;
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
      kernel_name_{std::move(kernel_name)},
      user_build_options_{std::move(build_options)}, build_options_{""},
      source_hash_{0LL}, global_hash_{0LL} {
  GGEMS_INFOEX("OpenCL", 2, "Initializing OpenCL program '{}'.", kernel_name_);
  GGEMS_INFOEX("OpenCL", 3, "OpenCL program source root: '{}'.",
               kernel_root_.string());

  auto default_options = BuildOptions();
  build_options_ = MergeOptions(default_options, user_build_options_);

  Initialize();
  Build();

  GGEMS_INFOEX("OpenCL", 2, "OpenCL program '{}' built.", kernel_name_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::string> GGEMSOpenCLProgram::BuildOptions() const {
  std::vector<std::string> opts;

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

void GGEMSOpenCLProgram::Initialize() {
  auto cl_path = kernel_root_ / (kernel_name_ + ".cl");
  source_path_ = cl_path.string();

  GGEMS_INFO("OpenCL", "Initializing program '{}' (path: '{}', source: '{}')",
             kernel_name_, kernel_root_.string(), source_path_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::filesystem::path>
GGEMSOpenCLProgram::BuildIncludeSearchRoots() const {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSOpenCLProgram::BuildSourceFingerprintText(
    std::filesystem::path const &source_path) const {
  std::vector<std::filesystem::path> include_roots = BuildIncludeSearchRoots();

  std::unordered_set<std::string> visited_sources;
  std::string fingerprint_text;

  AppendSourceFingerprintText(NormalizePath(source_path), include_roots,
                              visited_sources, fingerprint_text);

  return fingerprint_text;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCLProgram::AppendSourceFingerprintText(
    std::filesystem::path const &source_path,
    std::vector<std::filesystem::path> const &include_roots,
    std::unordered_set<std::string> &visited_sources,
    std::string &fingerprint_text) const {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSOpenCLProgram::Matches(
    GGEMSOpenCLContext const &context, std::filesystem::path const &kernel_root,
    std::string_view const kernel_name,
    std::string_view const user_build_options) const {
  if (&context_ != &context) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOpenCLProgram::Build() {
  std::filesystem::path source_path{source_path_};

  auto src = LoadTextFile(source_path);

  std::string source_fingerprint_text = BuildSourceFingerprintText(source_path);

  source_hash_ = detail::HashFNV1a64(source_fingerprint_text);

  auto &dev = context_.GetDevice();

  std::string concat;
  concat.reserve(1024);

  concat += k_opencl_cache_schema;
  concat += "\nVendor=" + Sanitize(dev.GetVendor());
  concat += "\nDevice=" + Sanitize(dev.GetName());
  concat += "\nDeviceVersion=" + dev.GetVersion();
  concat += "\nOpenCLCVersion=" + dev.GetOpenCLCVersion();
  concat += "\nDriverVersion=" + dev.GetDriverVersion();
  concat += "\nKernelName=" + kernel_name_;
  concat += "\nBuildOptions=" + build_options_;
  concat += "\nSourceHash=" + std::format("{:016x}", source_hash_);

  global_hash_ = detail::HashFNV1a64(concat);

  auto binary = LoadBinaryFromCache();
  if (!binary.empty()) {
    try {
      BuildFromBinary(binary);
      loaded_from_cache_ = true;
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
  if (!(ifs.good())) {
    throw ggems::core::GGEMSFatal(
        std::format("Failed to open program source file '{}'.", path.string()));
  }

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
    CheckCLError(
        err,
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

  CheckCLError(
      err, std::format(
               "Failed to create program with binary for '{}' (source='{}').",
               kernel_name_, source_path_));

  CheckCLError(binary_status,
               std::format(
                   "Binary status error for program '{}' (source='{}').",
                   kernel_name_, source_path_));

  program_ = cl::Program(prog, false);

  err = program_.build({device}, build_options_.c_str());
  if (err != CL_SUCCESS) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::filesystem::path GGEMSOpenCLProgram::ComputeCachePath() const {
  auto &dev = context_.GetDevice();
  std::string vendor = Sanitize(dev.GetVendor());
  std::string name = Sanitize(dev.GetName());

  std::string fname = std::format("{}__{}__{}__{:016x}.bin", vendor, name,
                                  kernel_name_, global_hash_);

  auto root = GetCacheRootDirectory();
  auto device_dir = root / (vendor + "__" + name);

  std::error_code ec;
  std::filesystem::create_directories(device_dir, ec);
  if (ec) {
    GGEMS_INFOEX("OpenCL", 2,
                 "Could not create OpenCL binary cache directory.");

    GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache directory: '{}' ({})",
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
    GGEMS_INFOEX("OpenCL", 3,
                 "No OpenCL binary available for '{}'; cache not written.",
                 kernel_name_);
    return;
  }

  auto cache_path = ComputeCachePath();
  std::ofstream ofs(cache_path, std::ios::binary);

  if (!ofs.good()) {
    GGEMS_INFOEX("OpenCL", 2, "Could not write OpenCL binary cache for '{}'.",
                 kernel_name_);

    GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache path: '{}'.",
                 cache_path.string());

    return;
  }

  ofs.write(reinterpret_cast<char const *>(bins[0].data()),
            static_cast<std::streamsize>(bins[0].size()));

  GGEMS_INFOEX("OpenCL", 2, "OpenCL binary cache saved for '{}'.",
               kernel_name_);

  GGEMS_INFOEX("OpenCL", 3, "OpenCL binary cache path: '{}'.",
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
  auto end_pos = ifs.tellg();

  if (end_pos <= std::streampos{0}) {
    return {};
  }

  auto byte_count = static_cast<std::streamoff>(end_pos);
  ifs.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> data(static_cast<std::size_t>(byte_count));

  if (!ifs.read(reinterpret_cast<char *>(data.data()),
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

cl::Kernel GGEMSOpenCLProgram::CreateKernel(std::string const &kernel_name) {
  GGEMS_INFOEX("OpenCL", 2, "Creating kernel '{}'", kernel_name);

  cl_int err{CL_SUCCESS};
  cl::Kernel kernel(program_, kernel_name.c_str(), &err);
  CheckCLError(err,
               std::format("Failed to create kernel '{}'", kernel_name));

  return kernel;
}
} // namespace ggems::ocl
