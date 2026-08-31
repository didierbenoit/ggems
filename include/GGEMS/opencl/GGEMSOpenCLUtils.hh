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
 * \brief Provides OpenCL error handling and generic information-query helpers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>
/// \endcond

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLInfoTraits.hh"

namespace ggems::ocl {

/*!
 * \brief Returns the symbolic name of an OpenCL error code.
 *
 * \param[in] err OpenCL status or error code.
 * \return Symbolic OpenCL error name, or CL_UNKNOWN_ERROR.
 */
[[nodiscard]] inline auto GetErrorCodeName(cl_int err) noexcept
    -> std::string_view {
  switch (err) {
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
    return "CL_INVALID_EVENT";
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
  case CL_PLATFORM_NOT_FOUND_KHR:
    return "CL_PLATFORM_NOT_FOUND_KHR";
  default:
    return "CL_UNKNOWN_ERROR";
  }
}

/*!
 * \brief Returns a human-readable description of an OpenCL error code.
 *
 * \param[in] err OpenCL status or error code.
 * \return Human-readable OpenCL error description.
 */
[[nodiscard]] inline auto GetErrorDescription(cl_int err) noexcept
    -> std::string_view {
  switch (err) {
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
  case CL_PLATFORM_NOT_FOUND_KHR:
    return "No valid ICDs found.";
  default:
    return "Unknown OpenCL error.";
  }
}

/*!
 * \brief Builds a combined OpenCL error name and description.
 *
 * \param[in] err OpenCL status or error code.
 * \return Combined symbolic name and description.
 */
[[nodiscard]] inline auto GetLongErrorString(cl_int err) -> std::string {
  return std::format("{} - {}", GetErrorCodeName(err),
                     GetErrorDescription(err));
}

/*!
 * \brief Throws a GGEMS exception when an OpenCL call fails.
 *
 * \tparam ExceptionType GGEMS exception type raised on failure.
 * \param[in] err OpenCL status code to check.
 * \param[in] context Diagnostic context prepended to the error message.
 * \param[in] loc Source location associated with the failure.
 */
template <core::GGEMSExceptionType ExceptionType = core::GGEMSFatal>
inline auto
CheckCLError(cl_int err, std::string_view context,
             std::source_location loc = std::source_location::current())
    -> void {
  if (err != CL_SUCCESS) {
    throw ExceptionType(std::format("{} (code {}): {}", context,
                                    static_cast<int>(err),
                                    GetLongErrorString(err)),
                        loc);
  }
}

/// \cond
namespace detail {
template <typename T> struct CLInfoReader {
  static auto Read(auto const &obj, cl_uint param, std::size_t size,
                   auto getter, cl_int &err) -> T {
    (void)size;
    T value{};
    err = getter(obj(), param, sizeof(T), &value, nullptr);
    return value;
  }
};

template <> struct CLInfoReader<std::string> {
  static auto Read(auto const &obj, cl_uint param, size_t size, auto getter,
                   cl_int &err) -> std::string {
    std::string value(size, '\0');
    err = getter(obj(), param, size, value.data(), nullptr);
    if (err == CL_SUCCESS && !value.empty() && value.back() == '\0') {
      value.pop_back();
    }
    return value;
  }
};

template <typename T, std::size_t N> struct CLInfoReader<std::array<T, N>> {
  static auto Read(auto const &obj, cl_uint param, size_t size, auto getter,
                   cl_int &err) -> std::array<T, N> {
    std::array<T, N> values{};
    size_t take = std::min<size_t>(size, N * sizeof(T));
    err = getter(obj(), param, take, values.data(), nullptr);
    return values;
  }
};

template <typename T> struct CLInfoReader<std::vector<T>> {
  static auto Read(auto const &obj, cl_uint param, size_t size, auto getter,
                   cl_int &err) -> std::vector<T> {
    std::vector<T> values(size / sizeof(T));
    err = getter(obj(), param, size, values.data(), nullptr);
    return values;
  }
};

template <class Obj> struct CLGetter;

template <> struct CLGetter<cl::Device> {
  static constexpr auto function = &clGetDeviceInfo;
};

template <> struct CLGetter<cl::Context> {
  static constexpr auto function = &clGetContextInfo;
};

template <> struct CLGetter<cl::Platform> {
  static constexpr auto function = &clGetPlatformInfo;
};

template <> struct CLGetter<cl::Program> {
  static constexpr auto function = &clGetProgramInfo;
};

template <> struct CLGetter<cl::CommandQueue> {
  static constexpr auto function = &clGetCommandQueueInfo;
};

template <> struct CLGetter<cl::Kernel> {
  static constexpr auto function = &clGetKernelInfo;
};
} // namespace detail
/// \endcond

/*!
 * \brief Returns typed OpenCL kernel-argument information.
 *
 * \tparam Info OpenCL kernel-argument information selector.
 * \tparam Kernel Kernel wrapper type.
 * \param[in] kernel Kernel queried for argument information.
 * \param[in] index Kernel argument index.
 * \return Requested kernel-argument information value.
 */
template <cl_uint Info, typename Kernel>
auto GetArgInfo(Kernel const &kernel, cl_uint index) {
  cl_int err{CL_SUCCESS};
  auto value = kernel.template getArgInfo<Info>(index, &err);
  CheckCLError<core::GGEMSRecoverable>(err, "Get kernel argument info failed.");
  return value;
}

/*!
 * \brief Returns typed OpenCL kernel work-group information.
 *
 * \tparam Info OpenCL work-group information selector.
 * \tparam Kernel Kernel wrapper type.
 * \tparam Device Device wrapper type.
 * \param[in] kernel Kernel queried for work-group information.
 * \param[in] device Device associated with the work-group query.
 * \return Requested work-group information value.
 */
template <cl_uint Info, typename Kernel, typename Device>
auto GetWorkGroupInfo(Kernel const &kernel, Device const &device) {
  cl_int err{CL_SUCCESS};
  auto value = kernel.template getWorkGroupInfo<Info>(device, &err);
  CheckCLError<core::GGEMSRecoverable>(err,
                                       "Get kernel work group info failed.");
  return value;
}

/*!
 * \brief Returns typed OpenCL information for a supported object.
 *
 * \tparam Info OpenCL information selector.
 * \tparam Object OpenCL C++ wrapper type.
 * \param[in] obj OpenCL object to query.
 * \return Requested information value.
 */
template <cl_uint Info, typename Object> auto GetInfo(Object const &obj) {
  using Traits = InfoTraits<Info>;
  using ReturnType = typename Traits::type;

  cl_int err{CL_SUCCESS};

  if constexpr (requires(Object const &object) {
                  object.template getInfo<Info>(nullptr);
                }) {
    auto value = obj.template getInfo<Info>(&err);
    CheckCLError<core::GGEMSRecoverable>(err, "GetInfo failed");
    return value;
  } else {
    auto getter = detail::CLGetter<Object>::function;

    std::size_t size = 0;
    err = getter(obj(), Info, 0, nullptr, &size);
    CheckCLError<core::GGEMSRecoverable>(err, std::string(Traits::name) +
                                                  " (query size)");

    auto value =
        detail::CLInfoReader<ReturnType>::Read(obj, Info, size, getter, err);
    CheckCLError<core::GGEMSRecoverable>(err,
                                         std::string(Traits::name) + " (read)");
    return value;
  }
}

/*!
 * \brief Queries and logs an OpenCL information value.
 *
 * \tparam Info OpenCL information selector.
 * \tparam Object OpenCL C++ wrapper type.
 * \param[in] obj OpenCL object to query.
 */
template <cl_uint Info, typename Object>
auto PrintInfo(Object const &obj) -> void {
  using Traits = InfoTraits<Info>;

  try {
    auto value = GetInfo<Info>(obj);
    GGEMS_INFO("OpenCL", "{}: {}", Traits::name, Traits::ToString(value));
  } catch (...) {
    GGEMS_INFO("OpenCL", "{}: N/A", Traits::name);
  }
}

/*!
 * \brief Checks whether an extension name is present in a parsed extension set.
 *
 * \param[in] extensions Parsed OpenCL extension names.
 * \param[in] name Extension name to search for.
 * \return True if the extension is present, false otherwise.
 */
[[nodiscard]] inline auto
HasExtension(std::unordered_set<std::string> const &extensions,
             std::string_view name) -> bool {
  if (name.empty()) {
    return false;
  }
  auto it_extension = extensions.find(std::string{name});
  return it_extension != extensions.end();
}

/*!
 * \brief Parses a space-separated OpenCL extension string into a set.
 *
 * \tparam Info OpenCL information selector that returns the extension string.
 * \tparam Object OpenCL object type.
 * \param[in] obj OpenCL object to query.
 * \return Set of parsed extension names.
 */
template <cl_uint Info, typename Object>
[[nodiscard]] auto ExtractExtensions(Object const &obj)
    -> std::unordered_set<std::string> {
  std::string ext_str = GetInfo<Info>(obj);
  std::unordered_set<std::string> result;
  std::istringstream iss(ext_str);
  std::string token;
  while (iss >> token) {
    result.insert(std::move(token));
  }
  return result;
}
} // namespace ggems::ocl
