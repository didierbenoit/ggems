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
 * \brief Declares the GGEMS OpenCL program build and cache wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {

/*!
 * \brief Builds, owns, fingerprints, and caches an OpenCL program for one device.
 *
 * The wrapper retains native OpenCL context and device handles so the program remains independent of the lifetime of the GGEMS context wrapper used at construction.
 */
class GGEMSOpenCLProgram {
public:
  /*!
   * \brief Constructs and builds an OpenCL program.
   *
   * \param[in] context OpenCL context used for compilation.
   * \param[in] kernel_root Root directory containing kernel sources.
   * \param[in] kernel_name Kernel source name.
   * \param[in] build_options Additional user build options.
   */
  GGEMSOpenCLProgram(GGEMSOpenCLContext const &context,
                     std::filesystem::path kernel_root, std::string kernel_name,
                     std::string build_options = {});

  /*!
   * \brief Destroys the OpenCL program wrapper.
   */
  ~GGEMSOpenCLProgram() = default;

  /*!
   * \brief Disables copy construction.
   */
  GGEMSOpenCLProgram(GGEMSOpenCLProgram const &) = delete;
  /*!
   * \brief Disables copy assignment.
   *
   * \return Reference to this program wrapper.
   */
  auto operator=(GGEMSOpenCLProgram const &) -> GGEMSOpenCLProgram & = delete;
  /*!
   * \brief Disables move construction.
   */
  GGEMSOpenCLProgram(GGEMSOpenCLProgram &&) noexcept = delete;
  /*!
   * \brief Disables move assignment.
   *
   * \return Reference to this program wrapper.
   */
  auto operator=(GGEMSOpenCLProgram &&) noexcept
      -> GGEMSOpenCLProgram & = delete;

  /*!
   * \brief Creates a native OpenCL kernel from this program.
   *
   * \param[in] kernel_name Kernel function name.
   * \return Native OpenCL kernel.
   */
  [[nodiscard]] auto CreateKernel(std::string const &kernel_name) const
      -> cl::Kernel;

  /*!
   * \brief Returns the native OpenCL program.
   *
   * \return Native OpenCL program.
   */
  [[nodiscard]] auto GetProgramNative() const noexcept -> cl::Program const & {
    return program_;
  }

  /*!
   * \brief Returns the configured kernel source name.
   *
   * \return Kernel source name.
   */
  [[nodiscard]] auto GetKernelName() const noexcept -> std::string_view {
    return kernel_name_;
  }

  /*!
   * \brief Returns the resolved kernel source path.
   *
   * \return Resolved kernel source path.
   */
  [[nodiscard]] auto GetSourcePath() const noexcept -> std::string_view {
    return source_path_;
  }

  /*!
   * \brief Returns the effective OpenCL build options.
   *
   * \return Effective OpenCL build options.
   */
  [[nodiscard]] auto GetBuildOptions() const noexcept -> std::string_view {
    return build_options_;
  }

  /*!
   * \brief Returns the number of devices associated with the program.
   *
   * \return Number of associated devices.
   */
  [[nodiscard]] auto GetNumDevices() const -> cl_uint;

  /*!
   * \brief Returns the compiled binary sizes for associated devices.
   *
   * \return Compiled binary sizes.
   */
  [[nodiscard]] auto GetBinarySizes() const -> std::vector<std::size_t>;

  /*!
   * \brief Returns the compiled program binaries.
   *
   * \return Compiled program binaries.
   */
  [[nodiscard]] auto GetBinaries() const
      -> std::vector<std::vector<unsigned char>>;

  /*!
   * \brief Checks whether this cached program matches a requested build identity.
   *
   * \param[in] context Requested OpenCL context.
   * \param[in] kernel_root Requested kernel root directory.
   * \param[in] kernel_name Requested kernel source name.
   * \param[in] user_build_options Requested user build options.
   * \return True if this program matches the requested build identity.
   */
  [[nodiscard]] auto Matches(GGEMSOpenCLContext const &context,
                             std::filesystem::path const &kernel_root,
                             std::string_view kernel_name,
                             std::string_view user_build_options) const -> bool;

private:
  /*!
   * \brief Loads a text file into memory.
   *
   * \param[in] path File path to read.
   * \return Complete text file contents.
   */
  [[nodiscard]]
  static auto LoadTextFile(std::filesystem::path const &path) -> std::string;

  /*!
   * \brief Builds the default GGEMS OpenCL compilation options.
   *
   * \return Default OpenCL build-option tokens.
   */
  [[nodiscard]] auto BuildOptions() const -> std::vector<std::string>;

  /*!
   * \brief Merges default and user-supplied OpenCL build options.
   *
   * \param[in] base Default build-option tokens.
   * \param[in] extra Additional user build options.
   * \return Merged OpenCL build-option string.
   */
  [[nodiscard]] static auto MergeOptions(std::vector<std::string> const &base,
                                         std::string const &extra)
      -> std::string;

  /*!
   * \brief Resolves source paths and computes program fingerprints.
   */
  auto Initialize() -> void;

  /*!
   * \brief Builds the program from cache or source.
   */
  auto Build() -> void;

  /*!
   * \brief Builds the OpenCL program from source text.
   *
   * \param[in] source OpenCL source text.
   */
  auto BuildFromSource(std::string const &source) -> void;

  /*!
   * \brief Builds the OpenCL program from a cached binary.
   *
   * \param[in] binary Cached OpenCL program binary.
   */
  auto BuildFromBinary(std::vector<std::uint8_t> const &binary) -> void;

  /*!
   * \brief Computes the persistent cache path for this program identity.
   *
   * \return Persistent program-cache path.
   */
  [[nodiscard]] auto ComputeCachePath() const -> std::filesystem::path;

  /*!
   * \brief Writes the built program binary to the persistent cache.
   */
  auto SaveBinaryToCache() -> void;

  /*!
   * \brief Loads a cached OpenCL program binary when available.
   *
   * \return Cached binary bytes, or an empty vector when no usable cache entry is available.
   */
  auto LoadBinaryFromCache() -> std::vector<std::uint8_t>;

  /*!
   * \brief Builds normalized include-search roots used for source fingerprinting.
   *
   * \return Normalized include-search roots.
   */
  [[nodiscard]] auto BuildIncludeSearchRoots() const
      -> std::vector<std::filesystem::path>;

  /*!
   * \brief Builds deterministic fingerprint text for a source file and its local includes.
   *
   * \param[in] source_path Root source file to fingerprint.
   * \return Deterministic source fingerprint text.
   */
  [[nodiscard]] auto
  BuildSourceFingerprintText(std::filesystem::path const &source_path) const
      -> std::string;

  /*!
   * \brief Appends one source file and recursively resolved local includes to fingerprint text.
   *
   * \param[in] source_path Source file to append.
   * \param[in] include_roots Include-search roots.
   * \param[in,out] visited_sources Normalized source paths already visited.
   * \param[in,out] fingerprint_text Accumulated fingerprint text.
   */
  auto AppendSourceFingerprintText(
      std::filesystem::path const &source_path,
      std::vector<std::filesystem::path> const &include_roots,
      std::unordered_set<std::string> &visited_sources,
      std::string &fingerprint_text) const -> void;

  /*!
   * \brief Retained native OpenCL context.
   */
  cl::Context context_;
  /*!
   * \brief Retained native OpenCL device.
   */
  cl::Device device_;
  /*!
   * \brief Kernel source root directory.
   */
  std::filesystem::path kernel_root_;
  /*!
   * \brief Kernel source name.
   */
  std::string kernel_name_;
  /*!
   * \brief Resolved kernel source path.
   */
  std::string source_path_;
  /*!
   * \brief User-supplied OpenCL build options.
   */
  std::string user_build_options_;
  /*!
   * \brief Effective merged OpenCL build options.
   */
  std::string build_options_;
  /*!
   * \brief Most recent OpenCL program build log.
   */
  std::string build_log_;
  /*!
   * \brief Native OpenCL program.
   */
  cl::Program program_;
  /*!
   * \brief Fingerprint hash of source text and recursively included local sources.
   */
  std::uint64_t source_hash_;
  /*!
   * \brief Fingerprint hash of the complete program build identity.
   */
  std::uint64_t global_hash_;
};
} // namespace ggems::ocl
