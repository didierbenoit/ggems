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
/// \endcond

#ifdef __APPLE__
  #include <OpenCL/opencl.hpp>
#else
  #include <CL/cl.hpp>
#endif

#ifdef _MSC_VER
  #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

/*!
 * \def __FILENAME__
 * \brief Basename-only view of \c __FILE__ for concise diagnostics.
 */
#ifdef _WIN32
  #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
  #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/*!
 * \def GGOCL_CHECK
 * \brief Check an OpenCL call result and throw \c GGEMSException on failure.
 *
 * Casts to \c cl_int explicitly for portability across vendors and toolchains.
 * The macro calls \ref ggocl::Failure which throws on any code different from \c CL_SUCCESS.
 *
 * Usage (any cl.hpp call returning a \c cl_int):
 * \code
 * cl_int err = obj.getInfo(CL_DEVICE_NAME, &value);
 * GGOCL_CHECK(err);
 * \endcode
 */
#define GGOCL_CHECK(error) \
  (ggocl::Failure(__FILENAME__, __PRETTY_FUNCTION__, __LINE__, static_cast<cl_int>(error)))

/*!
 * \namespace ggocl
 * \brief OpenCL utilities and error-management for GGEMS.
 */
namespace ggocl {

  /*!
   * \brief Translate an OpenCL error code into a human-readable string.
   * \param error_code OpenCL error code.
   * \return Human-readable description of \p error_code.
   *
   * Covers core and common vendor/extension ranges. Always returns a valid string.
   */
  [[nodiscard]] std::string GetErrorString(cl_int error_code);

  /*!
   * \brief Failure reporter: throws a \c GGEMSException if \p error_code != \c CL_SUCCESS.
   * \param filename      Basename of the source file where the failure occurred.
   * \param function_name Callable/function where the failure occurred.
   * \param line          Source line of the failure.
   * \param error_code    OpenCL error code.
   *
   * The thrown exception embeds a structured, multi-line diagnostic message including
   * file, function, line and a description of the OpenCL error.
   */
  void Failure(std::string_view filename, std::string_view function_name, int line, cl_int error_code);

  /*!
   * \brief A terminate handler suitable for HPC contexts and pybind11 integration.
   *
   * Logs a short message to \c std::cerr and aborts the process immediately.
   * Intended to be installed via \c std::set_terminate in the framework initialisation.
   * This function never returns.
   */
  [[noreturn]] void TerminateHandler() noexcept;

  // ---------------------------------------------------------------------------
  // Strongly-typed info accessors (C++98 cl.hpp wrapper, OpenCL 3.0)
  // ---------------------------------------------------------------------------

  /*!
   * \namespace ggocl::utils
   * \brief Header-only utilities to extract platform/device/context/queue/kernel/program info.
   *
   * The \c cl.hpp (C++98) wrapper provides an overload:
   * \code
   * obj.getInfo(param, &value)
   * \endcode
   * that we leverage for **scalars** and \c std::string (see \ref ggocl::utils::Get).
   *
   * For variable-sized arrays we switch to the C API (\c clGet*Info) through a tiny
   * indirection layer (see \ref ggocl::utils::InfoInvoker) and the public helper
   * \ref ggocl::utils::GetArray.
   *
   * Extra helpers:
   * - \ref ggocl::utils::ExtractExtensions : splits a space-separated list into a set.
   * - \ref ggocl::utils::HasExtension     : O(1) membership check on the set.
   * - \ref ggocl::utils::ClVersionToString : converts \c cl_version to \c std::string.
   * - \ref ggocl::utils::ClNameVersionToString : converts a \c std::vector<cl_name_version> to \c std::string.
   */
  namespace utils {

    // ----------------- Indirection to the right clGet*Info function -----------------

    /*!
     * \brief Low-level indirection that binds an OpenCL C info query function to an object type.
     *
     * Primary template is intentionally undefined. Specialisations provide two static functions:
     * - \c Size(Obj const&, cl_uint, std::size_t*)          — to query the required byte size,
     * - \c Data(Obj const&, cl_uint, std::size_t, void*)    — to fetch the payload.
     *
     * Provided specialisations:
     * - \c cl::Platform  -> \c clGetPlatformInfo
     * - \c cl::Device    -> \c clGetDeviceInfo
     * - \c cl::Context   -> \c clGetContextInfo
     * - \c cl::CommandQueue -> \c clGetCommandQueueInfo
     * - \c cl::Kernel    -> \c clGetKernelInfo
     * - \c cl::Program   -> \c clGetProgramInfo
     * - \c cl::Event     -> \c clGetEventInfo
     *
     * \tparam Obj OpenCL C++ wrapper type for which an information query indirection is provided.
     */
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
      GGOCL_CHECK(obj.getInfo(param, &value));
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
      GGOCL_CHECK(InfoInvoker<ObjType>::Size(obj, param, &bytes));
      if (bytes == 0) return {};

      // Usual case: sizeof(T) > 0. For byte blobs this would be specialised elsewhere.
      const std::size_t count = (sizeof(T) ? (bytes / sizeof(T)) : 0);
      std::vector<T> out;
      if constexpr (sizeof(T) != 0) out.resize(count);

      GGOCL_CHECK(InfoInvoker<ObjType>::Data(obj, param, bytes, out.data()));
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
      GGOCL_CHECK(obj.getInfo(param, &ext_str));

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

  } // namespace utils
} // namespace ggocl
