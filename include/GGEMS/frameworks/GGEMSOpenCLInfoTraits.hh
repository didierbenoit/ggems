#pragma once

#include <cstdint>
#include <string>
#include <cstddef>
#include <string_view>
#include <vector>

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/core/units/GGEMSBitsUnits.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::ocl {
using namespace ggems::units;

template <cl_uint Info> struct InfoTraits;

template <> struct InfoTraits<CL_PLATFORM_VENDOR> {
  using type = std::string; /*!< C++ type returned by this info query. */

  static constexpr std::string_view name =
      "CL_PLATFORM_VENDOR"; /*!< Symbolic name of this info token. */

  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_PLATFORM_NAME>
 * \brief Traits for \c CL_PLATFORM_NAME.
 */
template <> struct InfoTraits<CL_PLATFORM_NAME> {
  using type = std::string; /*!< C++ type returned by this info query. */

  static constexpr std::string_view name =
      "CL_PLATFORM_NAME"; /*!< Symbolic name of this info token. */

  /*!
   * \brief Convert vendor string to readable format
   * \param v Value returned by \c clGetPlatformInfo.
   * \return Platform name.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_PLATFORM_VERSION>
 * \brief Traits for \c CL_PLATFORM_VERSION.
 */
template <> struct InfoTraits<CL_PLATFORM_VERSION> {
  using type = std::string; /*!< C++ type returned by this info query.*/

  static constexpr std::string_view name =
      "CL_PLATFORM_VERSION"; /*!< Symbolic name of this info tolken. */

  /*!
   * \brief Convert vendor string to readable format
   * \param v Value returned by \c clGetPlatformInfo.
   * \return Platform version
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_PLATFORM_PROFILE>
 * \brief Traits for \c CL_PLATFORM_PROFILE.
 */
template <> struct InfoTraits<CL_PLATFORM_PROFILE> {
  using type = std::string; /*!< C++ type returned by this info query.*/

  static constexpr std::string_view name =
      "CL_PLATFORM_PROFILE"; /*!< Symbolic name of this info tolken. */

  /*!
   * \brief Convert vendor string to readable format
   * \param v Value returned by \c clGetPlatformInfo.
   * \return Platform version
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_PLATFORM_EXTENSIONS>
 * \brief Traits for \c CL_PLATFORM_EXTENSIONS.
 */
template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS> {
  using type = std::string; /*!< C++ type returned by this info query.*/

  static constexpr std::string_view name =
      "CL_PLATFORM_EXTENSIONS"; /*!< Symbolic name of this info token. */

  /*!
   * \brief Convert vendor string to readable format
   * \param v Value returned by \c clGetPlatformInfo.
   * \return Platform version
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_PLATFORM_NUMERIC_VERSION>
 * \brief Traits for \c CL_PLATFORM_NUMERIC_VERSION.
 */
template <> struct InfoTraits<CL_PLATFORM_NUMERIC_VERSION> {
  using type = cl_version; /*!< Numeric OpenCL version. */

  static constexpr std::string_view name =
      "CL_PLATFORM_NUMERIC_VERSION"; /*!< Symbolic name of this info token. */

  /*!
   * \brief Convert numeric version to "major.minor.patch".
   * \param v Version encoded in \c cl_version.
   * \return Human-readable version.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_PLATFORM_HOST_TIMER_RESOLUTION>
 * \brief Traits for \c CL_PLATFORM_HOST_TIMER_RESOLUTION.
 */
template <> struct InfoTraits<CL_PLATFORM_HOST_TIMER_RESOLUTION> {
  using type = cl_ulong; /*!< Resolution in nanoseconds. */

  static constexpr std::string_view name =
      "CL_PLATFORM_HOST_TIMER_RESOLUTION"; /*!< Symbolic name of this info
                                              token. */

  /*!
   * \brief Convert timer resolution to readable string.
   * \param v Timer resolution in ns.
   * \return Human-readable time.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    if (v == 0) {
      return "not supported";
    }
    auto const duration = TryMakeQuantity<Duration>(v, "ns");
    if (!duration.has_value()) {
      return "out of range";
    }
    return HumanReadable(*duration, 0, 3);
  }
};

/*!
 * \struct InfoTraits<CL_PLATFORM_EXTENSIONS_WITH_VERSION>
 * \brief Traits for \c CL_PLATFORM_EXTENSIONS_WITH_VERSION.
 */
template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS_WITH_VERSION> {
  using type = std::vector<cl_name_version>; /*!< List of name/version pairs. */

  static constexpr std::string_view name =
      "CL_PLATFORM_EXTENSIONS_WITH_VERSION"; /*!< Symbolic name of this info
                                                token */

  /*!
   * \brief Convert extension list to readable string.
   * \param v List of name/version descriptors.
   * \return Formatted string.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

// ============================================================================
// Device
// ============================================================================

/*!
 * \struct InfoTraits<CL_DEVICE_EXTENSIONS>
 * \brief Traits for \c CL_DEVICE_EXTENSIONS.
 */
template <> struct InfoTraits<CL_DEVICE_EXTENSIONS> {
  using type = std::string; /*!< Extension list. */

  static constexpr std::string_view name =
      "CL_DEVICE_EXTENSIONS"; /*!< Symbolic name. */

  /*!
   * \brief Convert extension list to readable string.
   * \param v Raw extension list.
   * \return Same string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IL_VERSION>
 * \brief Traits for \c CL_DEVICE_IL_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_IL_VERSION> {
  using type = std::string; /*!< Il version. */

  static constexpr std::string_view name =
      "CL_DEVICE_IL_VERSION"; /*!< Symbolic name. */

  /*!
   * \brief Convert IL version string.
   * \param v Raw version.
   * \return Readable string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_SPIR_VERSIONS>
 * \brief Traits for \c CL_DEVICE_SPIR_VERSIONS.
 */
template <> struct InfoTraits<CL_DEVICE_SPIR_VERSIONS> {
  using type = std::string; /*!< Spir versions. */

  static constexpr std::string_view name =
      "CL_DEVICE_SPIR_VERSIONS"; /*!< Symbolic name. */

  /*!
   * \brief Convert SPIR version list.
   * \param v Raw value.
   * \return Same string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NAME>
 * \brief Traits for \c CL_DEVICE_NAME.
 */
template <> struct InfoTraits<CL_DEVICE_NAME> {
  using type = std::string; /*!< Device name. */

  static constexpr std::string_view name =
      "CL_DEVICE_NAME"; /*!< Symbolic name. */

  /*!
   * \brief Convert device name.
   * \param v Raw value.
   * \return Device name.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_VENDOR>
 * \brief Traits for \c CL_DEVICE_VENDOR.
 */
template <> struct InfoTraits<CL_DEVICE_VENDOR> {
  using type = std::string; /*!< Device vendor. */

  static constexpr std::string_view name =
      "CL_DEVICE_VENDOR"; /*!< Symbolic name. */

  /*!
   * \brief Convert vendor string.
   * \param v Raw value.
   * \return Vendor name.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_VERSION>
 * \brief Traits for \c CL_DEVICE_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_VERSION> {
  using type = std::string; /*!< Device version. */

  static constexpr std::string_view name =
      "CL_DEVICE_VERSION"; /*!< Symbolic name. */

  /*!
   * \brief Convert device version.
   * \param v Raw version string.
   * \return Readable version.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DRIVER_VERSION>
 * \brief Traits for \c CL_DRIVER_VERSION.
 */
template <> struct InfoTraits<CL_DRIVER_VERSION> {
  using type = std::string; /*!< Driver version. */

  static constexpr std::string_view name =
      "CL_DRIVER_VERSION"; /*!< Symbolic name. */

  /*!
   * \brief Convert driver version.
   * \param v Raw value.
   * \return Readable version.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PROFILE>
 * \brief Traits for \c CL_DEVICE_PROFILE.
 */
template <> struct InfoTraits<CL_DEVICE_PROFILE> {
  using type = std::string; /*!< Device profile. */

  static constexpr std::string_view name =
      "CL_DEVICE_PROFILE"; /*!< Symbolic name. */

  /*!
   * \brief Convert profile.
   * \param v Raw profile.
   * \return Same string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_OPENCL_C_VERSION>
 * \brief Traits for \c CL_DEVICE_OPENCL_C_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_VERSION> {
  using type = std::string; /*!< OpenCL C compiler version */

  static constexpr std::string_view name =
      "CL_DEVICE_OPENCL_C_VERSION"; /*!< Symbolic name. */

  /*!
   * \brief Convert OpenCL C version string.
   * \param v Raw string.
   * \return Same string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_ILS_WITH_VERSION>
 * \brief Traits for \c CL_DEVICE_ILS_WITH_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_ILS_WITH_VERSION> {
  using type =
      std::vector<cl_name_version>; /*!< List of IL name/version descriptors. */

  static constexpr std::string_view name =
      "CL_DEVICE_ILS_WITH_VERSION"; /*!< Symbolic name of this OpenCL info
                                       token. */

  /*!
   * \brief Convert IL versions list to readable string.
   * \param v List of IL-version structures.
   * \return Human-readable formatted version list.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_OPENCL_C_ALL_VERSIONS>
 * \brief Traits for \c CL_DEVICE_OPENCL_C_ALL_VERSIONS.
 */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_ALL_VERSIONS> {
  using type =
      std::vector<cl_name_version>; /*!< All supported OpenCL C versions. */

  static constexpr std::string_view name =
      "CL_DEVICE_OPENCL_C_ALL_VERSIONS"; /*!< Symbolic name of this info token.
                                          */

  /*!
   * \brief Convert complete OpenCL C version list to readable string.
   * \param v List of OpenCL C versions with numeric descriptors.
   * \return Formatted version list.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR>
 * \brief Traits for \c CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR.
 */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR> {
  using type = cl_version_khr; /*!< Numeric OpenCL C version encoded in
                                  cl_version_khr. */

  static constexpr std::string_view name =
      "CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR"; /*!< Symbolic name of this info
                                                   token. */

  /*!
   * \brief Convert numeric OpenCL C version to readable string.
   * \param v Version encoded in \c cl_version_khr.
   * \return Human-readable version.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_OPENCL_C_FEATURES>
 * \brief Traits for \c CL_DEVICE_OPENCL_C_FEATURES.
 */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_FEATURES> {
  using type = std::vector<cl_name_version>; /*!< List of OpenCL C feature
                                                descriptors. */

  static constexpr std::string_view name =
      "CL_DEVICE_OPENCL_C_FEATURES"; /*!< Symbolic name of this OpenCL info
                                        token. */

  /*!
   * \brief Convert features list to readable string.
   * \param v List of OpenCL C feature/version descriptors.
   * \return Human-readable formatted information.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT>
 * \brief Traits for \c CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT.
 */
template <> struct InfoTraits<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT> {
  using type = cl_version; /*!< Numeric encoded version. */

  static constexpr std::string_view name =
      "CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT"; /*!< Symbolic token name.
                                                       */

  /*!
   * \brief Convert numeric version to readable string.
   * \param v OpenCL C++ version encoded in \c cl_version.
   * \return Human-readable version number.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NUMERIC_VERSION>
 * \brief Traits for \c CL_DEVICE_NUMERIC_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_NUMERIC_VERSION> {
  using type = cl_version; /*!< Encoded device version. */

  static constexpr std::string_view name =
      "CL_DEVICE_NUMERIC_VERSION"; /*!< Symbolic token name. */

  /*!
   * \brief Convert numeric version to readable string.
   * \param v Numeric version encoded in \c cl_version.
   * \return Human-readable version number.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_VENDOR_ID>
 * \brief Traits for \c CL_DEVICE_VENDOR_ID.
 */
template <> struct InfoTraits<CL_DEVICE_VENDOR_ID> {
  using type = cl_uint; /*!< Vendor numeric identifier. */

  static constexpr std::string_view name =
      "CL_DEVICE_VENDOR_ID"; /*!< Symbolic token name. */

  /*!
   * \brief Convert vendor ID to string.
   * \param v Numeric vendor identifier.
   * \return Vendor description string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return VendorIdToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_TYPE>
 * \brief Traits for \c CL_DEVICE_TYPE.
 */
template <> struct InfoTraits<CL_DEVICE_TYPE> {
  using type = cl_device_type; /*!< Bitfield describing the device type. */

  static constexpr std::string_view name =
      "CL_DEVICE_TYPE"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert device type bitfield to a readable string.
   * \param v Bitfield describing the OpenCL device type.
   * \return Human-readable device type.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DeviceTypeToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_COMPUTE_UNITS>
 * \brief Traits for \c CL_DEVICE_MAX_COMPUTE_UNITS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_COMPUTE_UNITS> {
  using type = cl_uint; /*!< Number of compute units. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_COMPUTE_UNITS"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert compute unit count to string.
   * \param v Number of compute units.
   * \return Decimal string representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_CLOCK_FREQUENCY>
 * \brief Traits for \c CL_DEVICE_MAX_CLOCK_FREQUENCY.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_CLOCK_FREQUENCY> {
  using type = cl_uint; /*!< Clock frequency in MHz. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_CLOCK_FREQUENCY"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert clock frequency to a human-readable string.
   * \param v Frequency in MHz, as returned by OpenCL.
   * \return Human-readable frequency, or CPU frequency fallback if unavailable.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    if (v != 0) {
      auto const frequency = TryMakeQuantity<Frequency>(v, "MHz");
      if (!frequency.has_value()) {
        return "N/A";
      }
      return HumanReadable(*frequency, 1, 5);
    }

    return "N/A";
  }
};

template <> struct InfoTraits<CL_DEVICE_MAX_WORK_GROUP_SIZE> {
  using type = std::size_t;

  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_GROUP_SIZE";

  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>
 * \brief Traits for \c CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS> {
  using type = cl_uint; /*!< Maximum number of supported dimensions. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert dimension count to string.
   * \param v Maximum number of work-item dimensions.
   * \return Decimal string representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_SIZES>
 * \brief Traits for \c CL_DEVICE_MAX_WORK_ITEM_SIZES.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_SIZES> {
  using type = std::vector<std::size_t>; /*!< Work-item size per dimension. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_WORK_ITEM_SIZES"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert dimension sizes to readable string.
   * \param v List of maximum work-item sizes per dimension.
   * \return Formatted string representation.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return SizeToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>
 * \brief Traits for \c CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE> {
  using type = std::size_t; /*!< Preferred work-group size multiple. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE"; /*!< Symbolic name. */

  /*!
   * \brief Convert preferred work-group size multiple to string.
   * \param v Preferred size.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR>
 * \brief Traits for \c CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR> {
  using type = cl_uint; /*!< Preferred vector width for char. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width value to string.
   * \param v Preferred vector width.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT>
 * \brief Traits for \c CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT> {
  using type = cl_uint; /*!< Preferred vector width for short. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT"; /*!< Symbolic OpenCL token name.
                                                 */

  /*!
   * \brief Convert vector width to string.
   * \param v Preferred vector width for \c short.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT>
 * \brief Traits for \c CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT> {
  using type = cl_uint; /*!< Preferred vector width for int. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT"; /*!< Symbolic token name. */

  /*!
   * \brief Convert vector width to string.
   * \param v Preferred vector width for \c int.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG>
 * \brief Traits for \c CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG> {
  using type = cl_uint; /*!< Preferred vector width for long. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG"; /*!< Symbolic token name. */

  /*!
   * \brief Convert vector width to string.
   * \param v Preferred vector width for \c long.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>
 * \brief Traits for \c CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT> {
  using type = cl_uint; /*!< Preferred vector width for float. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT"; /*!< Symbolic OpenCL token name.
                                                 */

  /*!
   * \brief Convert vector width to string.
   * \param v Preferred vector width for \c float.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>
 * \brief Traits for \c CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE> {
  using type = cl_uint; /*!< Preferred vector width for double. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE"; /*!< Symbolic OpenCL token
                                                    name. */

  /*!
   * \brief Convert vector width to string.
   * \param v Preferred vector width for \c double.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF>
 * \brief Traits for \c CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF> {
  using type = cl_uint; /*!< Preferred vector width for half-precision. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF"; /*!< Symbolic OpenCL token name.
                                                */

  /*!
   * \brief Convert vector width to string.
   * \param v Preferred vector width for half-precision.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR>
 * \brief Traits for \c CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR> {
  using type = cl_uint; /*!< Native vector width for char. */

  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width to string.
   * \param v Native vector width for \c char.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT>
 * \brief Traits for \c CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT> {
  using type = cl_uint; /*!< Native vector width for short. */

  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width to string.
   * \param v Native vector width for \c short.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT>
 * \brief Traits for \c CL_DEVICE_NATIVE_VECTOR_WIDTH_INT.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT> {
  using type = cl_uint; /*!< Native vector width for int. */

  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width to string.
   * \param v Native vector width for \c int.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG>
 * \brief Traits for \c CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG> {
  using type = cl_uint; /*!< Native vector width for long. */

  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width to string.
   * \param v Native vector width for \c long.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT>
 * \brief Traits for \c CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT> {
  using type = cl_uint; /*!< Native vector width for float. */

  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width to string.
   * \param v Native vector width for \c float.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>
 * \brief Traits for \c CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE> {
  using type = cl_uint; /*!< Native vector width for double. */

  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width to string.
   * \param v Native vector width for \c double.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF>
 * \brief Traits for \c CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF> {
  using type = cl_uint; /*!< Native vector width for half-precision. */

  static constexpr std::string_view name =
      "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert vector width to string.
   * \param v Native vector width for half-precision types.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_WIDTH>
 * \brief Traits for \c CL_DEVICE_IMAGE2D_MAX_WIDTH.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_WIDTH> {
  using type = std::size_t; /*!< Maximum 2D image width. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE2D_MAX_WIDTH"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert maximum 2D image width to string.
   * \param v Maximum width.
   * \return Decimal string representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_HEIGHT>
 * \brief Traits for \c CL_DEVICE_IMAGE2D_MAX_HEIGHT.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_HEIGHT> {
  using type = std::size_t; /*!< Maximum 2D image height. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE2D_MAX_HEIGHT"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum 2D image height to string.
   * \param v Maximum height.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_WIDTH>
 * \brief Traits for \c CL_DEVICE_IMAGE3D_MAX_WIDTH.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_WIDTH> {
  using type = std::size_t; /*!< Maximum 3D image width. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE3D_MAX_WIDTH"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum 3D image width to string.
   * \param v Maximum width.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_HEIGHT>
 * \brief Traits for \c CL_DEVICE_IMAGE3D_MAX_HEIGHT.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_HEIGHT> {
  using type = std::size_t; /*!< Maximum 3D image height. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE3D_MAX_HEIGHT"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum 3D image height to string.
   * \param v Maximum height.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_DEPTH>
 * \brief Traits for \c CL_DEVICE_IMAGE3D_MAX_DEPTH.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_DEPTH> {
  using type = std::size_t; /*!< Maximum 3D image depth. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE3D_MAX_DEPTH"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum 3D image depth to string.
   * \param v Maximum depth.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE>
 * \brief Traits for \c CL_DEVICE_IMAGE_MAX_BUFFER_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE> {
  using type = std::size_t; /*!< Maximum image buffer size. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_MAX_BUFFER_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum buffer size to string.
   * \param v Maximum buffer size.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE>
 * \brief Traits for \c CL_DEVICE_IMAGE_MAX_ARRAY_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE> {
  using type = std::size_t; /*!< Maximum image array size. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_MAX_ARRAY_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum array size to string.
   * \param v Maximum array size.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE_SUPPORT>
 * \brief Traits for \c CL_DEVICE_IMAGE_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_SUPPORT> {
  using type = cl_bool; /*!< Boolean indicating image support. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_SUPPORT"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert image support flag to readable string.
   * \param v OpenCL boolean value.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_READ_IMAGE_ARGS>
 * \brief Traits for \c CL_DEVICE_MAX_READ_IMAGE_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_READ_IMAGE_ARGS> {
  using type = cl_uint; /*!< Maximum number of read image arguments. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_READ_IMAGE_ARGS"; /*!< Symbolic token name. */

  /*!
   * \brief Convert maximum number of read image arguments to string.
   * \param v Maximum number of supported read image arguments.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>
 * \brief Traits for \c CL_DEVICE_MAX_WRITE_IMAGE_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_WRITE_IMAGE_ARGS> {
  using type = cl_uint; /*!< Maximum number of write image arguments. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_WRITE_IMAGE_ARGS"; /*!< Symbolic token name. */

  /*!
   * \brief Convert maximum number of write image arguments to string.
   * \param v Maximum number of supported write image arguments.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS>
 * \brief Traits for \c CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS> {
  using type = cl_uint; /*!< Maximum RW image argument count. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS"; /*!< Symbolic token name. */

  /*!
   * \brief Convert maximum number of RW image arguments to string.
   * \param v Maximum number of supported read/write image arguments.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE_PITCH_ALIGNMENT>
 * \brief Traits for \c CL_DEVICE_IMAGE_PITCH_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_PITCH_ALIGNMENT> {
  using type = cl_uint; /*!< Required image row pitch alignment. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_PITCH_ALIGNMENT"; /*!< Symbolic token name. */

  /*!
   * \brief Convert pitch alignment to string.
   * \param v Required alignment.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT>
 * \brief Traits for \c CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT> {
  using type = cl_uint; /*!< Required base address alignment. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT"; /*!< Symbolic token name. */

  /*!
   * \brief Convert base address alignment requirement to string.
   * \param v Alignment requirement.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_SAMPLERS>
 * \brief Traits for \c CL_DEVICE_MAX_SAMPLERS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_SAMPLERS> {
  using type = cl_uint; /*!< Maximum sampler count. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_SAMPLERS"; /*!< Symbolic token name. */

  /*!
   * \brief Convert sampler count to string.
   * \param v Maximum number of supported samplers.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_GLOBAL_MEM_SIZE>
 * \brief Traits for \c CL_DEVICE_GLOBAL_MEM_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_SIZE> {
  using type = cl_ulong; /*!< Size of global memory in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_MEM_SIZE"; /*!< Symbolic OpenCL token name. */

  static constexpr std::string_view unit =
      " bytes"; /*!< Unit of the returned value. */

  /*!
   * \brief Convert global memory size to a readable string.
   * \param v Size of global memory in bytes.
   * \return Human-readable memory size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>
 * \brief Traits for \c CL_DEVICE_GLOBAL_MEM_CACHE_TYPE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE> {
  using type = cl_device_mem_cache_type; /*!< Cache type enum. */

  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert cache type to a readable string.
   * \param v Cache type enumeration.
   * \return Human-readable cache type.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return CacheTypeToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>
 * \brief Traits for \c CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE> {
  using type = cl_uint; /*!< Cacheline size in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert cacheline size to readable string.
   * \param v Cacheline size in bytes.
   * \return Human-readable size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>
 * \brief Traits for \c CL_DEVICE_GLOBAL_MEM_CACHE_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE> {
  using type = cl_ulong; /*!< Global cache size in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert cache size to a readable string.
   * \param v Cache size in bytes.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_LOCAL_MEM_SIZE>
 * \brief Traits for \c CL_DEVICE_LOCAL_MEM_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_SIZE> {
  using type = cl_ulong; /*!< Local memory size in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_LOCAL_MEM_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert local memory size to readable string.
   * \param v Local memory size in bytes.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_LOCAL_MEM_TYPE>
 * \brief Traits for \c CL_DEVICE_LOCAL_MEM_TYPE.
 */
template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_TYPE> {
  using type = cl_device_local_mem_type; /*!< Local memory type enum. */

  static constexpr std::string_view name =
      "CL_DEVICE_LOCAL_MEM_TYPE"; /*!< Symbolic token name. */

  static constexpr std::string_view unit =
      ""; /*!< No unit associated with this type. */

  /*!
   * \brief Convert local memory type to readable string.
   * \param v Local memory type enum.
   * \return Human-readable description.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return LocalMemTypeToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_MEM_ALLOC_SIZE>
 * \brief Traits for \c CL_DEVICE_MAX_MEM_ALLOC_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_MEM_ALLOC_SIZE> {
  using type = cl_ulong; /*!< Maximum allocatable memory block in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_MEM_ALLOC_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum allocation size to readable string.
   * \param v Maximum allocation size in bytes.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>
 * \brief Traits for \c CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE> {
  using type = cl_ulong; /*!< Constant buffer size in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert constant buffer size to readable string.
   * \param v Maximum constant buffer size in bytes.
   * \return Human-readable memory size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_CONSTANT_ARGS>
 * \brief Traits for \c CL_DEVICE_MAX_CONSTANT_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_ARGS> {
  using type = cl_uint; /*!< Maximum number of constant arguments. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_CONSTANT_ARGS"; /*!< Symbolic token. */

  /*!
   * \brief Convert constant argument limit to string.
   * \param v Maximum number of constant kernel arguments.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MEM_BASE_ADDR_ALIGN>
 * \brief Traits for \c CL_DEVICE_MEM_BASE_ADDR_ALIGN.
 */
template <> struct InfoTraits<CL_DEVICE_MEM_BASE_ADDR_ALIGN> {
  using type = cl_uint; /*!< Base address alignment in bits. */

  static constexpr std::string_view name =
      "CL_DEVICE_MEM_BASE_ADDR_ALIGN"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert alignment value to a readable string.
   * \param v Alignment value in bits.
   * \return Human-readable alignment.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bits bits{static_cast<std::uint64_t>(v)};
    return HumanReadable(bits, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE>
 * \brief Traits for \c CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE> {
  using type = cl_uint; /*!< Minimum alignment size in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert minimum alignment size to readable string.
   * \param v Minimum alignment size in bytes.
   * \return Human-readable alignment.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_HOST_UNIFIED_MEMORY>
 * \brief Traits for \c CL_DEVICE_HOST_UNIFIED_MEMORY.
 */
template <> struct InfoTraits<CL_DEVICE_HOST_UNIFIED_MEMORY> {
  using type =
      cl_bool; /*!< Boolean indicating host–device memory unification. */

  static constexpr std::string_view name =
      "CL_DEVICE_HOST_UNIFIED_MEMORY"; /*!< Symbolic token. */

  /*!
   * \brief Convert unification flag to readable string.
   * \param v Boolean value.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES>
 * \brief Traits for \c CL_DEVICE_QUEUE_ON_HOST_PROPERTIES.
 */
template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES> {
  using type =
      cl_command_queue_properties; /*!< Host-side queue properties bitfield. */

  static constexpr std::string_view name =
      "CL_DEVICE_QUEUE_ON_HOST_PROPERTIES"; /*!< Symbolic token. */

  /*!
   * \brief Decode command queue properties.
   * \param v Bitfield describing host queue properties.
   * \return Human-readable list of properties.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES>
 * \brief Traits for \c CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES.
 */
template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES> {
  using type =
      cl_command_queue_properties; /*!< Device-side queue properties. */

  static constexpr std::string_view name =
      "CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES"; /*!< Symbolic token. */

  /*!
   * \brief Decode device-side queue properties.
   * \param v Bitfield describing device queue properties.
   * \return Human-readable list of properties.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE>
 * \brief Traits for \c CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE> {
  using type = cl_uint; /*!< Preferred queue size in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE"; /*!< Symbolic token. */

  static constexpr std::string_view unit =
      " bytes"; /*!< Unit of the returned size. */

  /*!
   * \brief Convert preferred queue size to readable string.
   * \param v Preferred size in bytes.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_QUEUES>
 * \brief Traits for \c CL_DEVICE_MAX_ON_DEVICE_QUEUES.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_QUEUES> {
  using type = cl_uint; /*!< Maximum number of on-device queues. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_ON_DEVICE_QUEUES"; /*!< Symbolic token. */

  /*!
   * \brief Convert queue count to string.
   * \param v Maximum number of device-managed queues.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_EVENTS>
 * \brief Traits for \c CL_DEVICE_MAX_ON_DEVICE_EVENTS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_EVENTS> {
  using type = cl_uint; /*!< Maximum number of events supported on-device. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_ON_DEVICE_EVENTS"; /*!< Symbolic token. */

  /*!
   * \brief Convert event count to string.
   * \param v Maximum number of supported device events.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_SVM_CAPABILITIES>
 * \brief Traits for \c CL_DEVICE_SVM_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_SVM_CAPABILITIES> {
  using type = cl_device_svm_capabilities; /*!< Bitfield of SVM capabilities. */

  static constexpr std::string_view name =
      "CL_DEVICE_SVM_CAPABILITIES"; /*!< Symbolic token. */

  /*!
   * \brief Decode SVM capabilities bitfield.
   * \param v SVM capabilities.
   * \return Human-readable list of supported SVM modes.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return SVMCapabilitiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES>
 * \brief Traits for \c CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES> {
  using type =
      cl_device_atomic_capabilities; /*!< Atomic memory capability bitfield. */

  static constexpr std::string_view name =
      "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES"; /*!< Symbolic token. */

  /*!
   * \brief Convert atomic memory capabilities to readable form.
   * \param v Bitfield describing atomic memory capabilities.
   * \return Human-readable capability list.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return AtomicCapabilitiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES>
 * \brief Traits for \c CL_DEVICE_ATOMIC_FENCE_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES> {
  using type =
      cl_device_atomic_capabilities; /*!< Atomic fence capability bitfield. */

  static constexpr std::string_view name =
      "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES"; /*!< Symbolic token. */

  /*!
   * \brief Convert atomic fence capabilities to readable form.
   * \param v Bitfield describing fence capabilities.
   * \return Human-readable capability list.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return AtomicCapabilitiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_NUM_SUB_GROUPS>
 * \brief Traits for \c CL_DEVICE_MAX_NUM_SUB_GROUPS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_NUM_SUB_GROUPS> {
  using type = cl_uint; /*!< Maximum number of sub-groups. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_NUM_SUB_GROUPS"; /*!< Symbolic token name. */

  /*!
   * \brief Convert maximum number of sub-groups to string.
   * \param v Maximum number of sub-groups.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS>
 * \brief Traits for \c CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS.
 */
template <>
struct InfoTraits<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS> {
  using type = cl_bool; /*!< Boolean indicating independent forward progress. */

  static constexpr std::string_view name =
      "CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS"; /*!< Symbolic token.
                                                           */

  /*!
   * \brief Convert boolean value to readable string.
   * \param v Boolean flag.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_EXECUTION_CAPABILITIES>
 * \brief Traits for \c CL_DEVICE_EXECUTION_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_EXECUTION_CAPABILITIES> {
  using type = cl_device_exec_capabilities; /*!< Execution capability flags. */

  static constexpr std::string_view name =
      "CL_DEVICE_EXECUTION_CAPABILITIES"; /*!< Symbolic token. */

  /*!
   * \brief Decode execution capability flags.
   * \param v Bitfield describing execution capabilities.
   * \return Human-readable capability list.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ExecCapabilitiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT>
 * \brief Traits for \c CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT> {
  using type = cl_bool; /*!< Boolean indicating non-uniform WG support. */

  static constexpr std::string_view name =
      "CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT"; /*!< Symbolic token. */

  /*!
   * \brief Convert boolean to human-readable form.
   * \param v Boolean flag.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT>
 * \brief Traits for \c CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT.
 */
template <>
struct InfoTraits<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT> {
  using type = cl_bool; /*!< Boolean indicating WG collective support. */

  static constexpr std::string_view name =
      "CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT"; /*!< Symbolic token.
                                                            */

  /*!
   * \brief Convert boolean to readable string.
   * \param v Boolean flag.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT>
 * \brief Traits for \c CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT> {
  using type = cl_bool; /*!< Indicates support for generic address space. */

  static constexpr std::string_view name =
      "CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT"; /*!< Symbolic token. */

  /*!
   * \brief Convert flag to readable string.
   * \param v Boolean value.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES>
 * \brief Traits for \c CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES> {
  using type = cl_device_device_enqueue_capabilities; /*!< Device enqueue
                                                         capability flags. */

  static constexpr std::string_view name =
      "CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES"; /*!< Symbolic token. */

  /*!
   * \brief Decode enqueue capability bitfield.
   * \param v Bitfield describing enqueue capabilities.
   * \return Human-readable list of capabilities.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DeviceEnqueueCapabilitiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PARTITION_MAX_SUB_DEVICES>
 * \brief Traits for \c CL_DEVICE_PARTITION_MAX_SUB_DEVICES.
 */
template <> struct InfoTraits<CL_DEVICE_PARTITION_MAX_SUB_DEVICES> {
  using type = cl_uint; /*!< Maximum number of sub-devices. */

  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_MAX_SUB_DEVICES"; /*!< Symbolic token. */

  /*!
   * \brief Convert sub-device count to string.
   * \param v Maximum number of sub-devices.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PARTITION_PROPERTIES>
 * \brief Traits for \c CL_DEVICE_PARTITION_PROPERTIES.
 */
template <> struct InfoTraits<CL_DEVICE_PARTITION_PROPERTIES> {
  using type =
      std::vector<cl_device_partition_property>; /*!< Supported partition
                                                    properties. */

  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_PROPERTIES"; /*!< Symbolic token. */

  /*!
   * \brief Convert partition properties to readable string.
   * \param v List of partition properties.
   * \return Formatted property list.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return PartitionPropertiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PARTITION_AFFINITY_DOMAIN>
 * \brief Traits for \c CL_DEVICE_PARTITION_AFFINITY_DOMAIN.
 */
template <> struct InfoTraits<CL_DEVICE_PARTITION_AFFINITY_DOMAIN> {
  using type = cl_device_affinity_domain; /*!< Affinity domain bitfield. */

  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_AFFINITY_DOMAIN"; /*!< Symbolic token. */

  /*!
   * \brief Convert affinity domain bitfield to readable string.
   * \param v Affinity domain bitfield.
   * \return Human-readable domain description.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return AffinityDomainToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PARTITION_TYPE>
 * \brief Traits for \c CL_DEVICE_PARTITION_TYPE.
 */
template <> struct InfoTraits<CL_DEVICE_PARTITION_TYPE> {
  using type =
      std::vector<cl_device_partition_property>; /*!< Partition type list. */

  static constexpr std::string_view name =
      "CL_DEVICE_PARTITION_TYPE"; /*!< Symbolic token. */

  /*!
   * \brief Convert partition type list to readable string.
   * \param v Partition type vector.
   * \return Human-readable list.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return PartitionPropertiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_PARAMETER_SIZE>
 * \brief Traits for \c CL_DEVICE_MAX_PARAMETER_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_PARAMETER_SIZE> {
  using type = std::size_t; /*!< Maximum size of kernel arguments. */

  static constexpr std::string_view name =
      "CL_DEVICE_IMAGE_MAX_PARAMETER_SIZE"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Convert parameter size to string.
   * \param v Maximum kernel parameter size.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return std::to_string(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_EXTENSIONS_WITH_VERSION>
 * \brief Traits for \c CL_DEVICE_EXTENSIONS_WITH_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_EXTENSIONS_WITH_VERSION> {
  using type = std::vector<cl_name_version>; /*!< List of extension/version
                                                descriptors. */

  static constexpr std::string_view name =
      "CL_DEVICE_EXTENSIONS_WITH_VERSION"; /*!< Symbolic token. */

  /*!
   * \brief Format extension list into readable string.
   * \param v Vector of name/version extension info.
   * \return Human-readable formatted list.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED>
 * \brief Traits for \c CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED.
 */
template <> struct InfoTraits<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED> {
  using type = std::string; /*!< Conformance version string. */

  static constexpr std::string_view name =
      "CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED"; /*!< Symbolic token. */

  /*!
   * \brief Return raw conformance version string.
   * \param v Version string.
   * \return Version string unchanged.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS>
 * \brief Traits for \c CL_DEVICE_BUILT_IN_KERNELS.
 */
template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS> {
  using type = std::string; /*!< Comma-separated list of built-in kernels. */

  static constexpr std::string_view name =
      "CL_DEVICE_BUILT_IN_KERNELS"; /*!< Symbolic OpenCL token name. */

  /*!
   * \brief Return raw kernel list string.
   * \param v Built-in kernel list.
   * \return String unchanged.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION>
 * \brief Traits for \c CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION> {
  using type = std::vector<cl_name_version>; /*!< Kernel/version descriptors. */

  static constexpr std::string_view name =
      "CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION"; /*!< Symbolic token. */

  /*!
   * \brief Convert built-in kernel version list to readable string.
   * \param v Vector of kernel/version descriptors.
   * \return Formatted string.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return ClNameVersionToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT>
 * \brief Traits for \c CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT> {
  using type = cl_uint; /*!< Preferred atomic alignment for platform SVM. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT"; /*!< Symbolic token. */

  static constexpr std::string_view unit =
      " bytes"; /*!< Unit of the returned value. */

  /*!
   * \brief Convert atomic alignment size to readable string.
   * \param v Alignment in bytes.
   * \return Human-readable size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT>
 * \brief Traits for \c CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT> {
  using type = cl_uint; /*!< Preferred atomic alignment for global memory. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT"; /*!< Symbolic token. */

  /*!
   * \brief Convert atomic alignment to readable string.
   * \param v Alignment in bytes.
   * \return Human-readable size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT>
 * \brief Traits for \c CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT> {
  using type = cl_uint; /*!< Preferred atomic alignment for local memory. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT"; /*!< Symbolic token. */

  /*!
   * \brief Convert atomic alignment to readable size.
   * \param v Alignment in bytes.
   * \return Human-readable string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_ADDRESS_BITS>
 * \brief Traits for \c CL_DEVICE_ADDRESS_BITS.
 */
template <> struct InfoTraits<CL_DEVICE_ADDRESS_BITS> {
  using type = cl_uint; /*!< Number of address bits. */

  static constexpr std::string_view name =
      "CL_DEVICE_ADDRESS_BITS"; /*!< Symbolic token. */

  static constexpr std::string_view unit =
      " bits"; /*!< Unit of the returned value. */

  /*!
   * \brief Convert address width to readable string.
   * \param v Address width in bits.
   * \return Human-readable size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bits B = Bits{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 0, 3);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PROFILING_TIMER_RESOLUTION>
 * \brief Traits for \c CL_DEVICE_PROFILING_TIMER_RESOLUTION.
 */
template <> struct InfoTraits<CL_DEVICE_PROFILING_TIMER_RESOLUTION> {
  using type = std::size_t; /*!< Timer resolution in nanoseconds. */

  static constexpr std::string_view name =
      "CL_DEVICE_PROFILING_TIMER_RESOLUTION"; /*!< Symbolic token. */

  /*!
   * \brief Convert timer resolution to readable string.
   * \param v Timer resolution in nanoseconds.
   * \return Human-readable time representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    auto const duration = TryMakeQuantity<Duration>(v, "ns");
    if (!duration.has_value()) {
      return "out of range";
    }
    return HumanReadable(*duration, 0, 3);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_COMPILER_AVAILABLE>
 * \brief Traits for \c CL_DEVICE_COMPILER_AVAILABLE.
 */
template <> struct InfoTraits<CL_DEVICE_COMPILER_AVAILABLE> {
  using type = cl_bool; /*!< Indicates availability of device compiler. */

  static constexpr std::string_view name =
      "CL_DEVICE_COMPILER_AVAILABLE"; /*!< Symbolic token. */

  /*!
   * \brief Convert boolean to readable string.
   * \param v Boolean value.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_LINKER_AVAILABLE>
 * \brief Traits for \c CL_DEVICE_LINKER_AVAILABLE.
 */
template <> struct InfoTraits<CL_DEVICE_LINKER_AVAILABLE> {
  using type = cl_bool; /*!< Indicates availability of device linker. */

  static constexpr std::string_view name =
      "CL_DEVICE_LINKER_AVAILABLE"; /*!< Symbolic token. */

  /*!
   * \brief Convert boolean to readable form.
   * \param v Boolean value.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_AVAILABLE>
 * \brief Traits for \c CL_DEVICE_AVAILABLE.
 */
template <> struct InfoTraits<CL_DEVICE_AVAILABLE> {
  using type = cl_bool; /*!< True if device is available for computation. */

  static constexpr std::string_view name =
      "CL_DEVICE_AVAILABLE"; /*!< Symbolic token. */

  /*!
   * \brief Convert availability flag to readable string.
   * \param v Boolean flag.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_ENDIAN_LITTLE>
 * \brief Traits for \c CL_DEVICE_ENDIAN_LITTLE.
 */
template <> struct InfoTraits<CL_DEVICE_ENDIAN_LITTLE> {
  using type = cl_bool; /*!< Boolean indicating little-endian architecture. */

  static constexpr std::string_view name =
      "CL_DEVICE_ENDIAN_LITTLE"; /*!< Symbolic token. */

  /*!
   * \brief Convert endianness to readable form.
   * \param v Boolean value.
   * \return "YES" for little-endian, "NO" otherwise.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_ERROR_CORRECTION_SUPPORT>
 * \brief Traits for \c CL_DEVICE_ERROR_CORRECTION_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_ERROR_CORRECTION_SUPPORT> {
  using type = cl_bool; /*!< Boolean indicating ECC memory support. */

  static constexpr std::string_view name =
      "CL_DEVICE_ERROR_CORRECTION_SUPPORT"; /*!< Symbolic token. */

  /*!
   * \brief Convert ECC support flag to readable string.
   * \param v Boolean value.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PRINTF_BUFFER_SIZE>
 * \brief Traits for \c CL_DEVICE_PRINTF_BUFFER_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_PRINTF_BUFFER_SIZE> {
  using type = std::size_t; /*!< Printf buffer size in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_PRINTF_BUFFER_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert printf buffer size to readable string.
   * \param v Printf buffer size in bytes.
   * \return Human-readable size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_PIPE_ARGS>
 * \brief Traits for \c CL_DEVICE_MAX_PIPE_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_PIPE_ARGS> {
  using type = cl_uint; /*!< Maximum number of pipe kernel arguments. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_PIPE_ARGS"; /*!< Symbolic token. */

  /*!
   * \brief Convert maximum number of pipe arguments to string.
   * \param v Maximum number of pipe arguments.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS>
 * \brief Traits for \c CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS.
 */
template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS> {
  using type = cl_uint; /*!< Maximum active pipe reservations. */

  static constexpr std::string_view name =
      "CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS"; /*!< Symbolic token. */

  /*!
   * \brief Convert reservation limit to string.
   * \param v Maximum active reservations.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PIPE_MAX_PACKET_SIZE>
 * \brief Traits for \c CL_DEVICE_PIPE_MAX_PACKET_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_PACKET_SIZE> {
  using type = cl_uint; /*!< Maximum size of pipe packet. */

  static constexpr std::string_view name =
      "CL_DEVICE_PIPE_MAX_PACKET_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert packet size to readable string.
   * \param v Packet size in bytes.
   * \return Human-readable size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PIPE_SUPPORT>
 * \brief Traits for \c CL_DEVICE_PIPE_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_PIPE_SUPPORT> {
  using type = cl_bool; /*!< Indicates availability of pipe functionality. */

  static constexpr std::string_view name =
      "CL_DEVICE_PIPE_SUPPORT"; /*!< Symbolic token. */

  /*!
   * \brief Convert pipe support flag to readable string.
   * \param v Boolean flag.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE>
 * \brief Traits for \c CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE> {
  using type = std::size_t; /*!< Max size of global-scope variables in bytes. */

  static constexpr std::string_view name =
      "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert size to readable string.
   * \param v Size in bytes.
   * \return Human-readable size.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE>
 * \brief Traits for \c CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE> {
  using type = std::size_t; /*!< Preferred total size of global variables. */

  static constexpr std::string_view name =
      "CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert preferred total size to readable string.
   * \param v Size in bytes.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    Bytes B = Bytes{static_cast<std::uint64_t>(v)};
    return HumanReadable(B, 1, 5);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_UUID_KHR>
 * \brief Traits for \c CL_DEVICE_UUID_KHR.
 */
template <> struct InfoTraits<CL_DEVICE_UUID_KHR> {
  using type =
      std::array<cl_uchar, CL_UUID_SIZE_KHR>; /*!< Device UUID byte array. */

  static constexpr std::string_view name =
      "CL_DEVICE_UUID_KHR"; /*!< Symbolic OpenCL token. */

  /*!
   * \brief Convert device UUID to readable string.
   * \param v UUID byte array.
   * \return Hexadecimal UUID string.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return UUIDToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DRIVER_UUID_KHR>
 * \brief Traits for \c CL_DRIVER_UUID_KHR.
 */
template <> struct InfoTraits<CL_DRIVER_UUID_KHR> {
  using type =
      std::array<cl_uchar, CL_UUID_SIZE_KHR>; /*!< Driver UUID byte array. */

  static constexpr std::string_view name =
      "CL_DRIVER_UUID_KHR"; /*!< Symbolic token. */

  /*!
   * \brief Convert driver UUID to readable string.
   * \param v UUID byte array.
   * \return Human-readable hexadecimal string.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return UUIDToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_LUID_VALID_KHR>
 * \brief Traits for \c CL_DEVICE_LUID_VALID_KHR.
 */
template <> struct InfoTraits<CL_DEVICE_LUID_VALID_KHR> {
  using type = cl_bool; /*!< Boolean indicating whether LUID is valid. */

  static constexpr std::string_view name =
      "CL_DEVICE_LUID_VALID_KHR"; /*!< Symbolic token. */

  /*!
   * \brief Convert LUID validity flag to readable string.
   * \param v Boolean flag.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_LUID_KHR>
 * \brief Traits for \c CL_DEVICE_LUID_KHR.
 */
template <> struct InfoTraits<CL_DEVICE_LUID_KHR> {
  using type = std::array<cl_uchar, CL_LUID_SIZE_KHR>; /*!< LUID byte array. */

  static constexpr std::string_view name =
      "CL_DEVICE_LUID_KHR"; /*!< Symbolic token. */

  /*!
   * \brief Convert device LUID to readable string.
   * \param v Byte array representing the LUID.
   * \return Human-readable formatted LUID string.
   */
  [[nodiscard]] static std::string ToString(type const &v) noexcept {
    return LUIDToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_HALF_FP_CONFIG>
 * \brief Traits for \c CL_DEVICE_HALF_FP_CONFIG.
 */
template <> struct InfoTraits<CL_DEVICE_HALF_FP_CONFIG> {
  using type =
      cl_device_fp_config; /*!< Bitmask of half-precision FP capabilities. */

  static constexpr std::string_view name =
      "CL_DEVICE_HALF_FP_CONFIG"; /*!< Symbolic token. */

  /*!
   * \brief Convert half-precision FP capabilities to string.
   * \param v Bitmask of FP capabilities.
   * \return Readable description.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return FPConfigToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_SINGLE_FP_CONFIG>
 * \brief Traits for \c CL_DEVICE_SINGLE_FP_CONFIG.
 */
template <> struct InfoTraits<CL_DEVICE_SINGLE_FP_CONFIG> {
  using type =
      cl_device_fp_config; /*!< Bitmask of single-precision FP capabilities. */

  static constexpr std::string_view name =
      "CL_DEVICE_SINGLE_FP_CONFIG"; /*!< Symbolic token. */

  /*!
   * \brief Convert single-precision FP capabilities to string.
   * \param v Bitmask of FP capabilities.
   * \return Readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return FPConfigToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_DOUBLE_FP_CONFIG>
 * \brief Traits for \c CL_DEVICE_DOUBLE_FP_CONFIG.
 */
template <> struct InfoTraits<CL_DEVICE_DOUBLE_FP_CONFIG> {
  using type =
      cl_device_fp_config; /*!< Bitmask of double-precision FP capabilities. */

  static constexpr std::string_view name =
      "CL_DEVICE_DOUBLE_FP_CONFIG"; /*!< Symbolic token. */

  /*!
   * \brief Convert double-precision FP capabilities to string.
   * \param v Bitmask of FP capabilities.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return FPConfigToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_REFERENCE_COUNT>
 * \brief Traits for \c CL_DEVICE_REFERENCE_COUNT.
 */
template <> struct InfoTraits<CL_DEVICE_REFERENCE_COUNT> {
  using type = cl_uint; /*!< Reference count. */

  static constexpr std::string_view name =
      "CL_DEVICE_REFERENCE_COUNT"; /*!< Symbolic token. */

  /*!
   * \brief Convert reference count to string.
   * \param v Reference count.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC>
 * \brief Traits for \c CL_DEVICE_PREFERRED_INTEROP_USER_SYNC.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC> {
  using type = cl_bool; /*!< Boolean flag. */

  static constexpr std::string_view name =
      "CL_DEVICE_PREFERRED_INTEROP_USER_SYNC"; /*!< Symbolic token. */

  /*!
   * \brief Convert boolean synchronisation preference to string.
   * \param v Boolean value.
   * \return "YES" or "NO".
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ClBoolToString(v);
  }
};

// =============================================================================
// === Context
// =============================================================================

/*!
 * \struct InfoTraits<CL_CONTEXT_REFERENCE_COUNT>
 * \brief Traits for \c CL_CONTEXT_REFERENCE_COUNT.
 */
template <> struct InfoTraits<CL_CONTEXT_REFERENCE_COUNT> {
  using type = cl_uint; /*!< Reference count for the context. */

  static constexpr std::string_view name =
      "CL_CONTEXT_REFERENCE_COUNT"; /*!< Symbolic token. */

  /*!
   * \brief Convert reference count to string.
   * \param v Reference count.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_CONTEXT_NUM_DEVICES>
 * \brief Traits for \c CL_CONTEXT_NUM_DEVICES.
 */
template <> struct InfoTraits<CL_CONTEXT_NUM_DEVICES> {
  using type = cl_uint; /*!< Number of devices. */

  static constexpr std::string_view name =
      "CL_CONTEXT_NUM_DEVICES"; /*!< Symbolic token. */

  /*!
   * \brief Convert device count to string.
   * \param v Number of devices.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_CONTEXT_DEVICES>
 * \brief Traits for \c CL_CONTEXT_DEVICES.
 */
template <> struct InfoTraits<CL_CONTEXT_DEVICES> {
  using type = std::vector<cl::Device>; /*!< Vector of OpenCL devices. */

  static constexpr std::string_view name =
      "CL_CONTEXT_DEVICES"; /*!< Symbolic token. */

  /*!
   * \brief Convert device list to readable string.
   * \param v List of devices.
   * \return Human-readable description.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DevicesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_CONTEXT_PROPERTIES>
 * \brief Traits for \c CL_CONTEXT_PROPERTIES.
 */
template <> struct InfoTraits<CL_CONTEXT_PROPERTIES> {
  using type =
      std::vector<cl_context_properties>; /*!< Vector of context properties. */

  static constexpr std::string_view name =
      "CL_CONTEXT_PROPERTIES"; /*!< Symbolic token. */

  /*!
   * \brief Convert context properties to string.
   * \param v Vector of context properties.
   * \return Human-readable property description.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return ContextPropertiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_QUEUE_CONTEXT>
 * \brief Traits for \c CL_QUEUE_CONTEXT.
 */
template <> struct InfoTraits<CL_QUEUE_CONTEXT> {
  using type = cl::Context; /*!< OpenCL context object. */

  static constexpr std::string_view name =
      "CL_QUEUE_CONTEXT"; /*!< Symbolic token. */

  /*!
   * \brief Convert context to string.
   * \param v Queue context (unused).
   * \return Empty string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    (void)v;
    return "";
  }
};

/*!
 * \struct InfoTraits<CL_QUEUE_DEVICE>
 * \brief Traits for \c CL_QUEUE_DEVICE.
 */
template <> struct InfoTraits<CL_QUEUE_DEVICE> {
  using type = cl::Device; /*!< Device associated with the queue. */

  static constexpr std::string_view name =
      "CL_QUEUE_DEVICE"; /*!< Symbolic token. */

  /*!
   * \brief Convert device to readable string.
   * \param v OpenCL device.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return DeviceToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_QUEUE_REFERENCE_COUNT>
 * \brief Traits for \c CL_QUEUE_REFERENCE_COUNT.
 */
template <> struct InfoTraits<CL_QUEUE_REFERENCE_COUNT> {
  using type = cl_uint; /*!< Reference count. */

  static constexpr std::string_view name =
      "CL_QUEUE_REFERENCE_COUNT"; /*!< Symbolic token. */

  /*!
   * \brief Convert reference count to string.
   * \param v Queue reference count.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_QUEUE_SIZE>
 * \brief Traits for \c CL_QUEUE_SIZE.
 */
template <> struct InfoTraits<CL_QUEUE_SIZE> {
  using type = cl_uint; /*!< Queue size. */

  static constexpr std::string_view name =
      "CL_QUEUE_SIZE"; /*!< Symbolic token. */

  /*!
   * \brief Convert queue size to string.
   * \param v Queue size.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_QUEUE_PROPERTIES>
 * \brief Traits for \c CL_QUEUE_PROPERTIES.
 */
template <> struct InfoTraits<CL_QUEUE_PROPERTIES> {
  using type = cl_command_queue_properties; /*!< Queue properties bitmask. */

  static constexpr std::string_view name =
      "CL_QUEUE_PROPERTIES"; /*!< Symbolic token. */

  /*!
   * \brief Convert queue properties to string.
   * \param v Queue property mask.
   * \return Human-readable description.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_QUEUE_PROPERTIES_ARRAY>
 * \brief Traits for \c CL_QUEUE_PROPERTIES_ARRAY.
 */
template <> struct InfoTraits<CL_QUEUE_PROPERTIES_ARRAY> {
  using type =
      std::vector<cl_queue_properties>; /*!< List of queue properties. */

  static constexpr std::string_view name =
      "CL_QUEUE_PROPERTIES_ARRAY"; /*!< Symbolic token. */

  /*!
   * \brief Convert queue property list to string.
   * \param v Vector of queue properties.
   * \return Human-readable representation.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return QueuePropertiesArrayToString(v);
  }
};

// =============================================================================
// === Program
// =============================================================================

/*!
 * \struct InfoTraits<CL_PROGRAM_NUM_DEVICES>
 * \brief Traits for \c CL_PROGRAM_NUM_DEVICES.
 */
template <> struct InfoTraits<CL_PROGRAM_NUM_DEVICES> {
  using type = cl_uint; /*!< Number of devices. */

  static constexpr std::string_view name =
      "CL_PROGRAM_NUM_DEVICES"; /*!< Symbolic token. */

  /*!
   * \brief Convert number of devices to string.
   * \param v Device count.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_PROGRAM_BINARY_SIZES>
 * \brief Traits for \c CL_PROGRAM_BINARY_SIZES.
 */
template <> struct InfoTraits<CL_PROGRAM_BINARY_SIZES> {
  using type = std::vector<std::size_t>; /*!< Binary sizes per device. */

  static constexpr std::string_view name =
      "CL_PROGRAM_BINARY_SIZES"; /*!< Symbolic token. */

  /*!
   * \brief Convert binary size list to string.
   * \param v Vector of binary sizes.
   * \return Human-readable vector representation.
   */
  static std::string ToString(type const &v) noexcept {
    return VectorToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_PROGRAM_BINARIES>
 * \brief Traits for \c CL_PROGRAM_BINARIES.
 */
template <> struct InfoTraits<CL_PROGRAM_BINARIES> {
  using type =
      std::vector<std::vector<unsigned char>>; /*!< Program binaries. */

  static constexpr std::string_view name =
      "CL_PROGRAM_BINARIES"; /*!< Symbolic token. */

  /*!
   * \brief Convert binaries to readable string.
   * \return Placeholder string ("<binary blobs>").
   */
  static std::string ToString(type const &) noexcept {
    return "<binary blobs>";
  }
};

// =============================================================================
// === Kernel
// =============================================================================

/*!
 * \struct InfoTraits<CL_KERNEL_FUNCTION_NAME>
 * \brief Traits for \c CL_KERNEL_FUNCTION_NAME.
 */
template <> struct InfoTraits<CL_KERNEL_FUNCTION_NAME> {
  using type = std::string; /*!< Kernel function name. */

  static constexpr std::string_view name =
      "CL_KERNEL_FUNCTION_NAME"; /*!< Symbolic token. */

  /*!
   * \brief Return kernel function name.
   * \param v Kernel name.
   * \return The name itself.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_KERNEL_NUM_ARGS>
 * \brief Traits for \c CL_KERNEL_NUM_ARGS.
 */
template <> struct InfoTraits<CL_KERNEL_NUM_ARGS> {
  using type = cl_uint; /*!< Number of kernel arguments. */

  static constexpr std::string_view name =
      "CL_KERNEL_NUM_ARGS"; /*!< Symbolic token. */

  /*!
   * \brief Convert argument count to string.
   * \param v Number of arguments.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_KERNEL_REFERENCE_COUNT>
 * \brief Traits for \c CL_KERNEL_REFERENCE_COUNT.
 */
template <> struct InfoTraits<CL_KERNEL_REFERENCE_COUNT> {
  using type = cl_uint; /*!< Reference count. */

  static constexpr std::string_view name =
      "CL_KERNEL_REFERENCE_COUNT"; /*!< Symbolic token. */

  /*!
   * \brief Convert reference count to string.
   * \param v Reference count.
   * \return Decimal string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    return UIntToString(v);
  }
};

/*!
 * \struct InfoTraits<CL_KERNEL_ATTRIBUTES>
 * \brief Traits for \c CL_KERNEL_ATTRIBUTES.
 */
template <> struct InfoTraits<CL_KERNEL_ATTRIBUTES> {
  using type = std::string; /*!< Raw kernel attributes string. */

  static constexpr std::string_view name =
      "CL_KERNEL_ATTRIBUTES"; /*!< Symbolic token. */

  /*!
   * \brief Convert attribute string.
   * \param v Raw attribute string.
   * \return Same string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept { return v; }
};

/*!
 * \struct InfoTraits<CL_KERNEL_CONTEXT>
 * \brief Traits for \c CL_KERNEL_CONTEXT.
 */
template <> struct InfoTraits<CL_KERNEL_CONTEXT> {
  using type = cl::Context; /*!< Kernel context. */

  static constexpr std::string_view name =
      "CL_KERNEL_CONTEXT"; /*!< Symbolic token. */

  /*!
   * \brief Convert kernel context to string.
   * \param v Kernel context (unused).
   * \return Empty string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    (void)v;
    return "";
  }
};

/*!
 * \struct InfoTraits<CL_KERNEL_PROGRAM>
 * \brief Traits for \c CL_KERNEL_PROGRAM.
 */
template <> struct InfoTraits<CL_KERNEL_PROGRAM> {
  using type = cl::Program; /*!< Kernel program. */

  static constexpr std::string_view name =
      "CL_KERNEL_PROGRAM"; /*!< Symbolic token. */

  /*!
   * \brief Convert program object to string.
   * \param v Kernel program (unused).
   * \return Empty string.
   */
  [[nodiscard]] static std::string ToString(type v) noexcept {
    (void)v;
    return "";
  }
};
} // namespace ggems::ocl
