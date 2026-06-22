#pragma once
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
 * \file GGEMSOpenCLProgram.hh
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

/// \cond
#include <filesystem>
#include <vector>
#include <unordered_set>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {
class GGEMSOpenCL; /*!< Forward declaration of the program manager class. */

/*!
 * \class GGEMSOpenCLProgram
 * \brief Represents a compiled OpenCL program and its cached state.
 *
 * An instance stores:
 *  - the kernel source path,
 *  - the kernel name,
 *  - the compilation build options,
 *  - the compiled cl::Program object,
 *  - any cached binaries relevant to the device.
 *
 * Programs are instantiated exclusively by GGEMSOpenCL to maintain a central
 * program cache, avoiding repeated compilations and promoting deterministic
 * execution flow.
 */
class GGEMSOpenCLProgram {
  friend class GGEMSOpenCL; /*!< Factory access for private constructor */

public:
  /*!
   * \brief Destructor.
   *
   * Automatically releases OpenCL program resources through cl.hpp RAII.
   */
  ~GGEMSOpenCLProgram() = default;

  GGEMSOpenCLProgram(GGEMSOpenCLProgram const &) = delete;
  GGEMSOpenCLProgram &operator=(GGEMSOpenCLProgram const &) = delete;
  GGEMSOpenCLProgram(GGEMSOpenCLProgram &&) noexcept = delete;
  GGEMSOpenCLProgram &operator=(GGEMSOpenCLProgram &&) noexcept = delete;

private:
  /*!
   * \brief Private constructor invoked only by GGEMSOpenCL.
   *
   * Loads the kernel source, attempts to retrieve a cached binary,
   * compiles the program when needed, and stores the build log.
   *
   * \param ctx OpenCL context used to build and manage the program.
   * \param kernel_root Directory containing the kernel source.
   * \param kernel_name Name of the kernel without extension.
   * \param build_options OpenCL build options for the program.
   */
  GGEMSOpenCLProgram(GGEMSOpenCLContext &ctx, std::filesystem::path kernel_root,
                     std::string kernel_name, std::string build_options = {});

public:
  /*!
   * \brief Create a cl::Kernel object.
   *
   * \param kernel_name Name of the kernel entry point in the program.
   * \return cl::Kernel instance ready for argument setup and execution.
   */
  cl::Kernel CreateKernel(std::string const &kernel_name);

  /*!
   * \brief Access the underlying cl::Program.
   *
   * \return const cl::Program reference.
   */
  cl::Program const &GetProgramNative() const noexcept { return program_; }

  /*!
   * \brief Return the kernel name (base name without extension).
   * \return String view of the kernel name.
   */
  [[nodiscard]] std::string_view GetKernelName() const noexcept {
    return kernel_name_;
  }

  /*!
   * \brief Return the absolute path to the kernel source file.
   * \return String view path used during compilation.
   */
  [[nodiscard]] std::string_view GetSourcePath() const noexcept {
    return source_path_;
  }

  /*!
   * \brief Return all user-specified build options.
   * \return Build option string.
   */
  [[nodiscard]] std::string_view GetBuildOptions() const noexcept {
    return build_options_;
  }

  /*!
   * \brief Number of devices associated with the program.
   * \return Number of devices.
   */
  [[nodiscard]] cl_uint GetNumDevices() const;

  /*!
   * \brief Retrieve compiled binary sizes for each device.
   * \return Vector of byte sizes.
   */
  [[nodiscard]] std::vector<std::size_t> GetBinarySizes() const;

  /*!
   * \brief Retrieve program binaries for each device.
   * \return Vector-of-vectors containing raw binary data.
   */
  [[nodiscard]] auto GetBinaries() const;

  [[nodiscard]] bool Matches(GGEMSOpenCLContext const &context,
                             std::filesystem::path const &kernel_root,
                             std::string_view kernel_name,
                             std::string_view user_build_options) const;

private:
  /*!
   * \brief Load a kernel source file into memory.
   * \param path Filesystem path to the text file.
   * \return File contents as a string.
   */
  [[nodiscard]]
  static std::string LoadTextFile(std::filesystem::path const &path);

  /*!
   * \brief Build the full list of OpenCL compiler options.
   *
   * Combines internal GGEMS-required options with user-specified ones.
   *
   * \return Vector of individual option tokens.
   */
  [[nodiscard]] std::vector<std::string> BuildOptions() const;

  /*!
   * \brief Merge base compiler options with an additional option string.
   *
   * \param base  Pre-tokenised option list.
   * \param extra Extra string passed by the user.
   * \return Merged option string.
   */
  [[nodiscard]] std::string MergeOptions(std::vector<std::string> const &base,
                                         std::string const &extra) const;

  /*!
   * \brief Initialise the program: load source, detect cache, build.
   *
   * This is executed once by the constructor wrapper in GGEMSOpenCL.
   */
  void Initialise();

  /*!
   * \brief Build either from cached binary or from source.
   *
   * The method decides the appropriate build path based on the presence
   * and validity of a cached file.
   */
  void Build();

  /*!
   * \brief Build the program directly from a text source.
   *
   * \param src Source code as a string.
   */
  void BuildFromSource(std::string const &src);

  /*!
   * \brief Build the program from a cached binary blob.
   *
   * \param binary Raw binary data extracted from a previous build.
   */
  void BuildFromBinary(std::vector<std::uint8_t> const &binary);

  /*!
   * \brief Compute the filesystem path to the cache entry.
   *
   * Path is based on kernel name, device information, and source hash.
   *
   * \return Cache file path.
   */
  std::filesystem::path ComputeCachePath() const;

  /*!
   * \brief Save the compiled binary to the computed cache path.
   */
  void SaveBinaryToCache();

  /*!
   * \brief Load binary from the computed cache path.
   *
   * \return Vector containing the binary data. May be empty if no cache exists.
   */
  std::vector<std::uint8_t> LoadBinaryFromCache();

  [[nodiscard]] std::vector<std::filesystem::path>
  BuildIncludeSearchRoots() const;

  [[nodiscard]] std::string
  BuildSourceFingerprintText(std::filesystem::path const &source_path) const;

  void AppendSourceFingerprintText(
      std::filesystem::path const &source_path,
      std::vector<std::filesystem::path> const &include_roots,
      std::unordered_set<std::string> &visited_sources,
      std::string &fingerprint_text) const;

private:
  GGEMSOpenCLContext &context_;       /*!< Context used for Build operations. */
  std::filesystem::path kernel_root_; /*!< Directory containing the kernel. */
  std::string kernel_name_;           /*!< Base kernel name. */
  std::string source_path_;           /*!< Source file used for compile. */
  std::string user_build_options_;
  std::string build_options_;     /*!< User-specified build options. */
  std::string build_log_;         /*!< Build log text from the OpenCL driver. */
  cl::Program program_;           /*!< Compiled OpenCL program. */
  bool loaded_from_cache_{false}; /*!< True if built from cached binary. */
  std::uint64_t source_hash_;     /*!< Hash of source contents. */
  std::uint64_t global_hash_;     /*!< Combined hash for cache invalidation. */
};
} // namespace ggems::ocl
