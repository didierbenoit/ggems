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
 * \file GGEMSOpenCLPlatform.hh
 * \brief Declaration of the GGEMSOpenCLPlatform class for OpenCL 3.0 platform
 * abstraction.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-14
 * \copyright GNU General Public License v3.0
 * \version 3.0
 *
 * This header defines the \c GGEMSOpenCLPlatform class, an OpenCL 3.0 platform
 * façade that:
 * - stores the native \c cl::Platform and its stable index,
 * - discovers and owns all CPU/GPU devices on the platform,
 * - exposes strongly-typed getters for platform information,
 * - formats a comprehensive textual report via the GGEMS logger.
 *
 * The design is RAII-driven and thread-safe at the logging boundary. Device
 * ownership is unique and non-transferable (vector of \c std::unique_ptr).
 * The API emphasises const-correctness and minimal exposure of internals.
 */

/// \cond
#include <string>
#include <unordered_set>
#include <vector>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {
// Forward declaration to decouple headers (the destructor is out-of-line).
class GGEMSOpenCLDevice;

/*!
 * \class GGEMSOpenCLPlatform
 * \brief Encapsulates a single OpenCL 3.0 platform and its CPU/GPU devices.
 *
 * \note Only CPU and GPU device types are discovered by default. This matches
 * GGEMS multi-architecture philosophy while avoiding exotic device kinds unless
 * explicitly needed later.
 */
class GGEMSOpenCLPlatform {
public:
  /*!
   * \brief Construct a platform façade with a native platform and its stable
   * index.
   * \param platform        Native OpenCL platform (cl::Platform) to wrap.
   * \param platform_index  Stable index of this platform in the system
   * enumeration.
   *
   * The constructor extracts the platform extensions list and immediately
   * discovers CPU/GPU devices. No contexts are created here; that is deferred
   * to higher-level orchestration (e.g. GGEMSOpenCL / GGEMSManager).
   */
  explicit GGEMSOpenCLPlatform(cl::Platform const &platform,
                               std::size_t platform_index);

  /*!
   * \brief Deleted default constructor — a platform façade must wrap a valid
   * platform.
   */
  GGEMSOpenCLPlatform() = delete;

  /*!
   * \brief Destructor — logs lifecycle and releases owned devices.
   *
   * Device objects are destroyed in reverse order of insertion. The native
   * platform object has no active lifetime management beyond its wrapper.
   */
  ~GGEMSOpenCLPlatform();

  // Non-copyable, non-movable — preserves ownership and index stability.
  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform const &) = delete;
  GGEMSOpenCLPlatform &operator=(GGEMSOpenCLPlatform const &) = delete;

  /*!
   * \brief Move constructor (noexcept).
   *
   * Enables storage within STL containers such as \c std::vector.
   * Ownership of devices and extension caches is transferred.
   */
  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform &&) noexcept = default;

  /*!
   * \brief Move assignment operator (noexcept).
   * \return A reference to GGEMSOpenCLPlatform
   * Transfers ownership of all internal data to the destination object.
   */
  GGEMSOpenCLPlatform &operator=(GGEMSOpenCLPlatform &&) noexcept = delete;

