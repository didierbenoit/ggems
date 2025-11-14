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
 * \brief Common utilities, error handling and strongly-typed info helpers for
 * OpenCL 3.0.
 *
 * This header centralises:
 * - error translation and failure reporting (\ref ggocl::GetErrorString, \ref
 * ggocl::Failure),
 * - a strict macro to check OpenCL return codes (\ref GGOCL_CHECK),
 * - generic, strongly-typed info accessors for the C++98 wrapper \c cl.hpp
 *   (\ref ggocl::utils::Get, \ref ggocl::utils::GetArray),
 * - extension helpers (\ref ggocl::utils::ExtractExtensions, \ref
 * ggocl::utils::HasExtension),
 * - a terminate handler suitable for HPC + pybind11 integration (\ref
 * ggocl::TerminateHandler),
 * - conversion helpers (\ref ggocl::utils::ClVersionToString,
 *   \ref ggocl::utils::ClNameVersionToString).
 *
 * All templates are header-only by design. Non-templates are defined in the \c
 * .cc.
 */

/// \cond
#include <cstring>
#include <format>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
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

/// \cond
#include <CL/opencl.hpp>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLInfoTraits.hh"

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace ggems::ocl {
// === Error name analyse ===
[[nodiscard]] inline std::string_view GetErrorCodeName(cl_int err) noexcept {
  switch (err) {
  // === Run-time and JIT Compiler Errors (driver-dependent) ===
  case CL_SUCCESS:
    return "CL_SUCCESS";
  case CL_DEVICE_NOT_FOUND:
    return "CL_DEVICE_NOT_FOUND";
  case CL_DEVICE_NOT_AVAILABLE:
    return "CL_DEVICE_NOT_AVAILABLE";
  case CL_COMPILER_NOT_AVAILABLE:
    return "CL_COMPILER_NOT_AVAILABLE";
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
    return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
  case CL_OUT_OF_RESOURCES:
    return "CL_OUT_OF_RESOURCES";
  case CL_OUT_OF_HOST_MEMORY:
    return "CL_OUT_OF_HOST_MEMORY";
  case CL_PROFILING_INFO_NOT_AVAILABLE:
    return "CL_PROFILING_INFO_NOT_AVAILABLE";
  case CL_MEM_COPY_OVERLAP:
    return "CL_MEM_COPY_OVERLAP";
  case CL_IMAGE_FORMAT_MISMATCH:
    return "CL_IMAGE_FORMAT_MISMATCH";
  case CL_IMAGE_FORMAT_NOT_SUPPORTED:
    return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
  case CL_BUILD_PROGRAM_FAILURE:
    return "CL_BUILD_PROGRAM_FAILURE";
  case CL_MAP_FAILURE:
    return "CL_MAP_FAILURE";
  case CL_MISALIGNED_SUB_BUFFER_OFFSET:
    return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
    return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
  case CL_COMPILE_PROGRAM_FAILURE:
    return "CL_COMPILE_PROGRAM_FAILURE";
  case CL_LINKER_NOT_AVAILABLE:
    return "CL_LINKER_NOT_AVAILABLE";
  case CL_LINK_PROGRAM_FAILURE:
    return "CL_LINK_PROGRAM_FAILURE";
  case CL_DEVICE_PARTITION_FAILED:
    return "CL_DEVICE_PARTITION_FAILED";
  case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
    return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

  // === Compile-time Errors (driver-independent) ===
  case CL_INVALID_VALUE:
    return "CL_INVALID_VALUE";
  case CL_INVALID_DEVICE_TYPE:
    return "CL_INVALID_DEVICE_TYPE";
  case CL_INVALID_PLATFORM:
    return "CL_INVALID_PLATFORM";
  case CL_INVALID_DEVICE:
    return "CL_INVALID_DEVICE";
  case CL_INVALID_CONTEXT:
    return "CL_INVALID_CONTEXT";
  case CL_INVALID_QUEUE_PROPERTIES:
    return "CL_INVALID_QUEUE_PROPERTIES";
  case CL_INVALID_COMMAND_QUEUE:
    return "CL_INVALID_COMMAND_QUEUE";
  case CL_INVALID_HOST_PTR:
    return "CL_INVALID_HOST_PTR";
  case CL_INVALID_MEM_OBJECT:
    return "CL_INVALID_MEM_OBJECT";
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
  case CL_INVALID_IMAGE_SIZE:
    return "CL_INVALID_IMAGE_SIZE";
  case CL_INVALID_SAMPLER:
    return "CL_INVALID_SAMPLER";
  case CL_INVALID_BINARY:
    return "CL_INVALID_BINARY";
  case CL_INVALID_BUILD_OPTIONS:
    return "CL_INVALID_BUILD_OPTIONS";
  case CL_INVALID_PROGRAM:
    return "CL_INVALID_PROGRAM";
  case CL_INVALID_PROGRAM_EXECUTABLE:
    return "CL_INVALID_PROGRAM_EXECUTABLE";
  case CL_INVALID_KERNEL_NAME:
    return "CL_INVALID_KERNEL_NAME";
  case CL_INVALID_KERNEL_DEFINITION:
    return "CL_INVALID_KERNEL_DEFINITION";
  case CL_INVALID_KERNEL:
    return "CL_INVALID_KERNEL";
  case CL_INVALID_ARG_INDEX:
    return "CL_INVALID_ARG_INDEX";
  case CL_INVALID_ARG_VALUE:
    return "CL_INVALID_ARG_VALUE";
  case CL_INVALID_ARG_SIZE:
    return "CL_INVALID_ARG_SIZE";
  case CL_INVALID_KERNEL_ARGS:
    return "CL_INVALID_KERNEL_ARGS";
  case CL_INVALID_WORK_DIMENSION:
    return "CL_INVALID_WORK_DIMENSION";
  case CL_INVALID_WORK_GROUP_SIZE:
    return "CL_INVALID_WORK_GROUP_SIZE";
  case CL_INVALID_WORK_ITEM_SIZE:
    return "CL_INVALID_WORK_ITEM_SIZE";
  case CL_INVALID_GLOBAL_OFFSET:
    return "CL_INVALID_GLOBAL_OFFSET";
  case CL_INVALID_EVENT_WAIT_LIST:
    return "CL_INVALID_EVENT_WAIT_LIST";
  case CL_INVALID_EVENT:
    return "CL_INVALID_EVENT:";
  case CL_INVALID_OPERATION:
    return "CL_INVALID_OPERATION";
  case CL_INVALID_GL_OBJECT:
    return "CL_INVALID_GL_OBJECT";
  case CL_INVALID_BUFFER_SIZE:
    return "CL_INVALID_BUFFER_SIZE";
  case CL_INVALID_MIP_LEVEL:
    return "CL_INVALID_MIP_LEVEL";
  case CL_INVALID_GLOBAL_WORK_SIZE:
    return "CL_INVALID_GLOBAL_WORK_SIZE";
  case CL_INVALID_PROPERTY:
    return "CL_INVALID_PROPERTY";
  case CL_INVALID_IMAGE_DESCRIPTOR:
    return "CL_INVALID_IMAGE_DESCRIPTOR";
  case CL_INVALID_COMPILER_OPTIONS:
    return "CL_INVALID_COMPILER_OPTIONS";
  case CL_INVALID_LINKER_OPTIONS:
    return "CL_INVALID_LINKER_OPTIONS";
  case CL_INVALID_DEVICE_PARTITION_COUNT:
    return "CL_INVALID_DEVICE_PARTITION_COUNT";
  case CL_INVALID_PIPE_SIZE:
    return "CL_INVALID_PIPE_SIZE";
  case CL_INVALID_DEVICE_QUEUE:
    return "CL_INVALID_DEVICE_QUEUE";

  // === Errors thrown by extensions ===
  case CL_PLATFORM_NOT_FOUND_KHR:
    return "CL_PLATFORM_NOT_FOUND_KHR";

  default:
    return "CL_UNKNOWN_ERROR";
  }
}

[[nodiscard]] inline std::string_view GetErrorDescription(cl_int err) noexcept {
  switch (err) {
  // === Run-time and JIT Compiler Errors (driver-dependent) ===
  case CL_SUCCESS:
    return "Operation completed successfully.";
  case CL_DEVICE_NOT_FOUND:
    return "No matching devices for the requested type.";
  case CL_DEVICE_NOT_AVAILABLE:
    return "Device found but currently unavailable.";
  case CL_COMPILER_NOT_AVAILABLE:
    return "Program built from source but no compiler available.";
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
    return "Failed to allocate memory for an OpenCL object.";
  case CL_OUT_OF_RESOURCES:
    return "Device-side resource allocation failure.";
  case CL_OUT_OF_HOST_MEMORY:
    return "Host-side resource allocation failure.";
  case CL_PROFILING_INFO_NOT_AVAILABLE:
    return "Queue lacks CL_QUEUE_PROFILING_ENABLE or event not complete.";
  case CL_MEM_COPY_OVERLAP:
    return "Source and destination buffer regions overlap.";
  case CL_IMAGE_FORMAT_MISMATCH:
    return "Incompatible image formats.";
  case CL_IMAGE_FORMAT_NOT_SUPPORTED:
    return "Unsupported image format.";
  case CL_BUILD_PROGRAM_FAILURE:
    return "Program build failed.";
  case CL_MAP_FAILURE:
    return "Mapping the requested region failed.";
  case CL_MISALIGNED_SUB_BUFFER_OFFSET:
    return "Sub-buffer base offset misaligned.";
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
    return "One or more waited events have negative status.";
  case CL_COMPILE_PROGRAM_FAILURE:
    return "Program compilation failed.";
  case CL_LINKER_NOT_AVAILABLE:
    return "Linker unavailable on device.";
  case CL_LINK_PROGRAM_FAILURE:
    return "Linking binaries and/or libraries failed.";
  case CL_DEVICE_PARTITION_FAILED:
    return "Device could not be further partitioned.";
  case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
    return "Argument information is not available for the kernel.";

  // === Compile-time Errors (driver-independent) ===
  case CL_INVALID_VALUE:
    return "One or more arguments have invalid values.";
  case CL_INVALID_DEVICE_TYPE:
    return "Invalid device type.";
  case CL_INVALID_PLATFORM:
    return "Invalid platform parameter.";
  case CL_INVALID_DEVICE:
    return "Invalid or mismatched device.";
  case CL_INVALID_CONTEXT:
    return "Invalid context object.";
  case CL_INVALID_QUEUE_PROPERTIES:
    return "Unsupported queue properties for the device.";
  case CL_INVALID_COMMAND_QUEUE:
    return "Invalid command queue.";
  case CL_INVALID_HOST_PTR:
    return "Inconsistent use of host pointer and flags.";
  case CL_INVALID_MEM_OBJECT:
    return "Invalid memory object.";
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    return "Invalid GL/DX image format mapping.";
  case CL_INVALID_IMAGE_SIZE:
    return "Unsupported image dimensions for the device.";
  case CL_INVALID_SAMPLER:
    return "Invalid sampler object.";
  case CL_INVALID_BINARY:
    return "Invalid program binary for the device list.";
  case CL_INVALID_BUILD_OPTIONS:
    return "Invalid build options string.";
  case CL_INVALID_PROGRAM:
    return "Invalid program object.";
  case CL_INVALID_PROGRAM_EXECUTABLE:
    return "No successfully built executable for the device.";
  case CL_INVALID_KERNEL_NAME:
    return "Kernel name not found in program.";
  case CL_INVALID_KERNEL_DEFINITION:
    return "Kernel definition mismatches across devices.";
  case CL_INVALID_KERNEL:
    return "Invalid kernel object.";
  case CL_INVALID_ARG_INDEX:
    return "Argument index out of range.";
  case CL_INVALID_ARG_VALUE:
    return "Invalid argument value.";
  case CL_INVALID_ARG_SIZE:
    return "Argument size mismatches its type.";
  case CL_INVALID_KERNEL_ARGS:
    return "Kernel arguments not fully specified.";
  case CL_INVALID_WORK_DIMENSION:
    return "Work_dim must be in [1..3].";
  case CL_INVALID_WORK_GROUP_SIZE:
    return "Local size incompatible with kernel/device constraints.";
  case CL_INVALID_WORK_ITEM_SIZE:
    return "Work-items per dimension exceed device limits.";
  case CL_INVALID_GLOBAL_OFFSET:
    return "Offset+size exceeds size_t range for the device.";
  case CL_INVALID_EVENT_WAIT_LIST:
    return "Inconsistent event wait list.";
  case CL_INVALID_EVENT:
    return "Invalid event object.";
  case CL_INVALID_OPERATION:
    return "Operation not permitted in current state.";
  case CL_INVALID_GL_OBJECT:
    return "Invalid or undefined GL object in interop.";
  case CL_INVALID_BUFFER_SIZE:
    return "Size is zero or exceeds max alloc.";
  case CL_INVALID_MIP_LEVEL:
    return "Non-zero mip level unsupported for this interop.";
  case CL_INVALID_GLOBAL_WORK_SIZE:
    return "Global work size is null/zero/out of range.";
  case CL_INVALID_PROPERTY:
    return "Invalid or unsupported property value.";
  case CL_INVALID_IMAGE_DESCRIPTOR:
    return "Invalid image descriptor values.";
  case CL_INVALID_COMPILER_OPTIONS:
    return "Compiler options string invalid.";
  case CL_INVALID_LINKER_OPTIONS:
    return "Linker options string invalid.";
  case CL_INVALID_DEVICE_PARTITION_COUNT:
    return "Sub-device counts exceed device capacity.";
  case CL_INVALID_PIPE_SIZE:
    return "pipe_packet_size is 0 or exceeds CL_DEVICE_PIPE_MAX_PACKET_SIZE";
  case CL_INVALID_DEVICE_QUEUE:
    return "Not a valid device queue object.";

  // === Errors thrown by extensions ===
  case CL_PLATFORM_NOT_FOUND_KHR:
    return "No valid ICDs found.";

  default:
    return "Unknown OpenCL error.";
  }
}

[[nodiscard]] inline std::string GetLongErrorString(cl_int err) noexcept {
  return std::format("{} - {}", GetErrorCodeName(err),
                     GetErrorDescription(err));
}

[[nodiscard]] inline std::string GetErrorString(cl_int err) noexcept {
  return std::format("{}", GetErrorCodeName(err));
}

// === Handles OpenCL errors
template <typename E, typename Enum, typename ToStringFunc>
[[noreturn]] inline void
ThrowCL(Enum code, ToStringFunc toString, std::string_view context,
        std::source_location loc = std::source_location::current()) {
  std::string msg =
      std::format("{} (code {}): {}", context, static_cast<int>(code),
                  toString(static_cast<int>(code)));
  ggems::core::Throw<E>(msg, loc, true);
}

template <typename E = ggems::core::GGEMSFatal>
inline void
CheckCLError(cl_int err, std::string_view context,
             std::source_location loc = std::source_location::current()) {
  if (err != CL_SUCCESS)
    ThrowCL<E>(err, GetLongErrorString, context, loc);
}

// === Info getters
namespace detail {
template <typename T> struct CLInfoReader {
  static T Read(auto obj, cl_uint param, std::size_t size, auto getter,
                cl_int &err) {
    (void)size;
    T value{};
    err = getter(obj(), param, sizeof(T), &value, nullptr);
    return value;
  }
};

template <> struct CLInfoReader<std::string> {
  static std::string Read(auto obj, cl_uint param, size_t size, auto getter,
                          cl_int &err) {
    std::string value(size, '\0');
    err = getter(obj(), param, size, value.data(), nullptr);
    return value;
  }
};

template <typename T, std::size_t N> struct CLInfoReader<std::array<T, N>> {
  static std::array<T, N> Read(auto obj, cl_uint param, size_t size,
                               auto getter, cl_int &err) {
    std::array<T, N> v{};
    size_t take = std::min<size_t>(size, N * sizeof(T));
    err = getter(obj(), param, take, v.data(), nullptr);
    return v;
  }
};

template <typename T> struct CLInfoReader<std::vector<T>> {
  static std::vector<T> Read(auto obj, cl_uint param, size_t size, auto getter,
                             cl_int &err) {
    std::vector<T> v(size / sizeof(T));
    err = getter(obj(), param, size, v.data(), nullptr);
    return v;
  }
};

template <class Obj> struct CLGetter;

template <> struct CLGetter<cl::Device> {
  static constexpr auto fn = &clGetDeviceInfo;
};

template <> struct CLGetter<cl::Context> {
  static constexpr auto fn = &clGetContextInfo;
};

template <> struct CLGetter<cl::Platform> {
  static constexpr auto fn = &clGetPlatformInfo;
};

template <> struct CLGetter<cl::Program> {
  static constexpr auto fn = &clGetProgramInfo;
};

template <> struct CLGetter<cl::CommandQueue> {
  static constexpr auto fn = &clGetCommandQueueInfo;
};

template <> struct CLGetter<cl::Kernel> {
  static constexpr auto fn = &clGetKernelInfo;
};
} // namespace detail

template <cl_uint Info, typename Object> auto GetInfo(Object const &obj) {
  using Traits = InfoTraits<Info>;
  using ReturnType = typename Traits::type;

  cl_int err{CL_SUCCESS};

  if constexpr (requires(Object o) { o.template getInfo<Info>(nullptr); }) {
    auto value = obj.template getInfo<Info>(&err);
    CheckCLError(err, "GetInfo failed");
    return value;
  } else {
    auto getter = detail::CLGetter<Object>::fn;

    std::size_t size = 0;
    err = getter(obj(), Info, 0, nullptr, &size);
    CheckCLError(err, std::string(Traits::name) + " (query size)");

    auto value =
        detail::CLInfoReader<ReturnType>::Read(obj, Info, size, getter, err);
    CheckCLError(err, std::string(Traits::name) + " (read)");
    return value;
  }
}

template <cl_uint Info, typename Object> void PrintInfo(Object const &obj) {
  using Traits = InfoTraits<Info>;
  auto value = GetInfo<Info>(obj);
  GGEMS_INFO("OpenCL", "{}: {}", Traits::name, Traits::ToString(value));
}

// === Checking extensions
[[nodiscard]] inline bool
HasExtension(std::unordered_set<std::string> const &extensions,
             std::string_view name) {
  if (name.empty())
    return false;
  auto it = extensions.find(std::string{name});
  return it != extensions.end();
}

template <cl_uint Info, typename Object>
[[nodiscard]] inline std::unordered_set<std::string>
ExtractExtensions(Object const &obj) {
  std::string ext_str = GetInfo<Info>(obj);
  std::unordered_set<std::string> result;
  std::istringstream iss(ext_str);
  std::string token;
  while (iss >> token)
    result.insert(std::move(token));
  return result;
}
} // namespace ggems::ocl
