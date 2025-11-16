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
 * \file GGEMSOpenCLStrings.hh
 * \brief
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-11-09
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <CL/cl.h>
#include <CL/opencl.hpp>
#include <concepts>
#include <format>
#include <span>
#include <sstream>
/// \endcond

namespace ggems::ocl {
[[nodiscard]] inline std::string
ClVersionToString(cl_version version) noexcept {
  cl_uint major = (version >> 22) & 0x3FFu;
  cl_uint minor = (version >> 12) & 0x3FFu;
  cl_uint patch = (version >> 0) & 0xFFFu;
  return std::format("{}.{}.{}", major, minor, patch);
}

/* -------------------------------------------------------*/

[[nodiscard]] inline std::string ClNameVersionToString(
    std::vector<cl_name_version> const &name_versions) noexcept {
  std::ostringstream oss;
  std::string out{""};
  for (auto const &nv : name_versions) {
    out += std::format("{} {} ", nv.name, ClVersionToString(nv.version));
  }
  return out;
}

/* -------------------------------------------------------*/

[[nodiscard]] inline std::string
SizeToString(std::vector<size_t> const &sizes) noexcept {
  std::string out{""};
  for (auto const &s : sizes) {
    out += std::format("{} ", s);
  }
  return out;
}

/* -------------------------------------------------------*/

[[nodiscard]] inline std::string
DeviceTypeToString(cl_device_type deviceType) noexcept {
  std::ostringstream oss;
  bool first = true;

  auto const append = [&](std::string const &name) {
    if (!first) {
      oss << " | ";
    }
    oss << name;
    first = false;
  };

  if (deviceType & CL_DEVICE_TYPE_CPU)
    append("CPU");
  if (deviceType & CL_DEVICE_TYPE_GPU)
    append("GPU");
  if (deviceType & CL_DEVICE_TYPE_ACCELERATOR)
    append("Accelerator");
  if (deviceType & CL_DEVICE_TYPE_CUSTOM)
    append("Custom");
  if (deviceType == CL_DEVICE_TYPE_DEFAULT)
    append("Default");
  if (deviceType == CL_DEVICE_TYPE_ALL)
    append("All");

  if (first)
    oss << "Unknown(" << deviceType << ')';

  return oss.str();
}

[[nodiscard]] inline std::string VendorIdToString(cl_uint vendor_id) noexcept {
  switch (vendor_id) {
  case 0x8086:
    return "Intel Corporation";
  case 0x10DE:
    return "NVIDIA Corporation";
  case 0x1002:
    return "AMD / ATI";
  case 0x13B5:
    return "ARM";
  case 0x5143:
    return "Qualcomm";
  case 0x106B:
    return "Apple";
  case 0x1010:
    return "Imagination Technologies";
  default: {
    std::ostringstream oss;
    oss << "Unknown (0x" << std::hex << vendor_id << ")" << std::dec;
    return oss.str();
  }
  }
}

[[nodiscard]] inline std::string
CacheTypeToString(cl_device_mem_cache_type type) noexcept {
  switch (type) {
  case CL_NONE:
    return "None";
  case CL_READ_ONLY_CACHE:
    return "Read-only cache";
  case CL_READ_WRITE_CACHE:
    return "Read/Write cache";
  default: {
    std::ostringstream oss;
    oss << "Unknown (0x" << std::hex << type << std::dec << ")";
    return oss.str();
  }
  }
}

[[nodiscard]] inline std::string ClBoolToString(cl_bool flag) noexcept {
  return (flag == CL_TRUE) ? "Yes" : "No";
}

[[nodiscard]] inline std::string
LocalMemTypeToString(cl_device_local_mem_type type) noexcept {
  switch (type) {
  case CL_NONE:
    return "None";
  case CL_LOCAL:
    return "Local memory (on-chip)";
  case CL_GLOBAL:
    return "Global memory (emulated local)";
  default: {
    std::ostringstream oss;
    oss << "Unknown (0x" << std::hex << type << std::dec << ")";
    return oss.str();
  }
  }
}

[[nodiscard]] inline std::string
QueuePropertiesToString(cl_command_queue_properties &props) noexcept {
  std::ostringstream oss;
  if (props & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
    oss << "Out-of-order execution, ";
  if (props & CL_QUEUE_PROFILING_ENABLE)
    oss << "Profiling enabled, ";
#ifdef CL_QUEUE_ON_DEVICE
  if (props & CL_QUEUE_ON_DEVICE)
    oss << "On-device queue, ";
#endif
#ifdef CL_QUEUE_ON_DEVICE_DEFAULT
  if (props & CL_QUEUE_ON_DEVICE_DEFAULT)
    oss << "Default on-device queue, ";
#endif

  std::string result = oss.str();
  if (!result.empty())
    result.erase(result.size() - 2);
  else
    result = "None";

  return result;
}

[[nodiscard]] inline std::string
SVMCapabilitiesToString(cl_device_svm_capabilities caps) noexcept {
  std::ostringstream oss;
  if (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)
    oss << "Coarse-grain buffer, ";
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
    oss << "Fine-grain buffer, ";
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
    oss << "Fine-grain system, ";
  if (caps & CL_DEVICE_SVM_ATOMICS)
    oss << "Atomics, ";

  std::string out = oss.str();
  if (!out.empty())
    out.erase(out.size() - 2);
  else
    out = "None";
  return out;
}

[[nodiscard]]
inline std::string VectorToString(std::vector<size_t> const &v) noexcept {
  if (v.empty())
    return "[]";

  std::string out = "[";
  for (size_t i = 0; i < v.size(); ++i) {
    out.append(std::to_string(v[i]));
    if (i + 1 < v.size())
      out.append(", ");
  }
  out.append("]");
  return out;
}

[[nodiscard]] inline std::string
AtomicCapabilitiesToString(cl_device_atomic_capabilities caps) noexcept {
  std::ostringstream oss;
#ifdef CL_DEVICE_ATOMIC_ORDER_RELAXED
  if (caps & CL_DEVICE_ATOMIC_ORDER_RELAXED)
    oss << "Relaxed order, ";
#endif
#ifdef CL_DEVICE_ATOMIC_ORDER_ACQ_REL
  if (caps & CL_DEVICE_ATOMIC_ORDER_ACQ_REL)
    oss << "Acquire/Release, ";
#endif
#ifdef CL_DEVICE_ATOMIC_ORDER_SEQ_CST
  if (caps & CL_DEVICE_ATOMIC_ORDER_SEQ_CST)
    oss << "Sequentially consistent, ";
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM
  if (caps & CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM)
    oss << "Scope: Work-item, ";
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP
  if (caps & CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP)
    oss << "Scope: Work-group, ";
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_DEVICE
  if (caps & CL_DEVICE_ATOMIC_SCOPE_DEVICE)
    oss << "Scope: Device, ";
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES
  if (caps & CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES)
    oss << "Scope: All devices, ";
#endif

  std::string s = oss.str();
  if (!s.empty())
    s.erase(s.size() - 2);
  else
    s = "None";
  return s;
}

[[nodiscard]] inline std::string DeviceEnqueueCapabilitiesToString(
    cl_device_device_enqueue_capabilities caps) noexcept {
  std::ostringstream oss;
#ifdef CL_DEVICE_QUEUE_SUPPORTED
  if (caps & CL_DEVICE_QUEUE_SUPPORTED)
    oss << "Device queues supported, ";
#endif
#ifdef CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT
  if (caps & CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT)
    oss << "Default queue replaceable, ";
#endif
#ifdef CL_DEVICE_QUEUE_CROSS_DEVICE
  if (caps & CL_DEVICE_QUEUE_CROSS_DEVICE)
    oss << "Cross-device enqueue, ";
#endif
#ifdef CL_DEVICE_QUEUE_CROSS_CONTEXT
  if (caps & CL_DEVICE_QUEUE_CROSS_CONTEXT)
    oss << "Cross-context enqueue, ";
#endif

  std::string s = oss.str();
  if (!s.empty())
    s.erase(s.size() - 2);
  else
    s = "None";
  return s;
}

[[nodiscard]] inline std::string PartitionPropertiesToString(
    std::vector<cl_device_partition_property> const &props) noexcept {
  if (props.empty())
    return "None";

  std::ostringstream oss;
  for (auto p : props) {
    switch (p) {
    case CL_DEVICE_PARTITION_EQUALLY:
      oss << "Equally, ";
      break;
    case CL_DEVICE_PARTITION_BY_COUNTS:
      oss << "By counts, ";
      break;
    case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
      oss << "By affinity domain, ";
      break;
    default:
      break;
    }
  }

  std::string s = oss.str();
  if (!s.empty())
    s.erase(s.size() - 2);
  else
    s = "None";
  return s;
}

[[nodiscard]] inline std::string
AffinityDomainToString(cl_device_affinity_domain domain) noexcept {
  std::ostringstream oss;

#ifdef CL_DEVICE_AFFINITY_DOMAIN_NUMA
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_NUMA)
    oss << "NUMA, ";
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE)
    oss << "L4 cache, ";
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE)
    oss << "L3 cache, ";
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE)
    oss << "L2 cache, ";
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE)
    oss << "L1 cache, ";
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE)
    oss << "Next partitionable, ";
