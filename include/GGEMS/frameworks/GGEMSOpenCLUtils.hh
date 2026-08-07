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
 * \file GGEMSOpenCLUtils.hh
 * \brief Utility helpers for OpenCL error handling, information queries and
 * extension parsing.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

/// \cond
#include <unordered_set>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCLInfoTraits.hh"

namespace ggems::ocl {
/*!
 * \brief Return the symbolic name of an OpenCL error code.
 *
 * Maps a cl_int error value to its corresponding OpenCL string token.
 *
 * \param err OpenCL error code.
 * \return String view containing the error name.
 */
[[nodiscard]] inline std::string_view GetErrorCodeName(cl_int err) noexcept {
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
  case CL_PLATFORM_NOT_FOUND_KHR:
    return "CL_PLATFORM_NOT_FOUND_KHR";
  default:
    return "CL_UNKNOWN_ERROR";
  }
}

/*!
 * \brief Retrieve a human-readable description for an OpenCL error code.
 *
 * Complements GetErrorCodeName() by providing a detailed explanation of the
 * failure reason.
 *
 * \param err OpenCL error code.
 * \return Human-readable error description.
 */
[[nodiscard]] inline std::string_view GetErrorDescription(cl_int err) noexcept {
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
 * \brief Build a long error string combining the symbolic name and description.
 *
 * \param err OpenCL error code.
 * \return Formatted string "{TOKEN} - {Description}".
 */
[[nodiscard]] inline std::string GetLongErrorString(cl_int err) noexcept {
  return std::format("{} - {}", GetErrorCodeName(err),
                     GetErrorDescription(err));
}

/*!
 * \brief Throw a GGEMS exception for a given OpenCL error.
 *
 * Utility wrapper used throughout GGEMS to convert an OpenCL error code
 * into a typed exception enriched with context and source location.
 *
 * \tparam E Exception type derived from GGEMSExceptionBase.
 * \tparam Enum Enum class representing the error code type.
 * \tparam ToStringFunc Functor producing a string from the error code.
 *
 * \param code Error code.
 * \param toString Functor converting the code into a string.
 * \param context Additional contextual message.
 * \param do_log Whether the exception should be logged.
 * \param loc Source location automatically filled by the compiler.
 */
template <typename E, typename Enum, typename ToStringFunc>
[[noreturn]] inline void
ThrowCL(Enum code, ToStringFunc toString, std::string_view context,
        bool do_log = true,
        std::source_location loc = std::source_location::current()) {
  std::string msg =
      std::format("{} (code {}): {}", context, static_cast<int>(code),
                  toString(static_cast<int>(code)));
  core::Throw<E>(msg, loc, do_log);
}

/*!
 * \brief Check an OpenCL return code and throw if not CL_SUCCESS.
 *
 * \tparam E Exception type to throw (defaults to GGEMSFatal).
 * \param err Error code returned by an OpenCL call.
 * \param context User-provided message describing the operation.
 * \param do_log Whether the error is logged.
 * \param loc Source location.
 */
template <typename E = core::GGEMSFatal>
inline void
CheckCLError(cl_int err, std::string_view context, bool do_log = true,
             std::source_location loc = std::source_location::current()) {
  if (err != CL_SUCCESS)
    ThrowCL<E>(err, GetLongErrorString, context, do_log, loc);
}

namespace detail {
/*!
 * \struct CLInfoReader
 * \brief Primary template for reading scalar OpenCL info values.
 *
 * Generic reader used for fixed-size types requested via clGet*Info functions.
 *
 * \tparam T Returned type.
 */
template <typename T> struct CLInfoReader {
  /*!
   * \brief Read the information value.
   *
   * \param obj OpenCL object wrapper.
   * \param param OpenCL parameter identifier.
   * \param size Buffer size.
   * \param getter OpenCL getter function pointer.
   * \param err Output error code.
   *
   * \return Retrieved value of type T.
   */
  static T Read(auto obj, cl_uint param, std::size_t size, auto getter,
                cl_int &err) {
    (void)size;
    T value{};
    err = getter(obj(), param, sizeof(T), &value, nullptr);
    return value;
  }
};

/*!
 * \brief Specialization for std::string info.
 */
template <> struct CLInfoReader<std::string> {
  /*!
   * \brief Read a string value from an OpenCL info query.
   *
   * \param obj    OpenCL object wrapper.
   * \param param  Info parameter identifier.
   * \param size   Expected size of the returned data.
   * \param getter Pointer to the OpenCL getter function.
   * \param err    Output variable receiving the OpenCL error code.
   *
   * \return Extracted string.
   */
  static std::string Read(auto obj, cl_uint param, size_t size, auto getter,
                          cl_int &err) {
    std::string value(size, '\0');
    err = getter(obj(), param, size, value.data(), nullptr);
    return value;
  }
};

/*!
 * \brief Specialization for std::array<T,N>.
 */
template <typename T, std::size_t N> struct CLInfoReader<std::array<T, N>> {
  /*!
   * \brief Read an array value from an OpenCL info query.
   *
   * \param obj    OpenCL object wrapper.
   * \param param  Info parameter identifier.
   * \param size   Expected size in bytes.
   * \param getter Pointer to the OpenCL getter function.
   * \param err    Output error code from the underlying OpenCL call.
   *
   * \return Extracted fixed-size array.
   */
  static std::array<T, N> Read(auto obj, cl_uint param, size_t size,
                               auto getter, cl_int &err) {
    std::array<T, N> v{};
    size_t take = std::min<size_t>(size, N * sizeof(T));
    err = getter(obj(), param, take, v.data(), nullptr);
    return v;
  }
};

/*!
 * \brief Specialization for std::vector<T>.
 */
template <typename T> struct CLInfoReader<std::vector<T>> {
  /*!
   * \brief Read a vector value from an OpenCL info query.
   *
   * \param obj    OpenCL object wrapper.
   * \param param  Info parameter identifier.
   * \param size   Expected size in bytes.
   * \param getter Pointer to the OpenCL getter function.
   * \param err    Output error code from the getter.
   *
   * \return Extracted vector of elements.
   */
  static std::vector<T> Read(auto obj, cl_uint param, size_t size, auto getter,
                             cl_int &err) {
    std::vector<T> v(size / sizeof(T));
    err = getter(obj(), param, size, v.data(), nullptr);
    return v;
  }
};

/*!
 * \struct CLGetter
 * \brief Traits mapping a cl::Object type to the corresponding clGet*Info
 * function.
 *
 * Used internally to drive the info-retrieval mechanism.
 */
template <class Obj> struct CLGetter;

/*! \brief Getter for cl::Device → clGetDeviceInfo */
template <> struct CLGetter<cl::Device> {
  /*!
   * \brief Pointer to clGetDeviceInfo used to query device information.
   */
  static constexpr auto fn = &clGetDeviceInfo;
};

/*! \brief Getter for cl::Context → clGetContextInfo */
template <> struct CLGetter<cl::Context> {
  /*!
   * \brief Pointer to clGetContextInfo used to query context information.
   */
  static constexpr auto fn = &clGetContextInfo;
};

/*! \brief Getter for cl::Platform → clGetPlatformInfo */
template <> struct CLGetter<cl::Platform> {
  /*!
   * \brief Pointer to clGetPlatformInfo used to query platform information.
   */
  static constexpr auto fn = &clGetPlatformInfo;
};

/*! \brief Getter for cl::Program → clGetProgramInfo */
template <> struct CLGetter<cl::Program> {
  /*!
   * \brief Pointer to clGetProgramInfo used to query program information.
   */
  static constexpr auto fn = &clGetProgramInfo;
};

/*! \brief Getter for cl::CommandQueue → clGetCommandQueueInfo */
template <> struct CLGetter<cl::CommandQueue> {
  /*!
   * \brief Pointer to clGetCommandQueueInfo used to query queue information.
   */
  static constexpr auto fn = &clGetCommandQueueInfo;
};

/*! \brief Getter for cl::Kernel → clGetKernelInfo */
template <> struct CLGetter<cl::Kernel> {
  /*!
   * \brief Pointer to clGetKernelInfo used to query kernel information.
   */
  static constexpr auto fn = &clGetKernelInfo;
};
} // namespace detail

/*!
 * \brief Retrieve information about a kernel argument.
 *
 * \tparam Info OpenCL kernel argument info identifier.
 * \tparam Kernel Kernel object type.
 *
 * \param k Kernel object.
 * \param index Argument index.
 * \return Retrieved info value.
 */
template <cl_uint Info, typename Kernel>
auto GetArgInfo(Kernel const &k, cl_uint index) {
  cl_int err{CL_SUCCESS};
  auto value = k.template getArgInfo<Info>(index, &err);
  GGEMS_OCL_CHECK_RECOVERABLE(err, "Get kernel argument info failed.");
  return value;
}

/*!
 * \brief Retrieve kernel work-group information.
 *
 * \tparam Info Work-group info identifier.
 * \tparam Kernel Kernel type.
 * \tparam Device Device type.
 *
 * \param k Kernel.
 * \param d Device.
 * \return Retrieved info value.
 */
template <cl_uint Info, typename Kernel, typename Device>
auto GetWorkGroupInfo(Kernel const &k, Device const &d) {
  cl_int err{CL_SUCCESS};
  auto value = k.template getWorkGroupInfo<Info>(d, &err);
  GGEMS_OCL_CHECK_RECOVERABLE(err, "Get kernel work group info failed.");
  return value;
}

/*!
 * \brief Retrieve general OpenCL information from any supported object.
 *
 * Automatically selects the correct getter function based on the object type.
 *
 * \tparam Info Parameter identifier.
 * \tparam Object OpenCL object wrapper type.
 *
 * \param obj Object instance.
 * \return Retrieved OpenCL info value.
 */
template <cl_uint Info, typename Object> auto GetInfo(Object const &obj) {
  using Traits = InfoTraits<Info>;
  using ReturnType = typename Traits::type;

  cl_int err{CL_SUCCESS};

  if constexpr (requires(Object o) { o.template getInfo<Info>(nullptr); }) {
    auto value = obj.template getInfo<Info>(&err);
    GGEMS_OCL_CHECK_RECOVERABLE(err, "GetInfo failed");
    return value;
  } else {
    auto getter = detail::CLGetter<Object>::fn;

    std::size_t size = 0;
    err = getter(obj(), Info, 0, nullptr, &size);
    GGEMS_OCL_CHECK_RECOVERABLE(err,
                                std::string(Traits::name) + " (query size)");

    auto value =
        detail::CLInfoReader<ReturnType>::Read(obj, Info, size, getter, err);
    GGEMS_OCL_CHECK_RECOVERABLE(err, std::string(Traits::name) + " (read)");
    return value;
  }
}

/*!
 * \brief Print an OpenCL info value using GGEMS logging.
 *
 * \tparam Info Parameter identifier.
 * \tparam Object Object type.
 *
 * \param obj Object instance.
 */
template <cl_uint Info, typename Object> void PrintInfo(Object const &obj) {
  using Traits = InfoTraits<Info>;

  try {
    auto value = GetInfo<Info>(obj);
    GGEMS_INFO("OpenCL", "{}: {}", Traits::name, Traits::ToString(value));
  } catch (...) {
    GGEMS_INFO("OpenCL", "{}: N/A", Traits::name);
  }
}

/*!
 * \brief Check whether an extension string set contains a given extension.
 *
 * \param extensions Set of extension names.
 * \param name Name of the extension to test.
 * \return True if present, false otherwise.
 */
[[nodiscard]] inline bool
HasExtension(std::unordered_set<std::string> const &extensions,
             std::string_view name) {
  if (name.empty()) {
    return false;
  }
  auto it = extensions.find(std::string{name});
  return it != extensions.end();
}

/*!
 * \brief Extract and split an extension string list into a set.
 *
 * \tparam Info Parameter identifier corresponding to an extension string.
 * \tparam Object OpenCL object type.
 *
 * \param obj Object from which extensions are extracted.
 * \return Set of individual extension names.
 */
template <cl_uint Info, typename Object>
[[nodiscard]] inline std::unordered_set<std::string>
ExtractExtensions(Object const &obj) {
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