public:
  // -------------------- High-level inspection API --------------------

  /*!
   * \brief Check whether a platform-level extension is advertised.
   * \param extension_name Name of the extension (e.g. "cl_khr_icd").
   * \return True if present in the platform's extension set, false otherwise.
   */
  [[nodiscard]] bool CheckExtension(std::string_view extension_name) const;

  /*!
   * \brief Retrieve the platform's human-readable name.
   * \return Platform name (e.g. "NVIDIA CUDA", "Intel(R) OpenCL", ...).
   */
  [[nodiscard]] std::string GetName() const;

  /*!
   * \brief Retrieve the supported OpenCL profile string.
   * \return Typically "FULL_PROFILE" for general-purpose platforms.
   */
  [[nodiscard]] std::string GetProfile() const;

  /*!
   * \brief Retrieve the platform version string.
   * \return String formatted as "OpenCL <major>.<minor> <vendor-info>".
   */
  [[nodiscard]] std::string GetVersion() const;

  /*!
   * \brief Retrieve the platform vendor string.
   * \return Vendor name (e.g. "NVIDIA Corporation", "Advanced Micro Devices,
   * Inc.").
   */
  [[nodiscard]] std::string GetVendor() const;

  /*!
   * \brief Retrieve the space-separated list of platform extensions (legacy
   * format).
   * \return Space-separated extension list (OpenCL core requirement).
   */
  [[nodiscard]] std::string GetExtensions() const;

  /*!
   * \brief Retrieve the numeric OpenCL version (major/minor/patch packed).
   * \return \c cl_version with packed fields per OpenCL 3.0 specification.
   */
  [[nodiscard]] cl_version GetNumericVersion() const;

  /*!
   * \brief Retrieve the host timer resolution in nanoseconds.
   * \return Timer resolution (\c cl_ulong).
   */
  [[nodiscard]] cl_ulong GetHostTimerResolution() const;

  /*!
   * \brief Retrieve the extensions with versions (OpenCL 3.0).
   * \return Vector of \c cl_name_version for all advertised extensions.
   */
  [[nodiscard]] std::vector<cl_name_version> GetExtensionsWithVersion() const;

  /*!
   * \brief Print a comprehensive platform report to the terminal using GGEMS
   * logger.
   *
   * The report includes:
   * - Basic identity (name, vendor, profile, version),
   * - Numeric version (decoded as major.minor.patch),
   * - Host timer resolution,
   * - Extensions with their version triplets,
   * - ICD suffix,
   * - A summary of discovered devices (count only; device details are handled
   * by the device layer).
   */
  void Print() const;

  /*!
   * \brief Explicitly release platform-level resources and owned devices.
   *
   * Unloads the platform compiler (if any), clears the device list and cached
   * extension set. Safe to call multiple times; typically invoked during
   * shutdown.
   */
  void Clean();

  // -------------------- Accessors for orchestration layers
  // --------------------

  /*!
   * \brief Get the stable index of this platform in the system enumeration.
   * \return Zero-based platform index.
   */
  [[nodiscard]] std::size_t GetPlatformIndex() const noexcept {
    return platform_index_;
  }

  /*!
   * \brief Access the native \c cl::Platform wrapper (const).
   * \return Const reference to the wrapped native platform object.
   */
  [[nodiscard]] cl::Platform const &GetNative() const noexcept {
    return platform_;
  }

  /*!
   * \brief Non-owning, read-only view of discovered devices.
   * \return A vector of raw pointers to const \c GGEMSOpenCLDevice instances.
   *
   * The platform retains ownership. Returned pointers remain valid until either
   * \c Clean() is called or the platform object is destroyed.
   */
  [[nodiscard]] std::vector<GGEMSOpenCLDevice> const &
  GetDevices() const noexcept {
    return devices_;
  }

private:
  void PrintIdentity() const;
  void PrintExtension() const;

private:
  // -------------------- Internal discovery --------------------

  /*!
   * \brief Discover CPU and GPU devices on this platform and instantiate
   * wrappers.
   *
   * Only devices of type \c CL_DEVICE_TYPE_CPU and \c CL_DEVICE_TYPE_GPU are
   * enumerated. Each native device is wrapped in a \c GGEMSOpenCLDevice and
   * owned by this platform.
   */
  void DiscoverDevices();

private:
  cl::Platform platform_;      /*!< Native OpenCL platform wrapper */
  std::size_t platform_index_; /*!< Stable platform index */
  std::unordered_set<std::string>
      extensions_; /*!< Cached platform extension names */
  std::vector<GGEMSOpenCLDevice> devices_; /*!< Owned CPU/GPU device wrappers */
};
} // namespace ggems::ocl
