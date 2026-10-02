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
 * \brief Defines internal OpenCL information traits used by generic query
 * helpers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <string>
#include <cstddef>
#include <string_view>
#include <vector>
#include <array>

#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLStrings.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSBitsUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace ggems::ocl {
using namespace ggems::units;

/*!
 * \brief Maps an OpenCL information selector to its value type and formatter.
 *
 * \inheritancegraph{TEXT}
 *
 * \tparam Info OpenCL information selector with a supported specialization.
 */
template <cl_uint Info> struct InfoTraits;

/*! \brief Defines the query representation for CL_PLATFORM_VENDOR. */
template <> struct InfoTraits<CL_PLATFORM_VENDOR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PLATFORM_VENDOR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_PLATFORM_NAME. */
template <> struct InfoTraits<CL_PLATFORM_NAME> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PLATFORM_NAME";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_PLATFORM_VERSION. */
template <> struct InfoTraits<CL_PLATFORM_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PLATFORM_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_PLATFORM_PROFILE. */
template <> struct InfoTraits<CL_PLATFORM_PROFILE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PLATFORM_PROFILE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_PLATFORM_EXTENSIONS. */
template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PLATFORM_EXTENSIONS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_PLATFORM_NUMERIC_VERSION. */
template <> struct InfoTraits<CL_PLATFORM_NUMERIC_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_version;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PLATFORM_NUMERIC_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClVersionToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_PLATFORM_HOST_TIMER_RESOLUTION.
 */
template <> struct InfoTraits<CL_PLATFORM_HOST_TIMER_RESOLUTION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_ulong;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PLATFORM_HOST_TIMER_RESOLUTION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    if (value == 0) {
      return "not supported";
    }
    auto const duration = MakeQuantity<Duration>(value, "ns");
    if (!duration.has_value()) {
      return "out of range";
    }
    return HumanReadable(*duration, 0, 3);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_PLATFORM_EXTENSIONS_WITH_VERSION.
 */
template <> struct InfoTraits<CL_PLATFORM_EXTENSIONS_WITH_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_name_version>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_PLATFORM_EXTENSIONS_WITH_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return ClNameVersionToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_EXTENSIONS. */
template <> struct InfoTraits<CL_DEVICE_EXTENSIONS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_EXTENSIONS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_IL_VERSION. */
template <> struct InfoTraits<CL_DEVICE_IL_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IL_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_SPIR_VERSIONS. */
template <> struct InfoTraits<CL_DEVICE_SPIR_VERSIONS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_SPIR_VERSIONS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_NAME. */
template <> struct InfoTraits<CL_DEVICE_NAME> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_NAME";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_VENDOR. */
template <> struct InfoTraits<CL_DEVICE_VENDOR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_VENDOR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_VERSION. */
template <> struct InfoTraits<CL_DEVICE_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DRIVER_VERSION. */
template <> struct InfoTraits<CL_DRIVER_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DRIVER_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_PROFILE. */
template <> struct InfoTraits<CL_DEVICE_PROFILE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_PROFILE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_OPENCL_C_VERSION. */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_ILS_WITH_VERSION. */
template <> struct InfoTraits<CL_DEVICE_ILS_WITH_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_name_version>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_ILS_WITH_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return ClNameVersionToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_OPENCL_C_ALL_VERSIONS.
 */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_ALL_VERSIONS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_name_version>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_ALL_VERSIONS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return ClNameVersionToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR.
 */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_version_khr;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClVersionToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_OPENCL_C_FEATURES. */
template <> struct InfoTraits<CL_DEVICE_OPENCL_C_FEATURES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_name_version>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_OPENCL_C_FEATURES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return ClNameVersionToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT.
 */
