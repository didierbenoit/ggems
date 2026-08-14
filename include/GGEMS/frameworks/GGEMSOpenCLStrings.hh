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
 * \brief Provides human-readable conversions for OpenCL information values.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <ios>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"

namespace ggems::ocl {

/*!
 * \brief Converts a packed OpenCL version to text.
 *
 * \param[in] version Packed OpenCL version value.
 * \return Dotted major.minor.patch version string.
 */
[[nodiscard]] inline auto ClVersionToString(cl_version version) -> std::string {
  return std::format("{}.{}.{}", CL_VERSION_MAJOR(version),
                     CL_VERSION_MINOR(version), CL_VERSION_PATCH(version));
}

/*!
 * \brief Formats OpenCL name/version entries.
 *
 * \param[in] name_versions OpenCL name/version entries.
 * \return Readable name/version list.
 */
[[nodiscard]] inline auto
ClNameVersionToString(std::vector<cl_name_version> const &name_versions)
    -> std::string {
  std::string output{};
  for (auto const &name_version : name_versions) {
    output += std::format("{} {} ", name_version.name,
                          ClVersionToString(name_version.version));
  }
  return output;
}

/*!
 * \brief Formats a sequence of size values.
 *
 * \param[in] sizes Size values to format.
 * \return Space-separated size values.
 */
[[nodiscard]] inline auto SizeToString(std::vector<std::size_t> const &sizes)
    -> std::string {
  std::string output{};
  for (auto const size : sizes) {
    output += std::format("{} ", size);
  }
  return output;
}

/*!
 * \brief Converts an OpenCL device-type bitfield to text.
 *
 * \param[in] device_type OpenCL device-type bitfield.
 * \return Readable device-type description.
 */
[[nodiscard]] inline auto DeviceTypeToString(cl_device_type device_type)
    -> std::string {
  std::ostringstream stream;
  bool first = true;

  auto const append = [&](std::string const &name) -> void {
    if (!first) {
      stream << " | ";
    }
    stream << name;
    first = false;
  };

  if ((device_type & CL_DEVICE_TYPE_CPU) != 0) {
    append("CPU");
  }
  if ((device_type & CL_DEVICE_TYPE_GPU) != 0) {
    append("GPU");
  }
  if ((device_type & CL_DEVICE_TYPE_ACCELERATOR) != 0) {
    append("Accelerator");
  }
  if ((device_type & CL_DEVICE_TYPE_CUSTOM) != 0) {
    append("Custom");
  }
  if (device_type == CL_DEVICE_TYPE_DEFAULT) {
    append("Default");
  }
  if (device_type == CL_DEVICE_TYPE_ALL) {
    append("All");
  }

  if (first) {
    stream << "Unknown(" << device_type << ')';
  }

  return stream.str();
}

/*!
 * \brief Converts a known OpenCL vendor identifier to text.
 *
 * \param[in] vendor_id OpenCL vendor identifier.
 * \return Known vendor name or a hexadecimal identifier.
 */
[[nodiscard]] inline auto VendorIdToString(cl_uint vendor_id) -> std::string {
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
    std::ostringstream stream;
    stream << "Unknown (0x" << std::hex << vendor_id << ")" << std::dec;
    return stream.str();
  }
  }
}

/*!
 * \brief Converts an OpenCL memory-cache type to text.
 *
 * \param[in] type OpenCL memory-cache type.
 * \return Readable cache-type name.
 */
[[nodiscard]] inline auto CacheTypeToString(cl_device_mem_cache_type type)
    -> std::string {
  switch (type) {
  case CL_NONE:
    return "None";
  case CL_READ_ONLY_CACHE:
    return "Read-only cache";
  case CL_READ_WRITE_CACHE:
    return "Read/Write cache";
  default: {
    std::ostringstream stream;
    stream << "Unknown (0x" << std::hex << type << std::dec << ")";
    return stream.str();
  }
  }
}

/*!
 * \brief Converts an OpenCL Boolean value to text.
 *
 * \param[in] flag OpenCL Boolean value.
 * \return "Yes" for CL_TRUE, otherwise "No".
 */
[[nodiscard]] inline auto ClBoolToString(cl_bool flag) -> std::string {
  return (flag == CL_TRUE) ? "Yes" : "No";
}

/*!
 * \brief Converts an OpenCL local-memory type to text.
 *
 * \param[in] type OpenCL local-memory type.
 * \return Readable local-memory type.
 */
