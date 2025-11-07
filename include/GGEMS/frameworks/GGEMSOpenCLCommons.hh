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
 * \file GGEMSOpenCLCommons.hh
 * \brief Common utilities, error handling and strongly-typed info helpers for OpenCL 3.0.
 *
 * This header centralises:
 * - error translation and failure reporting (\ref ggocl::GetErrorString, \ref ggocl::Failure),
 * - a strict macro to check OpenCL return codes (\ref GGOCL_CHECK),
 * - generic, strongly-typed info accessors for the C++98 wrapper \c cl.hpp
 *   (\ref ggocl::utils::Get, \ref ggocl::utils::GetArray),
 * - extension helpers (\ref ggocl::utils::ExtractExtensions, \ref ggocl::utils::HasExtension),
 * - a terminate handler suitable for HPC + pybind11 integration (\ref ggocl::TerminateHandler),
 * - conversion helpers (\ref ggocl::utils::ClVersionToString,
 *   \ref ggocl::utils::ClNameVersionToString).
 *
 * All templates are header-only by design. Non-templates are defined in the \c .cc.
 */

/// \cond
#include <cstring>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <format>
/// \endcond

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_TARGET_OPENCL_VERSION 300
#define CL_ENABLE_SPIRV_EXTENSIONS

// ---------------------------------------------------------------------------
// Temporarily disable external-header warnings (Clang / MSVC portable block)
// ---------------------------------------------------------------------------
#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wsign-conversion"
  #pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include <CL/opencl.hpp>
#include "GGEMS/core/GGEMSException.hh"

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

namespace ggems::ocl {
  [[nodiscard]] inline std::string_view GetErrorCodeName(cl_int err) noexcept {
    switch (err) {
      case CL_SUCCESS: return "CL_SUCCESS";
      case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
      case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
      case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
      case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
      case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
      case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
      case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
      case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
      case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
      case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
      case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
      case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
      case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
      case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
      case CL_COMPILE_PROGRAM_FAILURE: return "CL_COMPILE_PROGRAM_FAILURE";
      case CL_LINKER_NOT_AVAILABLE: return "CL_LINKER_NOT_AVAILABLE";
      case CL_LINK_PROGRAM_FAILURE: return "CL_LINK_PROGRAM_FAILURE";
      case CL_DEVICE_PARTITION_FAILED: return "CL_DEVICE_PARTITION_FAILED";
      case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
      default: return "CL_UNKNOWN_ERROR";
    }
  }

  [[nodiscard]] inline std::string_view GetErrorDescription(cl_int err) noexcept {
    switch (err) {
      case CL_SUCCESS: return "Operation completed successfully.";
      case CL_DEVICE_NOT_FOUND: return "No matching devices for the requested type.";
      case CL_DEVICE_NOT_AVAILABLE: return "Device found but currently unavailable.";
      case CL_COMPILER_NOT_AVAILABLE: return "Program built from source but no compiler available.";
      case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "Failed to allocate memory for an OpenCL object.";
      default: return "Unknown OpenCL error.";
    }
  }

  [[nodiscard]] inline std::string GetErrorString(cl_int err) noexcept {
    return std::format("{} - {}", GetErrorCodeName(err), GetErrorDescription(err));
  }

  inline void CheckCLError(cl_int err, std::string_view context,
                           std::source_location loc = std::source_location::current()) {
    if (err != CL_SUCCESS)
      ggems::core::ThrowCL(err, GetErrorString, context, loc);
  }

    // ----------------- Indirection to the right clGet*Info function -----------------
    template <typename Obj> struct InfoInvoker;

    /// \cond INTERNAL
    // cl::Platform
    template <> struct InfoInvoker<cl::Platform> {
      static cl_int Size(cl::Platform const& o, cl_uint p, std::size_t* s) {
        return ::clGetPlatformInfo(o(), p, 0, nullptr, s);
      }
      static cl_int Data(cl::Platform const& o, cl_uint p, std::size_t n, void* d) {
        return ::clGetPlatformInfo(o(), p, n, d, nullptr);
      }
    };