template <> struct InfoTraits<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_version;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClVersionToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_NUMERIC_VERSION. */
template <> struct InfoTraits<CL_DEVICE_NUMERIC_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_version;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_NUMERIC_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClVersionToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_VENDOR_ID. */
template <> struct InfoTraits<CL_DEVICE_VENDOR_ID> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_VENDOR_ID";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return VendorIdToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_TYPE. */
template <> struct InfoTraits<CL_DEVICE_TYPE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_type;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_TYPE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return DeviceTypeToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_MAX_COMPUTE_UNITS. */
template <> struct InfoTraits<CL_DEVICE_MAX_COMPUTE_UNITS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_COMPUTE_UNITS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MAX_CLOCK_FREQUENCY.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_CLOCK_FREQUENCY> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_CLOCK_FREQUENCY";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    if (value != 0) {
      auto const frequency = MakeQuantity<Frequency>(value, "MHz");
      if (!frequency.has_value()) {
        return "N/A";
      }
      return HumanReadable(*frequency, 1, 5);
    }

    return "N/A";
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MAX_WORK_GROUP_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_WORK_GROUP_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_GROUP_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MAX_WORK_ITEM_SIZES.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_WORK_ITEM_SIZES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<std::size_t>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_WORK_ITEM_SIZES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return SizeToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NATIVE_VECTOR_WIDTH_INT.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF.
 */
template <> struct InfoTraits<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_IMAGE2D_MAX_WIDTH. */
template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_WIDTH> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE2D_MAX_WIDTH";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_IMAGE2D_MAX_HEIGHT. */
template <> struct InfoTraits<CL_DEVICE_IMAGE2D_MAX_HEIGHT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE2D_MAX_HEIGHT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_IMAGE3D_MAX_WIDTH. */
template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_WIDTH> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_WIDTH";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_IMAGE3D_MAX_HEIGHT. */
template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_HEIGHT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_HEIGHT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_IMAGE3D_MAX_DEPTH. */
template <> struct InfoTraits<CL_DEVICE_IMAGE3D_MAX_DEPTH> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE3D_MAX_DEPTH";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_IMAGE_MAX_BUFFER_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_BUFFER_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_IMAGE_MAX_ARRAY_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_MAX_ARRAY_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_IMAGE_SUPPORT. */
template <> struct InfoTraits<CL_DEVICE_IMAGE_SUPPORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_SUPPORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MAX_READ_IMAGE_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_READ_IMAGE_ARGS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_READ_IMAGE_ARGS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MAX_WRITE_IMAGE_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_WRITE_IMAGE_ARGS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_WRITE_IMAGE_ARGS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_IMAGE_PITCH_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_PITCH_ALIGNMENT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_IMAGE_PITCH_ALIGNMENT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_MAX_SAMPLERS. */
template <> struct InfoTraits<CL_DEVICE_MAX_SAMPLERS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_SAMPLERS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_GLOBAL_MEM_SIZE. */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_ulong;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_SIZE";

  /*! \brief Unit suffix retained by this information specialization. */
  static constexpr std::string_view unit = " bytes";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_GLOBAL_MEM_CACHE_TYPE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_mem_cache_type;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return CacheTypeToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_GLOBAL_MEM_CACHE_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_ulong;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_LOCAL_MEM_SIZE. */
template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_ulong;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_LOCAL_MEM_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_LOCAL_MEM_TYPE. */
template <> struct InfoTraits<CL_DEVICE_LOCAL_MEM_TYPE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_local_mem_type;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_LOCAL_MEM_TYPE";

  /*! \brief Unit suffix retained by this information specialization. */
  static constexpr std::string_view unit{};

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return LocalMemTypeToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_MAX_MEM_ALLOC_SIZE. */
template <> struct InfoTraits<CL_DEVICE_MAX_MEM_ALLOC_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_ulong;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_MEM_ALLOC_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_ulong;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_MAX_CONSTANT_ARGS. */
template <> struct InfoTraits<CL_DEVICE_MAX_CONSTANT_ARGS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_CONSTANT_ARGS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MEM_BASE_ADDR_ALIGN.
 */
