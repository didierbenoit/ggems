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
 * \file GGEMSOpenCLInfoTraits.hh
 * \brief Declaration of the GGEMSOpenCLPlatform class for OpenCL 3.0 platform
 * abstraction.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-11-09
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <CL/opencl.hpp>
#include <string>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"

namespace ggems::ocl {
template <cl_uint Info> struct InfoTraits;

// === Platform ===
template <> struct InfoTraits<CL_PLATFORM_VENDOR> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_VENDOR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_NAME> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_NAME";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_PROFILE> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_PROFILE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_EXTENSIONS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_NUMERIC_VERSION> {
  using type = cl_version;
  static constexpr std::string_view name = "CL_PLATFORM_NUMERIC_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClVersionToString(v); }
};

template <> struct InfoTraits<CL_PLATFORM_HOST_TIMER_RESOLUTION> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_PLATFORM_HOST_TIMER_RESOLUTION";
  static constexpr std::string_view unit = "ns";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name =
      "CL_PLATFORM_EXTENSIONS_WITH_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return ClNameVersionToString(v);
  }
};

// === Device ===
template <> struct InfoTraits<CL_DEVICE_EXTENSIONS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_EXTENSIONS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_IL_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_IL_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_SPIR_VERSIONS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_SPIR_VERSIONS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_NAME> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_NAME";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_VENDOR> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_VENDOR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DRIVER_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DRIVER_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_PROFILE> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_PROFILE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_ILS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_ILS_WITH_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_ALL_VERSIONS> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_ALL_VERSIONS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR> {
  using type = cl_version_khr;
  static constexpr std::string_view name =
      "CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClVersionToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_FEATURES> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_FEATURES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT> {
  using type = cl_version;
  static constexpr std::string_view name =
      "CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClVersionToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_NUMERIC_VERSION> {
  using type = cl_version;
  static constexpr std::string_view name = "CL_DEVICE_NUMERIC_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClVersionToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_VENDOR_ID> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_VENDOR_ID";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_TYPE> {
  using type = cl_device_type;
  static constexpr std::string_view name = "CL_DEVICE_TYPE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return DeviceTypeToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_COMPUTE_UNITS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_COMPUTE_UNITS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_CLOCK_FREQUENCY> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_CLOCK_FREQUENCY";
  static constexpr std::string_view unit = "MHz";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WORK_GROUP_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_GROUP_SIZE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_SIZES> {
  using type = std::vector<std::size_t>;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_ITEM_SIZES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) { return SizeToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE> {
  using type = std::size_t;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_WIDTH> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE2D_MAX_WIDTH";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_HEIGHT> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE2D_MAX_HEIGHT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_WIDTH> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_WIDTH";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_HEIGHT> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_HEIGHT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_DEPTH> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_DEPTH";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_BUFFER_SIZE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_ARRAY_SIZE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_SUPPORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_READ_IMAGE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_READ_IMAGE_ARGS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WRITE_IMAGE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WRITE_IMAGE_ARGS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_PITCH_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_PITCH_ALIGNMENT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_SAMPLERS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_SAMPLERS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE> {
  using type = cl_device_mem_cache_type;
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return CacheTypeToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_LOCAL_MEM_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_TYPE> {
  using type = cl_device_local_mem_type;
  static constexpr std::string_view name = "CL_DEVICE_LOCAL_MEM_TYPE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return LocalMemTypeToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_MEM_ALLOC_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_MAX_MEM_ALLOC_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_CONSTANT_ARGS";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MEM_BASE_ADDR_ALIGN> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MEM_BASE_ADDR_ALIGN";
  static constexpr std::string_view unit = " bits";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_HOST_UNIFIED_MEMORY> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_HOST_UNIFIED_MEMORY";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES> {
  using type = cl_command_queue_properties;
  static constexpr std::string_view name = "CL_DEVICE_QUEUE_ON_HOST_PROPERTIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return QueuePropertiesToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES> {
  using type = cl_command_queue_properties;
  static constexpr std::string_view name =
      "CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return QueuePropertiesToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_QUEUES> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_ON_DEVICE_QUEUES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_EVENTS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_ON_DEVICE_EVENTS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_SVM_CAPABILITIES> {
  using type = cl_device_svm_capabilities;
  static constexpr std::string_view name = "CL_DEVICE_SVM_CAPABILITIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return SVMCapabilitiesToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES> {
  using type = cl_device_atomic_capabilities;
  static constexpr std::string_view name =
      "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return AtomicCapabilitiesToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES> {
  using type = cl_device_atomic_capabilities;
  static constexpr std::string_view name =
      "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return AtomicCapabilitiesToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_NUM_SUB_GROUPS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_NUM_SUB_GROUPS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <>
struct InfoTraits<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_EXECUTION_CAPABILITIES> {
  using type = cl_device_exec_capabilities;
  static constexpr std::string_view name = "CL_DEVICE_EXECUTION_CAPABILITIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ExecCapabilitiesToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <>
struct InfoTraits<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES> {
  using type = cl_device_device_enqueue_capabilities;
  static constexpr std::string_view name =
      "CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) {
    return DeviceEnqueueCapabilitiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_MAX_SUB_DEVICES> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_MAX_SUB_DEVICES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_PROPERTIES> {
  using type = std::vector<cl_device_partition_property>;
  static constexpr std::string_view name = "CL_DEVICE_PARTITION_PROPERTIES";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return PartitionPropertiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_AFFINITY_DOMAIN> {
  using type = cl_device_affinity_domain;
  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_AFFINITY_DOMAIN";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return AffinityDomainToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_TYPE> {
  using type = std::vector<cl_device_partition_property>;
  static constexpr std::string_view name = "CL_DEVICE_PARTITION_TYPE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return PartitionPropertiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_PARAMETER_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_PARAMETER_SIZE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_EXTENSIONS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_EXTENSIONS_WITH_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED> {
  using type = std::string;
  static constexpr std::string_view name =
      "CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_BUILT_IN_KERNELS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return v; }
};

template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name =
      "CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_ADDRESS_BITS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_ADDRESS_BITS";
  static constexpr std::string_view unit = " bits";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PROFILING_TIMER_RESOLUTION> {
  using type = std::size_t;
  static constexpr std::string_view name =
      "CL_DEVICE_PROFILING_TIMER_RESOLUTION";
  static constexpr std::string_view unit = " ns";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_COMPILER_AVAILABLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_COMPILER_AVAILABLE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_LINKER_AVAILABLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_LINKER_AVAILABLE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_AVAILABLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_AVAILABLE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_ENDIAN_LITTLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_ENDIAN_LITTLE";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_ERROR_CORRECTION_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_ERROR_CORRECTION_SUPPORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_PRINTF_BUFFER_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_PRINTF_BUFFER_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_PIPE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_PIPE_ARGS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_PACKET_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_PIPE_MAX_PACKET_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PIPE_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_PIPE_SUPPORT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE";
  static constexpr std::string_view unit = " bytes";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_UUID_KHR> {
  using type = std::array<cl_uchar, CL_UUID_SIZE_KHR>;
  static constexpr std::string_view name = "CL_DEVICE_UUID_KHR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) { return UUIDToString(v); }
};

template <> struct InfoTraits<CL_DRIVER_UUID_KHR> {
  using type = std::array<cl_uchar, CL_UUID_SIZE_KHR>;
  static constexpr std::string_view name = "CL_DRIVER_UUID_KHR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) { return UUIDToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_LUID_VALID_KHR> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_LUID_VALID_KHR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_LUID_KHR> {
  using type = std::array<cl_uchar, CL_LUID_SIZE_KHR>;
  static constexpr std::string_view name = "CL_DEVICE_LUID_KHR";
  static constexpr std::string_view unit = "";
  static std::string ToString(type const &v) { return LUIDToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_HALF_FP_CONFIG> {
  using type = cl_device_fp_config;
  static constexpr std::string_view name = "CL_DEVICE_HALF_FP_CONFIG";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return FPConfigToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_SINGLE_FP_CONFIG> {
  using type = cl_device_fp_config;
  static constexpr std::string_view name = "CL_DEVICE_SINGLE_FP_CONFIG";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return FPConfigToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_DOUBLE_FP_CONFIG> {
  using type = cl_device_fp_config;
  static constexpr std::string_view name = "CL_DEVICE_DOUBLE_FP_CONFIG";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return FPConfigToString(v); }
};

template <> struct InfoTraits<CL_DEVICE_REFERENCE_COUNT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_REFERENCE_COUNT";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return std::to_string(v); }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_INTEROP_USER_SYNC";
  static constexpr std::string_view unit = "";
  static std::string ToString(type v) { return ClBoolToString(v); }
};
} // namespace ggems::ocl
