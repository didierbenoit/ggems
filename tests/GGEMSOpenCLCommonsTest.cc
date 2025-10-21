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

#include <gtest/gtest.h>
#include <map>
#include "GGEMS/frameworks/GGEMSOpenCLCommons.hh"
#include <string>

namespace {
  std::map<int, std::string> expected = {
    {-1, "CL_DEVICE_NOT_FOUND"},
    {-2, "CL_DEVICE_NOT_AVAILABLE"},
    {-3, "CL_COMPILER_NOT_AVAILABLE"},
    {-4, "CL_MEM_OBJECT_ALLOCATION_FAILURE"},
    {-5, "CL_OUT_OF_RESOURCES"},
    {-6, "CL_OUT_OF_HOST_MEMORY"},
    {-7, "CL_PROFILING_INFO_NOT_AVAILABLE"},
    {-8, "CL_MEM_COPY_OVERLAP"},
    {-9, "CL_IMAGE_FORMAT_MISMATCH"},
    {-10, "CL_IMAGE_FORMAT_NOT_SUPPORTED"},
    {-11, "CL_BUILD_PROGRAM_FAILURE"},
    {-12, "CL_MAP_FAILURE"},
    {-13, "CL_MISALIGNED_SUB_BUFFER_OFFSET"},
    {-14, "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"},
    {-15, "CL_COMPILE_PROGRAM_FAILURE"},
    {-16, "CL_LINKER_NOT_AVAILABLE"},
    {-17, "CL_LINK_PROGRAM_FAILURE"},
    {-18, "CL_DEVICE_PARTITION_FAILED"},
    {-19, "CL_KERNEL_ARG_INFO_NOT_AVAILABLE"},
    {-30, "CL_INVALID_VALUE"},
    {-31, "CL_INVALID_DEVICE_TYPE"},
    {-32, "CL_INVALID_PLATFORM"},
    {-33, "CL_INVALID_DEVICE"},
    {-34, "CL_INVALID_CONTEXT"},
    {-35, "CL_INVALID_QUEUE_PROPERTIES"},
    {-36, "CL_INVALID_COMMAND_QUEUE"},
    {-37, "CL_INVALID_HOST_PTR"},
    {-38, "CL_INVALID_MEM_OBJECT"},
    {-39, "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"},
    {-40, "CL_INVALID_IMAGE_SIZE"},
    {-41, "CL_INVALID_SAMPLER"},
    {-42, "CL_INVALID_BINARY"},
    {-43, "CL_INVALID_BUILD_OPTIONS"},
    {-44, "CL_INVALID_PROGRAM"},
    {-45, "CL_INVALID_PROGRAM_EXECUTABLE"},
    {-46, "CL_INVALID_KERNEL_NAME"},
    {-47, "CL_INVALID_KERNEL_DEFINITION"},
    {-48, "CL_INVALID_KERNEL"},
    {-49, "CL_INVALID_ARG_INDEX"},
    {-50, "CL_INVALID_ARG_VALUE"},
    {-51, "CL_INVALID_ARG_SIZE"},
    {-52, "CL_INVALID_KERNEL_ARGS"},
    {-53, "CL_INVALID_WORK_DIMENSION"},
    {-54, "CL_INVALID_WORK_GROUP_SIZE"},
    {-55, "CL_INVALID_WORK_ITEM_SIZE"},
    {-56, "CL_INVALID_GLOBAL_OFFSET"},
    {-57, "CL_INVALID_EVENT_WAIT_LIST"},
    {-58, "CL_INVALID_EVENT"},
    {-59, "CL_INVALID_OPERATION"},
    {-60, "CL_INVALID_GL_OBJECT"},
    {-61, "CL_INVALID_BUFFER_SIZE"},
    {-62, "CL_INVALID_MIP_LEVEL"},
    {-63, "CL_INVALID_GLOBAL_WORK_SIZE"},
    {-64, "CL_INVALID_PROPERTY"},
    {-65, "CL_INVALID_IMAGE_DESCRIPTOR"},
    {-66, "CL_INVALID_COMPILER_OPTIONS"},
    {-67, "CL_INVALID_LINKER_OPTIONS"},
    {-68, "CL_INVALID_DEVICE_PARTITION_COUNT"},
    {-69, "CL_INVALID_PIPE_SIZE"},
    {-70, "CL_INVALID_DEVICE_QUEUE"},
    {-1000, "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR"},
    {-1001, "CL_PLATFORM_NOT_FOUND_KHR"},
    {-1002, "CL_INVALID_D3D10_DEVICE_KHR"},
    {-1003, "CL_INVALID_D3D10_RESOURCE_KHR"},
    {-1004, "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR"},
    {-1005, "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR"},
    {-1006, "CL_INVALID_D3D11_DEVICE_KHR"},
    {-1007, "CL_INVALID_D3D11_RESOURCE_KHR"},
    {-1008, "CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR"},
    {-1009, "CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR"},
    {-1010, "CL_INVALID_D3D9_DEVICE_NV or CL_INVALID_DX9_DEVICE_INTEL"},
    {-1011, "CL_INVALID_D3D9_RESOURCE_NV or CL_INVALID_DX9_RESOURCE_INTEL"},
    {-1012, "CL_D3D9_RESOURCE_ALREADY_ACQUIRED_NV or CL_DX9_RESOURCE_ALREADY_ACQUIRED_INTEL"},
    {-1013, "CL_D3D9_RESOURCE_NOT_ACQUIRED_NV or CL_DX9_RESOURCE_NOT_ACQUIRED_INTEL"},
    {-1092, "CL_EGL_RESOURCE_NOT_ACQUIRED_KHR"},
    {-1093, "CL_INVALID_EGL_OBJECT_KHR"},
    {-1094, "CL_INVALID_ACCELERATOR_INTEL"},
    {-1095, "CL_INVALID_ACCELERATOR_TYPE_INTEL"},
    {-1096, "CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL"},
    {-1097, "CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL"},
    {-1098, "CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL"},
    {-1099, "CL_INVALID_VA_API_MEDIA_SURFACE_INTEL"},
    {-1100, "CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL"},
    {-1101, "CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL"},
    {-9999, "NVidia"}
  };
}

TEST(GGEMSOpenCLCommonsTest, ErrorStringMessage) {
  for (auto& [error_code, expected_text] : ::expected) {
    EXPECT_TRUE(ggocl::GetErrorString(error_code).find(expected_text) != std::string::npos);
    EXPECT_FALSE(ggocl::GetErrorString(error_code).empty());
  }
}
