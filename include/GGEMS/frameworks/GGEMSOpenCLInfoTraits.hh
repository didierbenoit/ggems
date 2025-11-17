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
#include <CL/cl.h>
#include <CL/opencl.hpp>
#include <string>
/// \endcond

#include "GGEMS/core/GGEMSCoreUtils.hh"
#include "GGEMS/core/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"

namespace ggems::ocl {
using namespace ggems::units;

template <cl_uint Info> struct InfoTraits;

// === Platform ===
template <> struct InfoTraits<CL_PLATFORM_VENDOR> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_VENDOR";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_NAME> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_NAME";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_VERSION";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_PROFILE> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_PROFILE";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_PLATFORM_EXTENSIONS";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_PLATFORM_NUMERIC_VERSION> {
  using type = cl_version;
  static constexpr std::string_view name = "CL_PLATFORM_NUMERIC_VERSION";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

template <> struct InfoTraits<CL_PLATFORM_HOST_TIMER_RESOLUTION> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_PLATFORM_HOST_TIMER_RESOLUTION";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Time t = Time{static_cast<std::uint64_t>(v) * 1000ull};
    return HumanReadable(t);
  }
};

template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name =
      "CL_PLATFORM_EXTENSIONS_WITH_VERSION";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

// === Device ===
template <> struct InfoTraits<CL_DEVICE_EXTENSIONS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_EXTENSIONS";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_IL_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_IL_VERSION";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_SPIR_VERSIONS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_SPIR_VERSIONS";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_NAME> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_NAME";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_VENDOR> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_VENDOR";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_VERSION";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DRIVER_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DRIVER_VERSION";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_PROFILE> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_PROFILE";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_VERSION> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_VERSION";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_ILS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_ILS_WITH_VERSION";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_ALL_VERSIONS> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_ALL_VERSIONS";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR> {
  using type = cl_version_khr;
  static constexpr std::string_view name =
      "CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_OPENCL_C_FEATURES> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_FEATURES";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT> {
  using type = cl_version;
  static constexpr std::string_view name =
      "CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NUMERIC_VERSION> {
  using type = cl_version;
  static constexpr std::string_view name = "CL_DEVICE_NUMERIC_VERSION";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_VENDOR_ID> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_VENDOR_ID";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return VendorIdToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_TYPE> {
  using type = cl_device_type;
  static constexpr std::string_view name = "CL_DEVICE_TYPE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DeviceTypeToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_COMPUTE_UNITS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_COMPUTE_UNITS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_CLOCK_FREQUENCY> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_CLOCK_FREQUENCY";

  [[nodiscard]] static std::string ToString(type v) noexcept {
    if (v != 0) {
      Frequency f = Frequency{static_cast<uint64_t>(v) * 1'000'000ull};
      return HumanReadable(f);
    }

    auto const fallback = ggems::core::GetCPUFrequencyMHz();

    if (fallback.has_value()) {
      Frequency f = Frequency{static_cast<std::uint64_t>(fallback.value()) *
                              1'000'000ull};
      return HumanReadable(f);
    } else {
      return "";
    }
  };
};