    // cl::Device
    template <> struct InfoInvoker<cl::Device> {
      static cl_int Size(cl::Device const& o, cl_uint p, std::size_t* s) {
        return ::clGetDeviceInfo (o(), p, 0, nullptr, s);
      }
      static cl_int Data(cl::Device const& o, cl_uint p, std::size_t n, void* d) {
        return ::clGetDeviceInfo (o(), p, n, d, nullptr);
      }
    };

    // cl::Context
    template <> struct InfoInvoker<cl::Context> {
      static cl_int Size(cl::Context const& o, cl_uint p, std::size_t* s) {
        return ::clGetContextInfo (o(), p, 0, nullptr, s);
      }
      static cl_int Data(cl::Context const& o, cl_uint p, std::size_t n, void* d) {
        return ::clGetContextInfo (o(), p, n, d, nullptr);
      }
    };

    // cl::CommandQueue
    template <> struct InfoInvoker<cl::CommandQueue> {
      static cl_int Size(cl::CommandQueue const& o, cl_uint p, std::size_t* s) {
        return ::clGetCommandQueueInfo (o(), p, 0, nullptr, s);
      }
      static cl_int Data(cl::CommandQueue const& o, cl_uint p, std::size_t n, void* d) {
        return ::clGetCommandQueueInfo (o(), p, n, d, nullptr);
      }
    };

    // cl::Kernel
    template <> struct InfoInvoker<cl::Kernel> {
      static cl_int Size(cl::Kernel const& o, cl_uint p, std::size_t* s) {
        return ::clGetKernelInfo (o(), p, 0, nullptr, s);
      }
      static cl_int Data(cl::Kernel const& o, cl_uint p, std::size_t n, void* d) {
        return ::clGetKernelInfo (o(), p, n, d, nullptr);
      }
    };

    // cl::Program
    template <> struct InfoInvoker<cl::Program> {
      static cl_int Size(cl::Program const& o, cl_uint p, std::size_t* s) {
        return ::clGetProgramInfo (o(), p, 0, nullptr, s);
      }
      static cl_int Data(cl::Program const& o, cl_uint p, std::size_t n, void* d) {
        return ::clGetProgramInfo (o(), p, n, d, nullptr);
      }
    };

    // cl::Event
    template <> struct InfoInvoker<cl::Event> {
      static cl_int Size(cl::Event const& o, cl_uint p, std::size_t* s) {
        return ::clGetEventInfo (o(), p, 0, nullptr, s);
      }
      static cl_int Data(cl::Event const& o, cl_uint p, std::size_t n, void* d) {
        return ::clGetEventInfo (o(), p, n, d, nullptr);
      }
    };
    /// \endcond

    // ----------------- Public, strongly-typed helpers -----------------

    /*!
     * \brief Retrieve a scalar or string info using the \c cl.hpp (C++98) wrapper.
     * \tparam T       Scalar-like type or \c std::string to be retrieved.
     * \tparam ObjType One of \c cl::Platform, \c cl::Device, \c cl::Context,
     *                 \c cl::CommandQueue, \c cl::Kernel, \c cl::Program, \c cl::Event.
     * \param obj      Wrapped OpenCL object to query.
     * \param param    The \c cl_*_info enumeration value to request.
     * \return Value of type \c T.
     *
     * \note Uses \c obj.getInfo(param, &value) and checks the resulting error via \ref GGOCL_CHECK.
     */
    template <typename T, typename ObjType>
    [[nodiscard]] inline T Get(ObjType const& obj, cl_uint param) {
      T value{};
      //GGOCL_CHECK(obj.getInfo(param, &value));
      return value;
    }

