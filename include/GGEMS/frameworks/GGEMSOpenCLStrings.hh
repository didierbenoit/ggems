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
 * \brief String conversion utilities for OpenCL types.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"

/// \cond
#include <format>
#include <span>
#include <sstream>
/// \endcond

namespace ggems::ocl {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a packed OpenCL version value to a string.
 *
 * This helper decodes the \c cl_version integer into its \c major,
 * \c minor and \c patch components and formats them as a dotted
 * semantic version.
 *
 * \param version Packed OpenCL version value.
 *
 * \return Version string formatted as "major.minor.patch".
 */
[[nodiscard]] inline std::string ClVersionToString(cl_version version) {
  cl_uint major = (version >> 22) & 0x3FFu;
  cl_uint minor = (version >> 12) & 0x3FFu;
  cl_uint patch = (version >> 0) & 0xFFFu;
  return std::format("{}.{}.{}", major, minor, patch);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a list of OpenCL \c cl_name_version entries into a string.
 *
 * This helper formats each \c cl_name_version element as
 * "name (major.minor.patch)" and concatenates the results into a single
 * readable string. The ordering of elements is preserved.
 *
 * \param name_versions Vector of \c cl_name_version structures returned by
 *                 OpenCL device or platform queries.
 *
 * \return Human-readable concatenation of all entries, or an empty string if
 *         the vector is empty.
 */
[[nodiscard]] inline std::string
ClNameVersionToString(std::vector<cl_name_version> const &name_versions) {
  std::ostringstream oss;
  std::string out{""};
  for (auto const &nv : name_versions) {
    out += std::format("{} {} ", nv.name, ClVersionToString(nv.version));
  }
  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a vector of \c std::size_t elements to a string.
 *
 * Each element is written in decimal form and concatenated with commas.
 * This helper is primarily used to format work-item sizes and related
 * OpenCL dimensional values.
 *
 * \param sizes Vector of \c std::size_t values to convert.
 *
 * \return A comma-separated string containing all elements of \p values,
 *         or an empty string if the vector is empty.
 */
[[nodiscard]] inline std::string
SizeToString(std::vector<size_t> const &sizes) {
  std::string out{""};
  for (auto const &s : sizes) {
    out += std::format("{} ", s);
  }
  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL \c cl_device_type value to a human-readable string.
 *
 * This helper decodes the bitfield describing the type of a device
 * (CPU, GPU, accelerator, etc.). Multiple flags can be set simultaneously,
 * in which case they are concatenated with a separator.
 *
 * \param device_type Bitfield composed of \c CL_DEVICE_TYPE_* values.
 *
 * \return A string describing the device type, or "Unknown" if no
 *         recognised flag is set.
 */
[[nodiscard]] inline std::string
DeviceTypeToString(cl_device_type device_type) {
  std::ostringstream oss;
  bool first = true;

  auto const append = [&](std::string const &name) {
    if (!first) {
      oss << " | ";
    }
    oss << name;
    first = false;
  };

  if (device_type & CL_DEVICE_TYPE_CPU)
    append("CPU");
  if (device_type & CL_DEVICE_TYPE_GPU)
    append("GPU");
  if (device_type & CL_DEVICE_TYPE_ACCELERATOR)
    append("Accelerator");
  if (device_type & CL_DEVICE_TYPE_CUSTOM)
    append("Custom");
  if (device_type == CL_DEVICE_TYPE_DEFAULT)
    append("Default");
  if (device_type == CL_DEVICE_TYPE_ALL)
    append("All");

  if (first)
    oss << "Unknown(" << device_type << ')';

  return oss.str();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL vendor ID to a readable string.
 *
 * The mapping performed by this function is not standardised by OpenCL.
 * It uses a GGEMS-specific association to translate known vendor numeric
 * identifiers into their common textual names.
 *
 * \param vendor_id Numeric vendor identifier returned by OpenCL.
 *
 * \return A descriptive vendor name if recognised, otherwise the decimal
 *         string representation of \p id.
 */
[[nodiscard]] inline std::string VendorIdToString(cl_uint vendor_id) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL \c cl_device_mem_cache_type to a readable string.
 *
 * This helper interprets the memory cache type reported by the device and
 * maps it to a descriptive textual representation. The values correspond to
 * the OpenCL specification for device memory cache characteristics.
 *
 * \param type The \c cl_device_mem_cache_type value returned by OpenCL.
 *
 * \return A descriptive string such as "None", "Read-Only Cache",
 *         or "Read/Write Cache". Returns "Unknown" if the value is not part
 *         of the recognised enumeration set.
 */
[[nodiscard]] inline std::string
CacheTypeToString(cl_device_mem_cache_type type) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL \c cl_bool value to a human-readable string.
 *
 * This function is a safe \c noexcept helper returning a static textual
 * representation of a boolean value as defined by the OpenCL specification.
 *
 * \param flag The \c cl_bool value to convert, typically \c CL_TRUE or
 *              \c CL_FALSE.
 *
 * \return "Yes" for true values, "No" for false values.
 */
[[nodiscard]] inline std::string ClBoolToString(cl_bool flag) noexcept {
  return (flag == CL_TRUE) ? "Yes" : "No";
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL \c cl_device_local_mem_type to a readable string.
 *
 * This maps the local memory type reported by a device to its textual
 * representation. The values correspond to the OpenCL specification and
 * describe whether the device uses dedicated local memory or emulated
 * global memory.
 *
 * \param type The \c cl_device_local_mem_type value retrieved from OpenCL.
 *
 * \return A descriptive string such as "Local" or "Global". Returns "Unknown"
 *         if the value does not match a known enumeration entry.
 */
[[nodiscard]] inline std::string
LocalMemTypeToString(cl_device_local_mem_type type) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a bitfield of \c cl_command_queue_properties into a string.
 *
 * This helper decodes the command queue property flags associated with an
 * OpenCL queue. Since the argument is a bitfield, multiple properties may
 * be active simultaneously; the resulting string concatenates all detected
 * flags using the separator " | ".
 *
 * \param props Bitfield composed of \c CL_QUEUE_* flag values.
 *
 * \return A string listing all active properties, or "None" if the bitfield
 *         contains no recognised flags.
 */
[[nodiscard]] inline std::string
QueuePropertiesToString(cl_command_queue_properties &props) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a bitfield of \c cl_device_svm_capabilities into a string.
 *
 * This helper decodes the SVM (Shared Virtual Memory) capabilities supported
 * by an OpenCL device. Since the value is a bitfield, multiple capabilities
 * may be active simultaneously; all detected flags are concatenated using
 * the separator " | ".
 *
 * \param caps Bitfield composed of \c CL_DEVICE_SVM_* capability flags.
 *
 * \return A string listing all supported SVM capabilities, or "None" if the
 *         device reports no recognised feature.
 */
[[nodiscard]] inline std::string
SVMCapabilitiesToString(cl_device_svm_capabilities caps) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a vector of numeric values into a comma-separated string.
 *
 * This helper prints each element of the vector in decimal representation
 * and concatenates them using the separator ", ". It is used throughout the
 * OpenCL reporting utilities to format lists of numeric device properties.
 *
 * \param v Vector of numeric values to convert.
 *
 * \return A comma-separated string containing all values from \p values,
 *         or an empty string if the vector is empty.
 */
[[nodiscard]]
inline std::string VectorToString(std::vector<size_t> const &v) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a bitfield of \c cl_device_atomic_capabilities to a string.
 *
 * This helper decodes the atomic memory and fence capabilities supported by an
 * OpenCL device. Since the argument is a bitfield, several capabilities may be
 * active simultaneously; all detected flags are concatenated using the
 * separator " | ".
 *
 * \param caps Bitfield composed of \c CL_DEVICE_ATOMIC_* capability flags.
 *
 * \return A string describing all active capabilities, or "None" if no known
 *         flag is present.
 */
[[nodiscard]] inline std::string
AtomicCapabilitiesToString(cl_device_atomic_capabilities caps) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a bitfield of \c cl_device_device_enqueue_capabilities to a
 * string.
 *
 * This helper decodes the device enqueue capabilities supported by an OpenCL
 * device. The capabilities indicate whether kernels can enqueue other kernels
 * directly on the device. Since the argument is a bitfield, multiple flags
 * may be active and are concatenated using the separator " | ".
 *
 * \param caps Bitfield composed of \c CL_DEVICE_DEVICE_ENQUEUE_* capability
 * flags.
 *
 * \return A human-readable list of supported device enqueue capabilities,
 *         or "None" if no recognised flag is present.
 */
[[nodiscard]] inline std::string
DeviceEnqueueCapabilitiesToString(cl_device_device_enqueue_capabilities caps) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a list of \c cl_device_partition_property values to a string.
 *
 * This helper formats the partitioning properties supported or configured for
 * an OpenCL device. Since partition properties are expressed as an ordered list
 * of numeric constants, this function translates each entry into its readable
 * symbolic form.
 *
 * \param props Vector containing \c cl_device_partition_property values.
 *
 * \return A comma-separated list of partition properties, or an empty string
 *         if \p properties is empty.
 */
[[nodiscard]] inline std::string PartitionPropertiesToString(
    std::vector<cl_device_partition_property> const &props) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL \c cl_device_affinity_domain value into a string.
 *
 * This helper interprets the affinity domain reported by a device when
 * supporting partitioning based on cache hierarchy or NUMA topology. The value
 * is a bitfield and may contain multiple affinity traits simultaneously.
 *
 * \param domain Bitfield composed of \c CL_DEVICE_AFFINITY_DOMAIN_* flags.
 *
 * \return A string listing all active affinity domains, or "None" if no
 *         recognised domain flag is set.
 */
[[nodiscard]] inline std::string
AffinityDomainToString(cl_device_affinity_domain domain) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a 128-bit OpenCL UUID into a string.
 *
 * This helper formats a device or driver UUID retrieved through
 * \c CL_DEVICE_UUID_KHR or related queries. The function writes the UUID bytes
 * into a fixed-size buffer using the conventional hexadecimal representation.
 *
 * This function is \c noexcept because it performs no dynamic allocation and
 * uses only local fixed-size storage.
 *
 * \param s A span of \c CL_UUID_SIZE_KHR bytes containing the UUID value.
 *
 * \return A canonical UUID string formatted as
 *         "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX".
 */
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a 128-bit OpenCL UUID stored in a fixed-size array into a
 * string.
 *
 * This overload allows direct formatting of UUID values returned by OpenCL
 * as \c std::array<cl_uchar, CL_UUID_SIZE_KHR>. It forwards the conversion
 * to the span-based implementation, which performs the actual formatting.
 *
 * The operation is \c noexcept because no dynamic allocation is performed and
 * only fixed-size buffers are used internally.
 *
 * \param a Array containing the UUID value.
 *
 * \return Canonical UUID string formatted as
 *         "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX".
 */
[[nodiscard]]
inline std::string
UUIDToString(std::array<cl_uchar, CL_UUID_SIZE_KHR> const &a) noexcept {
  return UUIDToString(
      std::span<const cl_uchar, CL_UUID_SIZE_KHR>(a.data(), a.size()));
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a 64-bit Local Unique Identifier (LUID) into a string.
 *
 * This helper formats the LUID returned by OpenCL through
 * \c CL_DEVICE_LUID_KHR. Unlike UUIDs, LUIDs are 64-bit identifiers and do
 * not follow a canonical hyphenated layout. The function prints the bytes in
 * hexadecimal order.
 *
 * Since the function builds a dynamically sized \c std::string, it cannot be
 * declared \c noexcept.
 *
 * \param s Span containing \c CL_LUID_SIZE_KHR bytes representing the LUID.
 *
 * \return Hexadecimal string representation of the LUID.
 */
[[nodiscard]]
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a 64-bit LUID stored in a fixed-size array into a string.
 *
 * This overload forwards the array contents to the span-based implementation
 * of LUID formatting. It exists for convenience, allowing callers using
 * \c std::array<cl_uchar, CL_LUID_SIZE_KHR> to obtain a readable hexadecimal
 * representation without manually creating a span.
 *
 * \param a Array containing \c CL_LUID_SIZE_KHR bytes representing the LUID.
 *
 * \return Hexadecimal string representation of the LUID.
 */
[[nodiscard]]
inline std::string
LUIDToString(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &a) {
  return LUIDToString(
      std::span<const cl_uchar, CL_LUID_SIZE_KHR>(a.data(), a.size()));
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a \c cl_device_fp_config bitfield into a readable string.
 *
 * This helper decodes the floating-point capabilities supported by an
 * OpenCL device for a given precision (half, single, or double precision,
 * depending on where this function is used). Since the argument is a
 * bitfield, multiple flags may be simultaneously active; all detected entries
 * are concatenated using the separator " | ".
 *
 * \param cfg Bitfield composed of \c CL_FP_* flags.
 *
 * \return A human-readable list of FP configuration flags, or "None" if no
 *         recognised capability is present.
 */
[[nodiscard]] inline std::string FPConfigToString(cl_device_fp_config cfg) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an unsigned integer value to a string.
 *
 * This helper exists to keep formatting consistent across the GGEMS OpenCL
 * reporting utilities. It simply returns the decimal string representation
 * of the provided OpenCL integer.
 *
 * \param value Unsigned integer value to convert.
 *
 * \return Decimal representation of \p value.
 */
[[nodiscard]] inline std::string UIntToString(cl_uint value) {
  if (value == std::numeric_limits<cl_uint>::max())
    return "N/A";
  return std::to_string(value);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a list of OpenCL devices to a human-readable string.
 *
 * This helper retrieves the device name of each \c cl::Device present in
 * the input vector and concatenates them into a comma-separated list.
 * The order of devices is preserved.
 *
 * \param devices Vector of OpenCL \c cl::Device objects.
 *
 * \return A comma-separated list of device names, or an empty string if
 *         \p devices is empty.
 */
[[nodiscard]] inline std::string
DevicesToString(std::vector<cl::Device> &devices) {
  std::string out;
  for (auto const &d : devices) {
    out += std::format("{} ", d.getInfo<CL_DEVICE_NAME>());
  }
  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a single OpenCL device into a human-readable string.
 *
 * This helper retrieves and formats key descriptive information from the
 * provided \c cl::Device object. At minimum, it extracts the device name as
 * defined by \c CL_DEVICE_NAME.
 *
 * Additional fields may be appended by the caller when composing full device
 * reports.
 *
 * \param device OpenCL device object from which information is extracted.
 *
 * \return Name of the device as reported by OpenCL.
 */
[[nodiscard]] inline std::string DeviceToString(cl::Device &device) {
  return device.getInfo<CL_DEVICE_NAME>();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a \c cl_device_exec_capabilities bitfield into a string.
 *
 * This helper decodes the execution capabilities supported by an OpenCL
 * device. The capabilities indicate whether the device can execute regular
 * kernels and/or native kernels. Since the argument is a bitfield, several
 * capabilities may be active simultaneously.
 *
 * \param caps Bitfield composed of \c CL_EXEC_* flags.
 *
 * \return A comma-separated list of execution capabilities, or "None" if
 *         no recognised capability flag is set.
 */
[[nodiscard]] inline std::string
ExecCapabilitiesToString(cl_device_exec_capabilities caps) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a list of queue property key/value pairs into a readable
 * string.
 *
 * This helper processes the array of \c cl_queue_properties used when
 * creating OpenCL command queues. The list alternates between property keys
 * and associated values. The function formats each (key, value) pair into a
 * symbolic description when possible.
 *
 * \param qp Vector of queue property entries. The vector must contain
 *        an even number of elements representing successive (key, value) pairs.
 *
 * \return A comma-separated list of decoded queue properties, or an empty
 *         string if \p properties is empty.
 */
[[nodiscard]] inline std::string
QueuePropertiesArrayToString(std::vector<cl_queue_properties> const &qp) {
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
    default:
      out += std::format("UNKNOWN_PROPERTY({}) = {}", key, val);
      break;
    }
  }

  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert a list of OpenCL context properties into a readable string.
 *
 * This helper iterates through the raw \c cl_context_properties array used when
 * creating an OpenCL context. The property list alternates between a property
 * identifier and an associated value. This function formats each pair into a
 * descriptive textual representation.
 *
 * \param cp Vector of \c cl_context_properties elements. The vector
 *        must contain an even number of entries, representing (key, value)
 *        pairs as defined by the OpenCL specification.
 *
 * \return A comma-separated list of context property descriptions, or an empty
 *         string if \p properties is empty.
 */
[[nodiscard]] inline std::string
ContextPropertiesToString(std::vector<cl_context_properties> const &cp) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL kernel argument address qualifier to a string.
 *
 * This helper decodes the \c cl_kernel_arg_address_qualifier value returned
 * by \c clGetKernelArgInfo. Address qualifiers describe whether a kernel
 * argument refers to global, local, constant or private memory.
 *
 * \param aq The address qualifier value.
 *
 * \return A string such as "Global", "Local", "Constant" or "Private".
 *         Returns "Unknown" if the value is not part of the recognised set.
 */
[[nodiscard]] inline std::string
ArgAddressQualifierToString(cl_kernel_arg_address_qualifier aq) noexcept {
  switch (aq) {
  case CL_KERNEL_ARG_ADDRESS_GLOBAL:
    return "CL_KERNEL_ARG_ADDRESS_GLOBAL";
  case CL_KERNEL_ARG_ADDRESS_LOCAL:
    return "CL_KERNEL_ARG_ADDRESS_LOCAL";
  case CL_KERNEL_ARG_ADDRESS_CONSTANT:
    return "CL_KERNEL_ARG_ADDRESS_CONSTANT";
  default: {
    return "CL_KERNEL_ARG_ADDRESS_PRIVATE";
  }
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL kernel argument access qualifier to a string.
 *
 * This helper converts the \c cl_kernel_arg_access_qualifier value returned
 * by \c clGetKernelArgInfo for image arguments. It describes how a kernel is
 * allowed to access the underlying memory object.
 *
 * \param aq The access qualifier associated with a kernel argument.
 *
 * \return A human-readable string such as "ReadOnly", "WriteOnly",
 *         "ReadWrite" or "None". Returns "Unknown" if not recognised.
 */
[[nodiscard]] inline std::string
ArgAccessQualifierToString(cl_kernel_arg_address_qualifier aq) noexcept {
  switch (aq) {
  case CL_KERNEL_ARG_ACCESS_READ_ONLY:
    return "CL_KERNEL_ARG_ACCESS_READ_ONLY";
  case CL_KERNEL_ARG_ACCESS_WRITE_ONLY:
    return "CL_KERNEL_ARG_ACCESS_WRITE_ONLY";
  case CL_KERNEL_ARG_ACCESS_READ_WRITE:
    return "CL_KERNEL_ARG_ACCESS_READ_WRITE";
  default: {
    return "CL_KERNEL_ARG_ACCESS_NONE";
  }
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*!
 * \brief Convert an OpenCL kernel argument type qualifier to a readable string.
 *
 * Type qualifiers describe additional semantic constraints on kernel
 * arguments, such as whether the argument is declared \c const, \c volatile,
 * \c restrict, or \c pipe. This function maps the bitfield value returned by
 * \c clGetKernelArgInfo to its textual representation.
 *
 * \param aq The type qualifier bitfield.
 *
 * \return A comma-separated list of qualifiers (e.g. "Const,Volatile"), or
 *         "None" when no qualifier is present.
 */
[[nodiscard]] inline std::string
ArgTypeQualifierToString(cl_kernel_arg_type_qualifier aq) {
  std::string out;
  if (aq & CL_KERNEL_ARG_TYPE_CONST)
    out += "CL_KERNEL_ARG_TYPE_CONST ";
  if (aq & CL_KERNEL_ARG_TYPE_RESTRICT)
    out += "CL_KERNEL_ARG_TYPE_RESTRICT ";
  if (aq & CL_KERNEL_ARG_TYPE_VOLATILE)
    out += "CL_KERNEL_ARG_TYPE_VOLATILE ";
  if (aq & CL_KERNEL_ARG_TYPE_PIPE)
    out += "CL_KERNEL_ARG_TYPE_PIPE ";
  if (aq & CL_KERNEL_ARG_TYPE_NONE)
    out += "CL_KERNEL_ARG_TYPE_NONE ";

  if (!out.empty())
    out.erase(out.size() - 1);
  else
    out = "None";
  return out;
}
} // namespace ggems::ocl