template <> struct InfoTraits<CL_DEVICE_MAX_WORK_GROUP_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_GROUP_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_SIZES> {
  using type = std::vector<std::size_t>;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_ITEM_SIZES";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return SizeToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE> {
  using type = std::size_t;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_WIDTH> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE2D_MAX_WIDTH";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_HEIGHT> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE2D_MAX_HEIGHT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_WIDTH> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_WIDTH";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_HEIGHT> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_HEIGHT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_DEPTH> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_DEPTH";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_BUFFER_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_ARRAY_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_SUPPORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_READ_IMAGE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_READ_IMAGE_ARGS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WRITE_IMAGE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_WRITE_IMAGE_ARGS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_PITCH_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_PITCH_ALIGNMENT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_SAMPLERS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_SAMPLERS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_SIZE";
  static constexpr std::string_view unit = " bytes";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE> {
  using type = cl_device_mem_cache_type;
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return CacheTypeToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_LOCAL_MEM_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_TYPE> {
  using type = cl_device_local_mem_type;
  static constexpr std::string_view name = "CL_DEVICE_LOCAL_MEM_TYPE";
  static constexpr std::string_view unit = "";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return LocalMemTypeToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_MEM_ALLOC_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_MAX_MEM_ALLOC_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE> {
  using type = cl_ulong;
  static constexpr std::string_view name = "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_CONSTANT_ARGS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_MEM_BASE_ADDR_ALIGN> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MEM_BASE_ADDR_ALIGN";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_HOST_UNIFIED_MEMORY> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_HOST_UNIFIED_MEMORY";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES> {
  using type = cl_command_queue_properties;
  static constexpr std::string_view name = "CL_DEVICE_QUEUE_ON_HOST_PROPERTIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES> {
  using type = cl_command_queue_properties;
  static constexpr std::string_view name =
      "CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE";
  static constexpr std::string_view unit = " bytes";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_QUEUES> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_ON_DEVICE_QUEUES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_EVENTS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_ON_DEVICE_EVENTS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_SVM_CAPABILITIES> {
  using type = cl_device_svm_capabilities;
  static constexpr std::string_view name = "CL_DEVICE_SVM_CAPABILITIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return SVMCapabilitiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES> {
  using type = cl_device_atomic_capabilities;
  static constexpr std::string_view name =
      "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return AtomicCapabilitiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES> {
  using type = cl_device_atomic_capabilities;
  static constexpr std::string_view name =
      "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return AtomicCapabilitiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_NUM_SUB_GROUPS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_NUM_SUB_GROUPS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <>
struct InfoTraits<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_EXECUTION_CAPABILITIES> {
  using type = cl_device_exec_capabilities;
  static constexpr std::string_view name = "CL_DEVICE_EXECUTION_CAPABILITIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ExecCapabilitiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <>
struct InfoTraits<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES> {
  using type = cl_device_device_enqueue_capabilities;
  static constexpr std::string_view name =
      "CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DeviceEnqueueCapabilitiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_MAX_SUB_DEVICES> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_MAX_SUB_DEVICES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_PROPERTIES> {
  using type = std::vector<cl_device_partition_property>;
  static constexpr std::string_view name = "CL_DEVICE_PARTITION_PROPERTIES";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return PartitionPropertiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_AFFINITY_DOMAIN> {
  using type = cl_device_affinity_domain;
  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_AFFINITY_DOMAIN";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return AffinityDomainToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PARTITION_TYPE> {
  using type = std::vector<cl_device_partition_property>;
  static constexpr std::string_view name = "CL_DEVICE_PARTITION_TYPE";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return PartitionPropertiesToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_PARAMETER_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_PARAMETER_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_EXTENSIONS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name = "CL_DEVICE_EXTENSIONS_WITH_VERSION";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED> {
  using type = std::string;
  static constexpr std::string_view name =
      "CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS> {
  using type = std::string;
  static constexpr std::string_view name = "CL_DEVICE_BUILT_IN_KERNELS";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION> {
  using type = std::vector<cl_name_version>;
  static constexpr std::string_view name =
      "CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT";
  static constexpr std::string_view unit = " bytes";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_ADDRESS_BITS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_ADDRESS_BITS";
  static constexpr std::string_view unit = " bits";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_PROFILING_TIMER_RESOLUTION> {
  using type = std::size_t;
  static constexpr std::string_view name =
      "CL_DEVICE_PROFILING_TIMER_RESOLUTION";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Time t = Time{static_cast<uint64_t>(v) * 1000ull};
    return HumanReadable(t);
  }
};

template <> struct InfoTraits<CL_DEVICE_COMPILER_AVAILABLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_COMPILER_AVAILABLE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_LINKER_AVAILABLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_LINKER_AVAILABLE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_AVAILABLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_AVAILABLE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_ENDIAN_LITTLE> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_ENDIAN_LITTLE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_ERROR_CORRECTION_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_ERROR_CORRECTION_SUPPORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PRINTF_BUFFER_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_PRINTF_BUFFER_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_PIPE_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_MAX_PIPE_ARGS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS> {
  using type = cl_uint;
  static constexpr std::string_view name =
      "CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_PACKET_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_PIPE_MAX_PACKET_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_PIPE_SUPPORT> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_PIPE_SUPPORT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name = "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE> {
  using type = std::size_t;
  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B);
  }
};

template <> struct InfoTraits<CL_DEVICE_UUID_KHR> {
  using type = std::array<cl_uchar, CL_UUID_SIZE_KHR>;
  static constexpr std::string_view name = "CL_DEVICE_UUID_KHR";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return UUIDToString(v);
  }
};

template <> struct InfoTraits<CL_DRIVER_UUID_KHR> {
  using type = std::array<cl_uchar, CL_UUID_SIZE_KHR>;
  static constexpr std::string_view name = "CL_DRIVER_UUID_KHR";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return UUIDToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_LUID_VALID_KHR> {
  using type = cl_bool;
  static constexpr std::string_view name = "CL_DEVICE_LUID_VALID_KHR";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_LUID_KHR> {
  using type = std::array<cl_uchar, CL_LUID_SIZE_KHR>;
  static constexpr std::string_view name = "CL_DEVICE_LUID_KHR";
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return LUIDToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_HALF_FP_CONFIG> {
  using type = cl_device_fp_config;
  static constexpr std::string_view name = "CL_DEVICE_HALF_FP_CONFIG";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return FPConfigToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_SINGLE_FP_CONFIG> {
  using type = cl_device_fp_config;
  static constexpr std::string_view name = "CL_DEVICE_SINGLE_FP_CONFIG";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return FPConfigToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_DOUBLE_FP_CONFIG> {
  using type = cl_device_fp_config;
  static constexpr std::string_view name = "CL_DEVICE_DOUBLE_FP_CONFIG";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return FPConfigToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_REFERENCE_COUNT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_DEVICE_REFERENCE_COUNT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC> {
  using type = cl_bool;
  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_INTEROP_USER_SYNC";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

// === Context ===
template <> struct InfoTraits<CL_CONTEXT_REFERENCE_COUNT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_CONTEXT_REFERENCE_COUNT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_CONTEXT_NUM_DEVICES> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_CONTEXT_NUM_DEVICES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_CONTEXT_DEVICES> {
  using type = std::vector<cl::Device>;
  static constexpr std::string_view name = "CL_CONTEXT_DEVICES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DevicesToString(v);
  }
};

template <> struct InfoTraits<CL_CONTEXT_PROPERTIES> {
  using type = std::vector<cl_context_properties>;
  static constexpr std::string_view name = "CL_CONTEXT_PROPERTIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ContextPropertiesToString(v);
  }
};

// === Command Queue ===
template <> struct InfoTraits<CL_QUEUE_CONTEXT> {
  using type = cl::Context;
  static constexpr std::string_view name = "CL_QUEUE_CONTEXT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    (void)v;
    return "";
  }
};

template <> struct InfoTraits<CL_QUEUE_DEVICE> {
  using type = cl::Device;
  static constexpr std::string_view name = "CL_QUEUE_DEVICE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DeviceToString(v);
  }
};

template <> struct InfoTraits<CL_QUEUE_REFERENCE_COUNT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_QUEUE_REFERENCE_COUNT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_QUEUE_SIZE> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_QUEUE_SIZE";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_QUEUE_PROPERTIES> {
  using type = cl_command_queue_properties;
  static constexpr std::string_view name = "CL_QUEUE_PROPERTIES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesToString(v);
  }
};

template <> struct InfoTraits<CL_QUEUE_PROPERTIES_ARRAY> {
  using type = std::vector<cl_queue_properties>;
  static constexpr std::string_view name = "CL_QUEUE_PROPERTIES_ARRAY";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesArrayToString(v);
  }
};

// === Program ===
template <> struct InfoTraits<CL_PROGRAM_NUM_DEVICES> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_PROGRAM_NUM_DEVICES";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_PROGRAM_BINARY_SIZES> {
  using type = std::vector<size_t>;
  static constexpr std::string_view name = "CL_PROGRAM_BINARY_SIZES";
  static std::string ToString(type const &v) noexcept {
    return VectorToString(v);
  }
};

template <> struct InfoTraits<CL_PROGRAM_BINARIES> {
  using type = std::vector<std::vector<unsigned char>>;
  static constexpr std::string_view name = "CL_PROGRAM_BINARIES";
  static std::string ToString(type const &) noexcept {
    return "<binary blobs>";
  }
};

// === Kernel ===
template <> struct InfoTraits<CL_KERNEL_FUNCTION_NAME> {
  using type = std::string;
  static constexpr std::string_view name = "CL_KERNEL_FUNCTION_NAME";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_KERNEL_NUM_ARGS> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_KERNEL_NUM_ARGS";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_KERNEL_REFERENCE_COUNT> {
  using type = cl_uint;
  static constexpr std::string_view name = "CL_KERNEL_REFERENCE_COUNT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

template <> struct InfoTraits<CL_KERNEL_ATTRIBUTES> {
  using type = std::string;
  static constexpr std::string_view name = "CL_KERNEL_ATTRIBUTES";
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

template <> struct InfoTraits<CL_KERNEL_CONTEXT> {
  using type = cl::Context;
  static constexpr std::string_view name = "CL_KERNEL_CONTEXT";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    (void)v;
    return "";
  }
};

template <> struct InfoTraits<CL_KERNEL_PROGRAM> {
  using type = cl::Program;
  static constexpr std::string_view name = "CL_KERNEL_PROGRAM";
  [[nodiscard]] static std::string ToString(type v) noexcept {
    (void)v;
    return "";
  }
};
} // namespace ggems::ocl