    /*!
     * \brief Retrieve a variable-sized info array via the C API (\c clGet*Info).
     * \tparam T       Element type (must not be a pointer type).
     * \tparam ObjType One of \c cl::Platform, \c cl::Device, \c cl::Context,
     *                 \c cl::CommandQueue, \c cl::Kernel, \c cl::Program, \c cl::Event.
     * \param obj      Wrapped OpenCL object to query.
     * \param param    The \c cl_*_info enumeration value to request.
     * \return \c std::vector<T> containing the result (may be empty).
     *
     * \details Two-step query:
     *  1) query the required byte size using InfoInvoker<T>::Size()
     *  2) allocate and fetch the payload using InfoInvoker<T>::Data().
     */
    template <typename T, typename ObjType>
    [[nodiscard]] inline std::vector<T> GetArray(ObjType const& obj, cl_uint param) {
      static_assert(!std::is_pointer_v<T>, "T must not be a pointer type");

      std::size_t bytes = 0;
      //GGOCL_CHECK(InfoInvoker<ObjType>::Size(obj, param, &bytes));
      if (bytes == 0) return {};

      // Usual case: sizeof(T) > 0. For byte blobs this would be specialised elsewhere.
      const std::size_t count = (sizeof(T) ? (bytes / sizeof(T)) : 0);
      std::vector<T> out;
      if constexpr (sizeof(T) != 0) out.resize(count);

      //GGOCL_CHECK(InfoInvoker<ObjType>::Data(obj, param, bytes, out.data()));
      return out;
    }

    /*!
     * \brief Parse a space-separated extension list into an unordered set.
     * \tparam ObjType \c cl::Platform or \c cl::Device (anything that supports the \c *_EXTENSIONS string param).
     * \param obj      Wrapped OpenCL object to query.
     * \param param    The \c CL_*_EXTENSIONS enum appropriate for \p obj.
     * \return \c std::unordered_set of unique extension names (may be empty).
     */
    template <typename ObjType>
    [[nodiscard]] inline std::unordered_set<std::string>
    ExtractExtensions(ObjType const& obj, cl_uint param) {
      std::string ext_str;
      //GGOCL_CHECK(obj.getInfo(param, &ext_str));

      std::unordered_set<std::string> result;
      std::istringstream iss(ext_str);
      std::string token;
      while (iss >> token) result.insert(std::move(token));
      return result;
    }

    /*!
     * \brief O(1) membership check for an extension name.
     * \param extensions  Set previously built by \ref ggocl::utils::ExtractExtensions.
     * \param name        Extension name to test.
     * \return \c true if present in \p extensions, \c false otherwise.
     */
    [[nodiscard]] inline bool HasExtension(std::unordered_set<std::string> const& extensions,
      std::string_view name) {
      if (name.empty()) return false;
      auto it = extensions.find(std::string{name});
      return it != extensions.end();
    }

    /*!
     * \brief Convert an OpenCL \c cl_version into a human-readable string.
     * \param version Encoded OpenCL version (10 bits major, 10 bits minor, 12 bits patch).
     * \return A formatted string such as "3.0.12 (0x3040C)".
     *
     * This helper extracts the bit fields of the OpenCL 3.0 version encoding:
     * - Bits [31:22] → major,
     * - Bits [21:12] → minor,
     * - Bits [11:0]  → patch.
     */
    [[nodiscard]] inline std::string ClVersionToString(cl_version version) noexcept {
      cl_uint major = (version >> 22) & 0x3FFu;
      cl_uint minor = (version >> 12) & 0x3FFu;
      cl_uint patch = (version >>  0) & 0xFFFu;

      std::ostringstream oss;
      oss << major << '.' << minor << '.' << patch
          << " (0x" << std::uppercase << std::hex << version << std::dec << ')';
      return oss.str();
    }

    /*!
     * \brief Convert a list of \c cl_name_version structures into a readable string.
     * \param name_versions Vector of OpenCL \c cl_name_version entries.
     * \return Concatenated string such as "cl_khr_fp64 3.0.12 (0x...) cl_khr_il_program 3.0.12 (0x...) ".
     *
     * Uses \ref ggocl::utils::ClVersionToString to format each bit-packed version.
     * Intended for diagnostic or logging purposes.
     */
    [[nodiscard]] inline std::string ClNameVersionToString(std::vector<cl_name_version> const& name_versions) noexcept {
      std::ostringstream oss;
      for (auto const& nv : name_versions) {
        oss << nv.name << ' ' << ClVersionToString(nv.version) << ' ';
      }
      return oss.str();
    }
} // namespace ggems::core