template <> struct InfoTraits<CL_DEVICE_MEM_BASE_ADDR_ALIGN> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MEM_BASE_ADDR_ALIGN";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    Bits bits{static_cast<std::uint64_t>(value)};
    return HumanReadable(bits, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_HOST_UNIFIED_MEMORY.
 */
template <> struct InfoTraits<CL_DEVICE_HOST_UNIFIED_MEMORY> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_HOST_UNIFIED_MEMORY";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_QUEUE_ON_HOST_PROPERTIES.
 */
template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_command_queue_properties;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_QUEUE_ON_HOST_PROPERTIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return QueuePropertiesToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES.
 */
template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_command_queue_properties;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return QueuePropertiesToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE";

  /*! \brief Unit suffix retained by this information specialization. */
  static constexpr std::string_view unit = " bytes";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MAX_ON_DEVICE_QUEUES.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_QUEUES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_ON_DEVICE_QUEUES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_MAX_ON_DEVICE_EVENTS.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_ON_DEVICE_EVENTS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_ON_DEVICE_EVENTS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_SVM_CAPABILITIES. */
template <> struct InfoTraits<CL_DEVICE_SVM_CAPABILITIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_svm_capabilities;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_SVM_CAPABILITIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return SVMCapabilitiesToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_atomic_capabilities;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return AtomicCapabilitiesToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_ATOMIC_FENCE_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_atomic_capabilities;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return AtomicCapabilitiesToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_MAX_NUM_SUB_GROUPS. */
template <> struct InfoTraits<CL_DEVICE_MAX_NUM_SUB_GROUPS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_NUM_SUB_GROUPS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS.
 */
template <>
struct InfoTraits<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_EXECUTION_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_EXECUTION_CAPABILITIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_exec_capabilities;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_EXECUTION_CAPABILITIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ExecCapabilitiesToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT.
 */
template <>
struct InfoTraits<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES.
 */
template <> struct InfoTraits<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_device_enqueue_capabilities;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return DeviceEnqueueCapabilitiesToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PARTITION_MAX_SUB_DEVICES.
 */
template <> struct InfoTraits<CL_DEVICE_PARTITION_MAX_SUB_DEVICES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PARTITION_MAX_SUB_DEVICES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_PARTITION_PROPERTIES.
 */
template <> struct InfoTraits<CL_DEVICE_PARTITION_PROPERTIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_device_partition_property>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_PARTITION_PROPERTIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return PartitionPropertiesToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PARTITION_AFFINITY_DOMAIN.
 */
template <> struct InfoTraits<CL_DEVICE_PARTITION_AFFINITY_DOMAIN> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_affinity_domain;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PARTITION_AFFINITY_DOMAIN";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return AffinityDomainToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_PARTITION_TYPE. */
template <> struct InfoTraits<CL_DEVICE_PARTITION_TYPE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_device_partition_property>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_PARTITION_TYPE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return PartitionPropertiesToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_MAX_PARAMETER_SIZE. */
template <> struct InfoTraits<CL_DEVICE_MAX_PARAMETER_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_PARAMETER_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return std::to_string(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_EXTENSIONS_WITH_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_EXTENSIONS_WITH_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_name_version>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_EXTENSIONS_WITH_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return ClNameVersionToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED.
 */
template <> struct InfoTraits<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_DEVICE_BUILT_IN_KERNELS. */
template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_BUILT_IN_KERNELS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION.
 */
template <> struct InfoTraits<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_name_version>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return ClNameVersionToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT";

  /*! \brief Unit suffix retained by this information specialization. */
  static constexpr std::string_view unit = " bytes";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_ADDRESS_BITS. */
template <> struct InfoTraits<CL_DEVICE_ADDRESS_BITS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_ADDRESS_BITS";

  /*! \brief Unit suffix retained by this information specialization. */
  static constexpr std::string_view unit = " bits";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    Bits bits = Bits{static_cast<std::uint64_t>(value)};
    return HumanReadable(bits, 0, 3);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PROFILING_TIMER_RESOLUTION.
 */
template <> struct InfoTraits<CL_DEVICE_PROFILING_TIMER_RESOLUTION> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PROFILING_TIMER_RESOLUTION";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto const duration = MakeQuantity<Duration>(value, "ns");
    if (!duration.has_value()) {
      return "out of range";
    }
    return HumanReadable(*duration, 0, 3);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_COMPILER_AVAILABLE. */
template <> struct InfoTraits<CL_DEVICE_COMPILER_AVAILABLE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_COMPILER_AVAILABLE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_LINKER_AVAILABLE. */
template <> struct InfoTraits<CL_DEVICE_LINKER_AVAILABLE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_LINKER_AVAILABLE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_AVAILABLE. */
template <> struct InfoTraits<CL_DEVICE_AVAILABLE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_AVAILABLE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_ENDIAN_LITTLE. */
template <> struct InfoTraits<CL_DEVICE_ENDIAN_LITTLE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_ENDIAN_LITTLE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_ERROR_CORRECTION_SUPPORT.
 */
template <> struct InfoTraits<CL_DEVICE_ERROR_CORRECTION_SUPPORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_ERROR_CORRECTION_SUPPORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_PRINTF_BUFFER_SIZE. */
template <> struct InfoTraits<CL_DEVICE_PRINTF_BUFFER_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_PRINTF_BUFFER_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_MAX_PIPE_ARGS. */
template <> struct InfoTraits<CL_DEVICE_MAX_PIPE_ARGS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_PIPE_ARGS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS.
 */
template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for CL_DEVICE_PIPE_MAX_PACKET_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_PIPE_MAX_PACKET_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_PIPE_MAX_PACKET_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_PIPE_SUPPORT. */
template <> struct InfoTraits<CL_DEVICE_PIPE_SUPPORT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_PIPE_SUPPORT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE.
 */
template <> struct InfoTraits<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::size_t;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    auto bytes = Bytes{static_cast<std::uint64_t>(value)};
    return HumanReadable(bytes, 1, 5);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_UUID_KHR. */
template <> struct InfoTraits<CL_DEVICE_UUID_KHR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::array<cl_uchar, CL_UUID_SIZE_KHR>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_UUID_KHR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return UUIDToString(value);
  }
};