#endif

  std::string s = oss.str();
  if (!s.empty())
    s.erase(s.size() - 2);
  else
    s = "None";
  return s;
}

[[nodiscard]]
inline std::string
UUIDToString(std::span<const cl_uchar, CL_UUID_SIZE_KHR> s) noexcept {
  // format 8-4-4-4-12
  char buf[36 + 1]{};

  auto hex = [](cl_uchar b) -> std::array<char, 2> {
    constexpr char k[] = "0123456789abcdef";
    return {k[(b >> 4) & 0xF], k[b & 0xF]};
  };

  int p = 0;
  auto put = [&](cl_uchar b) {
    auto h = hex(b);
    buf[p++] = h[0];
    buf[p++] = h[1];
  };

  for (std::size_t i = 0; i < 16; ++i) {
    put(s[i]);
    if (i == 3 || i == 5 || i == 7 || i == 9)
      buf[p++] = '-';
  }
  buf[p] = 0;
  return std::string(buf);
}

[[nodiscard]]
inline std::string
UUIDToString(std::array<cl_uchar, CL_UUID_SIZE_KHR> const &a) noexcept {
  return UUIDToString(
      std::span<const cl_uchar, CL_UUID_SIZE_KHR>(a.data(), a.size()));
}

inline std::string LUIDToString(std::span<const cl_uchar, CL_LUID_SIZE_KHR> s) {
  std::string out;
  out.reserve(CL_LUID_SIZE_KHR * 2);
  constexpr char k[] = "0123456789abcdef";
  for (auto b : s) {
    out.push_back(k[(b >> 4) & 0xF]);
    out.push_back(k[b & 0xF]);
  }
  return out;
}

