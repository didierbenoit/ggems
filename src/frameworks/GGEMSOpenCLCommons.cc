
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
 * \file GGEMSOpenCLCommons.cc
 * \brief Definitions for error translation, failure reporting and termination.
 */

/// \cond
#include <iostream>
#include <sstream>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLCommons.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogger.hh"

void ggocl::Failure(std::string_view filename, std::string_view function_name, int line, cl_int error_code) {
  if (error_code == CL_SUCCESS) return;
  const std::string err = GetErrorString(error_code);
  throw GGEMSException(filename, function_name, line, err);
}

[[noreturn]] void ggocl::TerminateHandler() noexcept {
  try {
    gglog::err() << "[GGEMS] Uncaught exception — terminating now." << gglog::endl;
  } catch (...) {
    // Swallow all
  }
  std::abort();
}

std::string ggocl::GetErrorString(cl_int error) {
  std::ostringstream oss;
  // Group common ranges for clarity:
  //  -1..-19  : Run-time & JIT
  // -30..-70  : Compile-time & API misuse
  // -1000..   : Extensions / vendor
  switch (error) {
    case CL_DEVICE_NOT_FOUND:                        oss << "CL_DEVICE_NOT_FOUND: No matching devices for the requested type."; break;
    case CL_DEVICE_NOT_AVAILABLE:                    oss << "CL_DEVICE_NOT_AVAILABLE: Device found but currently unavailable."; break;
    case CL_COMPILER_NOT_AVAILABLE:                  oss << "CL_COMPILER_NOT_AVAILABLE: Program built from source but no compiler available."; break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:           oss << "CL_MEM_OBJECT_ALLOCATION_FAILURE: Failed to allocate memory for an OpenCL object."; break;
    case CL_OUT_OF_RESOURCES:                        oss << "CL_OUT_OF_RESOURCES: Device-side resource allocation failure."; break;
    case CL_OUT_OF_HOST_MEMORY:                      oss << "CL_OUT_OF_HOST_MEMORY: Host-side resource allocation failure."; break;
    case CL_PROFILING_INFO_NOT_AVAILABLE:            oss << "CL_PROFILING_INFO_NOT_AVAILABLE: Queue lacks CL_QUEUE_PROFILING_ENABLE or event not complete."; break;
    case CL_MEM_COPY_OVERLAP:                        oss << "CL_MEM_COPY_OVERLAP: Source and destination buffer regions overlap."; break;
    case CL_IMAGE_FORMAT_MISMATCH:                   oss << "CL_IMAGE_FORMAT_MISMATCH: Incompatible image formats."; break;
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:              oss << "CL_IMAGE_FORMAT_NOT_SUPPORTED: Unsupported image format."; break;
    case CL_BUILD_PROGRAM_FAILURE:                   oss << "CL_BUILD_PROGRAM_FAILURE: Program build failed."; break;
    case CL_MAP_FAILURE:                             oss << "CL_MAP_FAILURE: Mapping the requested region failed."; break;
    case CL_MISALIGNED_SUB_BUFFER_OFFSET:            oss << "CL_MISALIGNED_SUB_BUFFER_OFFSET: Sub-buffer base offset misaligned."; break;
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: oss << "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: One or more waited events have negative status."; break;
    case CL_COMPILE_PROGRAM_FAILURE:                 oss << "CL_COMPILE_PROGRAM_FAILURE: Program compilation failed."; break;
    case CL_LINKER_NOT_AVAILABLE:                    oss << "CL_LINKER_NOT_AVAILABLE: Linker unavailable on device."; break;
    case CL_LINK_PROGRAM_FAILURE:                    oss << "CL_LINK_PROGRAM_FAILURE: Linking binaries and/or libraries failed."; break;
    case CL_DEVICE_PARTITION_FAILED:                 oss << "CL_DEVICE_PARTITION_FAILED: Device could not be further partitioned."; break;
    case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:           oss << "CL_KERNEL_ARG_INFO_NOT_AVAILABLE: Argument information is not available for the kernel."; break;

    case CL_INVALID_VALUE:                           oss << "CL_INVALID_VALUE: One or more arguments have invalid values."; break;
    case CL_INVALID_DEVICE_TYPE:                     oss << "CL_INVALID_DEVICE_TYPE: Invalid device type."; break;
    case CL_INVALID_PLATFORM:                        oss << "CL_INVALID_PLATFORM: Invalid platform parameter."; break;
    case CL_INVALID_DEVICE:                          oss << "CL_INVALID_DEVICE: Invalid or mismatched device."; break;
    case CL_INVALID_CONTEXT:                         oss << "CL_INVALID_CONTEXT: Invalid context object."; break;
    case CL_INVALID_QUEUE_PROPERTIES:                oss << "CL_INVALID_QUEUE_PROPERTIES: Unsupported queue properties for the device."; break;
    case CL_INVALID_COMMAND_QUEUE:                   oss << "CL_INVALID_COMMAND_QUEUE: Invalid command queue."; break;
    case CL_INVALID_HOST_PTR:                        oss << "CL_INVALID_HOST_PTR: Inconsistent use of host pointer and flags."; break;
    case CL_INVALID_MEM_OBJECT:                      oss << "CL_INVALID_MEM_OBJECT: Invalid memory object."; break;
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:         oss << "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: Invalid GL/DX image format mapping."; break;
    case CL_INVALID_IMAGE_SIZE:                      oss << "CL_INVALID_IMAGE_SIZE: Unsupported image dimensions for the device."; break;
    case CL_INVALID_SAMPLER:                         oss << "CL_INVALID_SAMPLER: Invalid sampler object."; break;
    case CL_INVALID_BINARY:                          oss << "CL_INVALID_BINARY: Invalid program binary for the device list."; break;
    case CL_INVALID_BUILD_OPTIONS:                   oss << "CL_INVALID_BUILD_OPTIONS: Invalid build options string."; break;
    case CL_INVALID_PROGRAM:                         oss << "CL_INVALID_PROGRAM: Invalid program object."; break;
    case CL_INVALID_PROGRAM_EXECUTABLE:              oss << "CL_INVALID_PROGRAM_EXECUTABLE: No successfully built executable for the device."; break;
    case CL_INVALID_KERNEL_NAME:                     oss << "CL_INVALID_KERNEL_NAME: Kernel name not found in program."; break;
    case CL_INVALID_KERNEL_DEFINITION:               oss << "CL_INVALID_KERNEL_DEFINITION: Kernel definition mismatches across devices."; break;
    case CL_INVALID_KERNEL:                          oss << "CL_INVALID_KERNEL: Invalid kernel object."; break;
    case CL_INVALID_ARG_INDEX:                       oss << "CL_INVALID_ARG_INDEX: Argument index out of range."; break;
    case CL_INVALID_ARG_VALUE:                       oss << "CL_INVALID_ARG_VALUE: Invalid argument value."; break;
    case CL_INVALID_ARG_SIZE:                        oss << "CL_INVALID_ARG_SIZE: Argument size mismatches its type."; break;
    case CL_INVALID_KERNEL_ARGS:                     oss << "CL_INVALID_KERNEL_ARGS: Kernel arguments not fully specified."; break;
    case CL_INVALID_WORK_DIMENSION:                  oss << "CL_INVALID_WORK_DIMENSION: work_dim must be in [1..3]."; break;
    case CL_INVALID_WORK_GROUP_SIZE:                 oss << "CL_INVALID_WORK_GROUP_SIZE: Local size incompatible with kernel/device constraints."; break;
    case CL_INVALID_WORK_ITEM_SIZE:                  oss << "CL_INVALID_WORK_ITEM_SIZE: Work-items per dimension exceed device limits."; break;
    case CL_INVALID_GLOBAL_OFFSET:                   oss << "CL_INVALID_GLOBAL_OFFSET: Offset+size exceeds size_t range for the device."; break;
    case CL_INVALID_EVENT_WAIT_LIST:                 oss << "CL_INVALID_EVENT_WAIT_LIST: Inconsistent event wait list."; break;
    case CL_INVALID_EVENT:                           oss << "CL_INVALID_EVENT: Invalid event object."; break;
    case CL_INVALID_OPERATION:                       oss << "CL_INVALID_OPERATION: Operation not permitted in current state."; break;
    case CL_INVALID_GL_OBJECT:                       oss << "CL_INVALID_GL_OBJECT: Invalid or undefined GL object in interop."; break;
    case CL_INVALID_BUFFER_SIZE:                     oss << "CL_INVALID_BUFFER_SIZE: Size is zero or exceeds max alloc."; break;
    case CL_INVALID_MIP_LEVEL:                       oss << "CL_INVALID_MIP_LEVEL: Non-zero mip level unsupported for this interop."; break;
    case CL_INVALID_GLOBAL_WORK_SIZE:                oss << "CL_INVALID_GLOBAL_WORK_SIZE: Global work size is null/zero/out of range."; break;
    case CL_INVALID_PROPERTY:                        oss << "CL_INVALID_PROPERTY: Invalid or unsupported property value."; break;
    case CL_INVALID_IMAGE_DESCRIPTOR:                oss << "CL_INVALID_IMAGE_DESCRIPTOR: Invalid image descriptor values."; break;
    case CL_INVALID_COMPILER_OPTIONS:                oss << "CL_INVALID_COMPILER_OPTIONS: Compiler options string invalid."; break;
    case CL_INVALID_LINKER_OPTIONS:                  oss << "CL_INVALID_LINKER_OPTIONS: Linker options string invalid."; break;
    case CL_INVALID_DEVICE_PARTITION_COUNT:          oss << "CL_INVALID_DEVICE_PARTITION_COUNT: Sub-device counts exceed device capacity."; break;

    // Extensions / vendor buckets (selected)
    case CL_PLATFORM_NOT_FOUND_KHR:                  oss << "CL_PLATFORM_NOT_FOUND_KHR: No valid ICDs found."; break;
    default:                                         oss << "Unknown OpenCL error (" << error << ")"; break;
  }

  return oss.str();
}