/*! \brief Defines the query representation for CL_DRIVER_UUID_KHR. */
template <> struct InfoTraits<CL_DRIVER_UUID_KHR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::array<cl_uchar, CL_UUID_SIZE_KHR>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DRIVER_UUID_KHR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return UUIDToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_LUID_VALID_KHR. */
template <> struct InfoTraits<CL_DEVICE_LUID_VALID_KHR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_LUID_VALID_KHR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_LUID_KHR. */
template <> struct InfoTraits<CL_DEVICE_LUID_KHR> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::array<cl_uchar, CL_LUID_SIZE_KHR>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_LUID_KHR";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return LUIDToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_HALF_FP_CONFIG. */
template <> struct InfoTraits<CL_DEVICE_HALF_FP_CONFIG> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_fp_config;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_HALF_FP_CONFIG";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return FPConfigToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_SINGLE_FP_CONFIG. */
template <> struct InfoTraits<CL_DEVICE_SINGLE_FP_CONFIG> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_fp_config;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_SINGLE_FP_CONFIG";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return FPConfigToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_DOUBLE_FP_CONFIG. */
template <> struct InfoTraits<CL_DEVICE_DOUBLE_FP_CONFIG> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_device_fp_config;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_DOUBLE_FP_CONFIG";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return FPConfigToString(value);
  }
};

/*! \brief Defines the query representation for CL_DEVICE_REFERENCE_COUNT. */
template <> struct InfoTraits<CL_DEVICE_REFERENCE_COUNT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_DEVICE_REFERENCE_COUNT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*!
 * \brief Defines the query representation for
 *   CL_DEVICE_PREFERRED_INTEROP_USER_SYNC.
 */
template <> struct InfoTraits<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_bool;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name =
    "CL_DEVICE_PREFERRED_INTEROP_USER_SYNC";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return ClBoolToString(value);
  }
};

/*! \brief Defines the query representation for CL_CONTEXT_REFERENCE_COUNT. */
template <> struct InfoTraits<CL_CONTEXT_REFERENCE_COUNT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_CONTEXT_REFERENCE_COUNT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_CONTEXT_NUM_DEVICES. */
template <> struct InfoTraits<CL_CONTEXT_NUM_DEVICES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_CONTEXT_NUM_DEVICES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_CONTEXT_DEVICES. */
template <> struct InfoTraits<CL_CONTEXT_DEVICES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl::Device>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_CONTEXT_DEVICES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return DevicesToString(value);
  }
};

/*! \brief Defines the query representation for CL_CONTEXT_PROPERTIES. */
template <> struct InfoTraits<CL_CONTEXT_PROPERTIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_context_properties>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_CONTEXT_PROPERTIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return ContextPropertiesToString(value);
  }
};