[[nodiscard]]
inline std::string
LUIDToString(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &a) noexcept {
  return LUIDToString(
      std::span<const cl_uchar, CL_LUID_SIZE_KHR>(a.data(), a.size()));
}

[[nodiscard]] inline std::string
FPConfigToString(cl_device_fp_config cfg) noexcept {
  std::ostringstream oss;
  if (cfg & CL_FP_DENORM)
    oss << "Denormals, ";
  if (cfg & CL_FP_INF_NAN)
    oss << "Inf/NaN, ";
  if (cfg & CL_FP_ROUND_TO_NEAREST)
    oss << "RoundToNearest, ";
  if (cfg & CL_FP_ROUND_TO_ZERO)
    oss << "RoundToZero, ";
  if (cfg & CL_FP_ROUND_TO_INF)
    oss << "RoundToInf, ";
  if (cfg & CL_FP_FMA)
    oss << "FMA, ";
  if (cfg & CL_FP_SOFT_FLOAT)
    oss << "SoftFloat, ";
  if (cfg & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT)
    oss << "CorrectDivideSqrt, ";

  auto s = oss.str();
  if (!s.empty())
    s.pop_back(), s.pop_back();

  return s.empty() ? "None" : s;
}

[[nodiscard]] inline std::string UIntToString(cl_uint value) noexcept {
  if (value == std::numeric_limits<cl_uint>::max())
    return "N/A";
  return std::to_string(value);
}

[[nodiscard]] inline std::string
DevicesToString(std::vector<cl::Device> &devices) noexcept {
  std::string out;
  for (auto const &d : devices) {
    out += std::format("{} ", d.getInfo<CL_DEVICE_NAME>());
  }
  return out;
}

[[nodiscard]] inline std::string DeviceToString(cl::Device &device) noexcept {
  return device.getInfo<CL_DEVICE_NAME>();
}

[[nodiscard]] inline std::string
ExecCapabilitiesToString(cl_device_exec_capabilities caps) noexcept {
  if (caps == 0)
    return "None";

  std::ostringstream oss;
  bool first = true;

  auto add = [&](std::string_view name) {
    if (!first)
      oss << ", ";
    oss << name;
    first = false;
  };

  if (caps & CL_EXEC_KERNEL)
    add("Kernel execution");
  if (caps & CL_EXEC_NATIVE_KERNEL)
    add("Native kernel execution");

  return oss.str();
}

[[nodiscard]] inline std::string QueuePropertiesArrayToString(
    std::vector<cl_queue_properties> const &qp) noexcept {
  std::string out;

  for (std::size_t i = 0; i < qp.size(); i += 2) {
    auto key = qp[i];
    if (key == 0)
      break;

    auto val = qp[i + 1];

    switch (key) {
    case CL_QUEUE_PROPERTIES:
      out += std::format("CL_QUEUE_PROPERTIES = {} ",
                         QueuePropertiesToString(val));
      break;

      //   case CL_QUEUE_SIZE:
      //   out += std::format("CL_QUEUE_SIZE = {} ", val);
      //   break;

    default:
      out += std::format("UNKNOWN_PROPERTY({}) = {}", key, val);
      break;
    }
  }

  return out;
}

[[nodiscard]] inline std::string ContextPropertiesToString(
    std::vector<cl_context_properties> const &cp) noexcept {
  std::string out;

  for (std::size_t i = 0; i < cp.size(); i += 2) {
    auto key = cp[i];
    if (key == 0)
      break;

    auto val = cp[i + 1];
    void *ptr = reinterpret_cast<void *>(val);

    switch (key) {
    case CL_CONTEXT_PLATFORM:
      out += std::format("CL_CONTEXT_PLATFORM = 0x{:016x} ",
                         reinterpret_cast<std::uintptr_t>(ptr));
      break;

    case CL_GL_CONTEXT_KHR:
      out += std::format("CL_GL_CONTEXT_KHR = 0x{:016x} ",
                         reinterpret_cast<std::uintptr_t>(ptr));
      break;

    default:
      out += std::format("UNKNOWN_PROPERTY({}) = 0x{:016x}", key,
                         reinterpret_cast<std::uintptr_t>(ptr));
      break;
    }
  }

  return out;
}
} // namespace ggems::ocl