[[nodiscard]] inline auto LocalMemTypeToString(cl_device_local_mem_type type)
    -> std::string {
  switch (type) {
  case CL_NONE:
    return "None";
  case CL_LOCAL:
    return "Local memory (on-chip)";
  case CL_GLOBAL:
    return "Global memory (emulated local)";
  default: {
    std::ostringstream stream;
    stream << "Unknown (0x" << std::hex << type << std::dec << ")";
    return stream.str();
  }
  }
}

/*!
 * \brief Formats an OpenCL command-queue property bitfield.
 *
 * \param[in] properties Command-queue property bitfield.
 * \return Readable command-queue properties.
 */
[[nodiscard]] inline auto
QueuePropertiesToString(cl_command_queue_properties const &properties)
    -> std::string {
  std::ostringstream stream;
  if ((properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0) {
    stream << "Out-of-order execution, ";
  }
  if ((properties & CL_QUEUE_PROFILING_ENABLE) != 0) {
    stream << "Profiling enabled, ";
  }
#ifdef CL_QUEUE_ON_DEVICE
  if ((properties & CL_QUEUE_ON_DEVICE) != 0) {
    stream << "On-device queue, ";
  }
#endif
#ifdef CL_QUEUE_ON_DEVICE_DEFAULT
  if ((properties & CL_QUEUE_ON_DEVICE_DEFAULT) != 0) {
    stream << "Default on-device queue, ";
  }
#endif

  std::string result = stream.str();
  if (!result.empty()) {
    result.erase(result.size() - 2);
  } else {
    result = "None";
  }

  return result;
}

/*!
 * \brief Formats OpenCL SVM capability flags.
 *
 * \param[in] capabilities SVM capability bitfield.
 * \return Readable SVM capabilities.
 */
[[nodiscard]] inline auto
SVMCapabilitiesToString(cl_device_svm_capabilities capabilities)
    -> std::string {
  std::ostringstream stream;
  if ((capabilities & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) != 0) {
    stream << "Coarse-grain buffer, ";
  }
  if ((capabilities & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) != 0) {
    stream << "Fine-grain buffer, ";
  }
  if ((capabilities & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) != 0) {
    stream << "Fine-grain system, ";
  }
  if ((capabilities & CL_DEVICE_SVM_ATOMICS) != 0) {
    stream << "Atomics, ";
  }

  std::string output = stream.str();
  if (!output.empty()) {
    output.erase(output.size() - 2);
  } else {
    output = "None";
  }
  return output;
}

/*!
 * \brief Formats a vector of size values.
 *
 * \param[in] values Values to format.
 * \return Bracketed comma-separated values.
 */
[[nodiscard]] inline auto VectorToString(std::vector<std::size_t> const &values)
    -> std::string {
  if (values.empty()) {
    return "[]";
  }

  std::string output = "[";
  for (std::size_t index = 0; index < values.size(); ++index) {
    output.append(std::to_string(values[index]));
    if (index + 1 < values.size()) {
      output.append(", ");
    }
  }
  output.append("]");
  return output;
}

/*!
 * \brief Formats OpenCL atomic capability flags.
 *
 * \param[in] capabilities Atomic capability bitfield.
 * \return Readable atomic capabilities.
 */
[[nodiscard]] inline auto
AtomicCapabilitiesToString(cl_device_atomic_capabilities capabilities)
    -> std::string {
  std::ostringstream stream;
#ifdef CL_DEVICE_ATOMIC_ORDER_RELAXED
  if ((capabilities & CL_DEVICE_ATOMIC_ORDER_RELAXED) != 0) {
    stream << "Relaxed order, ";
  }
#endif
#ifdef CL_DEVICE_ATOMIC_ORDER_ACQ_REL
  if ((capabilities & CL_DEVICE_ATOMIC_ORDER_ACQ_REL) != 0) {
    stream << "Acquire/Release, ";
  }
#endif
#ifdef CL_DEVICE_ATOMIC_ORDER_SEQ_CST
  if ((capabilities & CL_DEVICE_ATOMIC_ORDER_SEQ_CST) != 0) {
    stream << "Sequentially consistent, ";
  }
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM
  if ((capabilities & CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM) != 0) {
    stream << "Scope: Work-item, ";
  }
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP
  if ((capabilities & CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP) != 0) {
    stream << "Scope: Work-group, ";
  }
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_DEVICE
  if ((capabilities & CL_DEVICE_ATOMIC_SCOPE_DEVICE) != 0) {
    stream << "Scope: Device, ";
  }
#endif
#ifdef CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES
  if ((capabilities & CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES) != 0) {
    stream << "Scope: All devices, ";
  }
#endif

  std::string result = stream.str();
  if (!result.empty()) {
    result.erase(result.size() - 2);
  } else {
    result = "None";
  }
  return result;
}

/*!
 * \brief Formats OpenCL device-enqueue capability flags.
 *
 * \param[in] capabilities Device-enqueue capability bitfield.
 * \return Readable device-enqueue capabilities.
 */
[[nodiscard]] inline auto DeviceEnqueueCapabilitiesToString(
    cl_device_device_enqueue_capabilities capabilities) -> std::string {
  std::ostringstream stream;
#ifdef CL_DEVICE_QUEUE_SUPPORTED
  if ((capabilities & CL_DEVICE_QUEUE_SUPPORTED) != 0) {
    stream << "Device queues supported, ";
  }
#endif
#ifdef CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT
  if ((capabilities & CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT) != 0) {
    stream << "Default queue replaceable, ";
  }
#endif
#ifdef CL_DEVICE_QUEUE_CROSS_DEVICE
  if ((capabilities & CL_DEVICE_QUEUE_CROSS_DEVICE) != 0) {
    stream << "Cross-device enqueue, ";
  }
#endif
#ifdef CL_DEVICE_QUEUE_CROSS_CONTEXT
  if ((capabilities & CL_DEVICE_QUEUE_CROSS_CONTEXT) != 0) {
    stream << "Cross-context enqueue, ";
  }
#endif

  std::string result = stream.str();
  if (!result.empty()) {
    result.erase(result.size() - 2);
  } else {
    result = "None";
  }
  return result;
}

/*!
 * \brief Formats OpenCL device-partition properties.
 *
 * \param[in] properties Partition property values.
 * \return Readable partition properties.
 */
[[nodiscard]] inline auto PartitionPropertiesToString(
    std::vector<cl_device_partition_property> const &properties)
    -> std::string {
  if (properties.empty()) {
    return "None";
  }

  std::ostringstream stream;
  for (auto const property : properties) {
    switch (property) {
    case CL_DEVICE_PARTITION_EQUALLY:
      stream << "Equally, ";
      break;
    case CL_DEVICE_PARTITION_BY_COUNTS:
      stream << "By counts, ";
      break;
    case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
      stream << "By affinity domain, ";
      break;
    default:
      break;
    }
  }

  std::string result = stream.str();
  if (!result.empty()) {
    result.erase(result.size() - 2);
  } else {
    result = "None";
  }
  return result;
}

/*!
 * \brief Formats OpenCL device-affinity domain flags.
 *
 * \param[in] domain Affinity-domain bitfield.
 * \return Readable affinity domains.
 */
[[nodiscard]] inline auto
AffinityDomainToString(cl_device_affinity_domain domain) -> std::string {
  std::ostringstream stream;

#ifdef CL_DEVICE_AFFINITY_DOMAIN_NUMA
  if ((domain & CL_DEVICE_AFFINITY_DOMAIN_NUMA) != 0) {
    stream << "NUMA, ";
  }
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE
  if ((domain & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE) != 0) {
    stream << "L4 cache, ";
  }
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE
  if ((domain & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE) != 0) {
    stream << "L3 cache, ";
  }
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE
  if ((domain & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE) != 0) {
    stream << "L2 cache, ";
  }
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE
  if ((domain & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE) != 0) {
    stream << "L1 cache, ";
  }
#endif
#ifdef CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE
  if ((domain & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE) != 0) {
    stream << "Next partitionable, ";
  }
#endif

  std::string result = stream.str();
  if (!result.empty()) {
    result.erase(result.size() - 2);
  } else {
    result = "None";
  }
  return result;
}

/*!
 * \brief Formats an OpenCL UUID as canonical hexadecimal text.
 *
 * \param[in] uuid OpenCL UUID bytes.
 * \return Canonical UUID string.
 */
[[nodiscard]] inline auto
UUIDToString(std::span<cl_uchar const, CL_UUID_SIZE_KHR> uuid) -> std::string {
  std::array<char, 37> buffer{};

  auto const to_hex_pair = [](cl_uchar byte) -> std::array<char, 2> {
    constexpr std::string_view hex_digits{"0123456789abcdef"};
    return {hex_digits[(byte >> 4) & 0xF], hex_digits[byte & 0xF]};
  };

  std::size_t position = 0;
  auto const append_byte = [&](cl_uchar byte) -> void {
    auto const hex_pair = to_hex_pair(byte);
    buffer[position++] = hex_pair[0];
    buffer[position++] = hex_pair[1];
  };

  for (std::size_t index = 0; index < uuid.size(); ++index) {
    append_byte(uuid[index]);
    if (index == 3 || index == 5 || index == 7 || index == 9) {
      buffer[position++] = '-';
    }
  }
  buffer[position] = '\0';
  return {buffer.data()};
}

/*!
 * \brief Formats an OpenCL UUID as canonical hexadecimal text.
 *
 * \param[in] uuid OpenCL UUID bytes.
 * \return Canonical UUID string.
 */
[[nodiscard]] inline auto
UUIDToString(std::array<cl_uchar, CL_UUID_SIZE_KHR> const &uuid)
    -> std::string {
  return UUIDToString(
      std::span<cl_uchar const, CL_UUID_SIZE_KHR>(uuid.data(), uuid.size()));
}

/*!
 * \brief Formats an OpenCL LUID as hexadecimal text.
 *
 * \param[in] luid OpenCL LUID bytes.
 * \return Hexadecimal LUID string.
 */
[[nodiscard]] inline auto
LUIDToString(std::span<cl_uchar const, CL_LUID_SIZE_KHR> luid) -> std::string {
  std::string output;
  output.reserve(CL_LUID_SIZE_KHR * 2);
  constexpr std::string_view hex_digits{"0123456789abcdef"};

  for (auto byte : luid) {
    output.push_back(hex_digits[(byte >> 4) & 0xF]);
    output.push_back(hex_digits[byte & 0xF]);
  }
  return output;
}

/*!
 * \brief Formats an OpenCL LUID as hexadecimal text.
 *
 * \param[in] luid OpenCL LUID bytes.
 * \return Hexadecimal LUID string.
 */
[[nodiscard]] inline auto
LUIDToString(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid)
    -> std::string {
  return LUIDToString(
      std::span<cl_uchar const, CL_LUID_SIZE_KHR>(luid.data(), luid.size()));
}

/*!
 * \brief Formats OpenCL floating-point capability flags.
 *
 * \param[in] config Floating-point capability bitfield.
 * \return Readable floating-point capabilities.
 */
[[nodiscard]] inline auto FPConfigToString(cl_device_fp_config config)
    -> std::string {
  std::ostringstream stream;
  if ((config & CL_FP_DENORM) != 0) {
    stream << "Denormals, ";
  }
  if ((config & CL_FP_INF_NAN) != 0) {
    stream << "Inf/NaN, ";
  }
  if ((config & CL_FP_ROUND_TO_NEAREST) != 0) {
    stream << "RoundToNearest, ";
  }
  if ((config & CL_FP_ROUND_TO_ZERO) != 0) {
    stream << "RoundToZero, ";
  }
  if ((config & CL_FP_ROUND_TO_INF) != 0) {
    stream << "RoundToInf, ";
  }
  if ((config & CL_FP_FMA) != 0) {
    stream << "FMA, ";
  }
  if ((config & CL_FP_SOFT_FLOAT) != 0) {
    stream << "SoftFloat, ";
  }
  if ((config & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT) != 0) {
    stream << "CorrectDivideSqrt, ";
  }

  auto result = stream.str();
  if (!result.empty()) {
    result.pop_back();
    result.pop_back();
  }

  return result.empty() ? "None" : result;
}

/*!
 * \brief Formats an OpenCL unsigned integer value.
 *
 * \param[in] value OpenCL unsigned integer value.
 * \return Decimal value, or "N/A" for the maximum cl_uint sentinel.
 */
[[nodiscard]] inline auto UIntToString(cl_uint value) -> std::string {
  if (value == std::numeric_limits<cl_uint>::max()) {
    return "N/A";
  }
  return std::to_string(value);
}

/*!
 * \brief Formats a list of OpenCL devices.
 *
 * \param[in] devices Native OpenCL devices.
 * \return Readable device-name list.
 */
[[nodiscard]] inline auto
DevicesToString(std::vector<cl::Device> const &devices) -> std::string {
  std::string output;
  for (auto const &device : devices) {
    output += std::format("{} ", device.getInfo<CL_DEVICE_NAME>());
  }
  return output;
}

/*!
 * \brief Returns the name of an OpenCL device.
 *
 * \param[in] device Native OpenCL device.
 * \return OpenCL device name.
 */
[[nodiscard]] inline auto DeviceToString(cl::Device const &device)
    -> std::string {
  return device.getInfo<CL_DEVICE_NAME>();
}

/*!
 * \brief Formats OpenCL execution capability flags.
 *
 * \param[in] capabilities Execution capability bitfield.
 * \return Readable execution capabilities.
 */
[[nodiscard]] inline auto
ExecCapabilitiesToString(cl_device_exec_capabilities capabilities)
    -> std::string {
  if (capabilities == 0) {
    return "None";
  }

  std::ostringstream stream;
  bool first = true;

  auto const append_capability = [&](std::string_view name) -> void {
    if (!first) {
      stream << ", ";
    }
    stream << name;
    first = false;
  };

  if ((capabilities & CL_EXEC_KERNEL) != 0) {
    append_capability("Kernel execution");
  }
  if ((capabilities & CL_EXEC_NATIVE_KERNEL) != 0) {
    append_capability("Native kernel execution");
  }

  return stream.str();
}

/*!
 * \brief Formats an OpenCL command-queue property array.
 *
 * \param[in] queue_properties OpenCL queue property array.
 * \return Readable queue property list.
 */
[[nodiscard]] inline auto QueuePropertiesArrayToString(
    std::vector<cl_queue_properties> const &queue_properties) -> std::string {
  std::string output;

  for (std::size_t index = 0; index < queue_properties.size(); index += 2) {
    auto property = queue_properties[index];
    if (property == 0) {
      break;
    }

    auto value = queue_properties[index + 1];

    switch (property) {
    case CL_QUEUE_PROPERTIES:
      output += std::format("CL_QUEUE_PROPERTIES = {} ",
                            QueuePropertiesToString(value));
      break;
    default:
      output += std::format("UNKNOWN_PROPERTY({}) = {}", property, value);
      break;
    }
  }

  return output;
}

/*!
 * \brief Formats an OpenCL context property array.
 *
 * \param[in] context_properties OpenCL context property array.
 * \return Readable context property list.
 */
[[nodiscard]] inline auto ContextPropertiesToString(
    std::vector<cl_context_properties> const &context_properties)
    -> std::string {
  std::string output;

  for (std::size_t index = 0; index < context_properties.size(); index += 2) {
    auto property = context_properties[index];
    if (property == 0) {
      break;
    }

    auto value = context_properties[index + 1];
    auto const raw_value = static_cast<std::uintptr_t>(value);

    switch (property) {
    case CL_CONTEXT_PLATFORM:
      output += std::format("CL_CONTEXT_PLATFORM = 0x{:016x} ", raw_value);
      break;

    case CL_GL_CONTEXT_KHR:
      output += std::format("CL_GL_CONTEXT_KHR = 0x{:016x} ", raw_value);
      break;

    default:
      output +=
          std::format("UNKNOWN_PROPERTY({}) = 0x{:016x}", property, raw_value);
      break;
    }
  }

  return output;
}

/*!
 * \brief Converts a kernel-argument address qualifier to text.
 *
 * \param[in] qualifier Kernel-argument address qualifier.
 * \return Readable address qualifier.
 */
[[nodiscard]] inline auto
ArgAddressQualifierToString(cl_kernel_arg_address_qualifier qualifier)
    -> std::string {
  switch (qualifier) {
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

/*!
 * \brief Converts a kernel-argument access qualifier to text.
 *
 * \param[in] qualifier Kernel-argument access qualifier.
 * \return Readable access qualifier.
 */
[[nodiscard]] inline auto
ArgAccessQualifierToString(cl_kernel_arg_access_qualifier qualifier)
    -> std::string {
  switch (qualifier) {
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

/*!
 * \brief Formats kernel-argument type qualifier flags.
 *
 * \param[in] qualifier Kernel-argument type qualifier bitfield.
 * \return Readable type qualifiers.
 */
[[nodiscard]] inline auto
ArgTypeQualifierToString(cl_kernel_arg_type_qualifier qualifier)
    -> std::string {
  std::string output;
  if ((qualifier & CL_KERNEL_ARG_TYPE_CONST) != 0) {
    output += "CL_KERNEL_ARG_TYPE_CONST ";
  }
  if ((qualifier & CL_KERNEL_ARG_TYPE_RESTRICT) != 0) {
    output += "CL_KERNEL_ARG_TYPE_RESTRICT ";
  }
  if ((qualifier & CL_KERNEL_ARG_TYPE_VOLATILE) != 0) {
    output += "CL_KERNEL_ARG_TYPE_VOLATILE ";
  }
  if ((qualifier & CL_KERNEL_ARG_TYPE_PIPE) != 0) {
    output += "CL_KERNEL_ARG_TYPE_PIPE ";
  }
  if ((qualifier & CL_KERNEL_ARG_TYPE_NONE) != 0) {
    output += "CL_KERNEL_ARG_TYPE_NONE ";
  }

  if (!output.empty()) {
    output.erase(output.size() - 1);
  } else {
    output = "None";
  }
  return output;
}
} // namespace ggems::ocl