/*! \brief Defines the query representation for CL_QUEUE_CONTEXT. */
template <> struct InfoTraits<CL_QUEUE_CONTEXT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl::Context;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_QUEUE_CONTEXT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    (void)value;
    return "";
  }
};

/*! \brief Defines the query representation for CL_QUEUE_DEVICE. */
template <> struct InfoTraits<CL_QUEUE_DEVICE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl::Device;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_QUEUE_DEVICE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return DeviceToString(value);
  }
};

/*! \brief Defines the query representation for CL_QUEUE_REFERENCE_COUNT. */
template <> struct InfoTraits<CL_QUEUE_REFERENCE_COUNT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_QUEUE_REFERENCE_COUNT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_QUEUE_SIZE. */
template <> struct InfoTraits<CL_QUEUE_SIZE> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_QUEUE_SIZE";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_QUEUE_PROPERTIES. */
template <> struct InfoTraits<CL_QUEUE_PROPERTIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_command_queue_properties;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_QUEUE_PROPERTIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return QueuePropertiesToString(value);
  }
};

/*! \brief Defines the query representation for CL_QUEUE_PROPERTIES_ARRAY. */
template <> struct InfoTraits<CL_QUEUE_PROPERTIES_ARRAY> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<cl_queue_properties>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_QUEUE_PROPERTIES_ARRAY";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    return QueuePropertiesArrayToString(value);
  }
};

// =============================================================================
// === Program
// =============================================================================

/*! \brief Defines the query representation for CL_PROGRAM_NUM_DEVICES. */
template <> struct InfoTraits<CL_PROGRAM_NUM_DEVICES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PROGRAM_NUM_DEVICES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_PROGRAM_BINARY_SIZES. */
template <> struct InfoTraits<CL_PROGRAM_BINARY_SIZES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<std::size_t>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PROGRAM_BINARY_SIZES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  static auto ToString(type const &value) -> std::string {
    return VectorToString(value);
  }
};

/*! \brief Defines the query representation for CL_PROGRAM_BINARIES. */
template <> struct InfoTraits<CL_PROGRAM_BINARIES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::vector<std::vector<unsigned char>>;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_PROGRAM_BINARIES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  static auto ToString([[maybe_unused]] type const &value) -> std::string {
    return "<binary blobs>";
  }
};

/*! \brief Defines the query representation for CL_KERNEL_FUNCTION_NAME. */
template <> struct InfoTraits<CL_KERNEL_FUNCTION_NAME> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_KERNEL_FUNCTION_NAME";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_KERNEL_NUM_ARGS. */
template <> struct InfoTraits<CL_KERNEL_NUM_ARGS> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_KERNEL_NUM_ARGS";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_KERNEL_REFERENCE_COUNT. */
template <> struct InfoTraits<CL_KERNEL_REFERENCE_COUNT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl_uint;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_KERNEL_REFERENCE_COUNT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) -> std::string {
    return UIntToString(value);
  }
};

/*! \brief Defines the query representation for CL_KERNEL_ATTRIBUTES. */
template <> struct InfoTraits<CL_KERNEL_ATTRIBUTES> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = std::string;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_KERNEL_ATTRIBUTES";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type value) noexcept -> std::string {
    return value;
  }
};

/*! \brief Defines the query representation for CL_KERNEL_CONTEXT. */
template <> struct InfoTraits<CL_KERNEL_CONTEXT> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl::Context;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_KERNEL_CONTEXT";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    (void)value;
    return "";
  }
};

/*! \brief Defines the query representation for CL_KERNEL_PROGRAM. */
template <> struct InfoTraits<CL_KERNEL_PROGRAM> {
  /*! \brief Value type returned by the OpenCL information query. */
  using type = cl::Program;

  /*! \brief OpenCL selector name used in diagnostics. */
  static constexpr std::string_view name = "CL_KERNEL_PROGRAM";

  /*!
   * \brief Formats the query result for diagnostic output.
   *
   * \param[in] value Value returned for this information selector.
   * \return Human-readable query value or a selector-specific status label.
   */
  [[nodiscard]] static auto ToString(type const &value) -> std::string {
    (void)value;
    return "";
  }
};
} // namespace ggems::ocl
