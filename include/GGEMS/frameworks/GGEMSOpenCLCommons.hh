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
 * \file GGEMSOpenCLCommons.hh
 * \brief Common utility functions and macros for GGEMS OpenCL handling
 *
 * This header provides utility functions and macros for OpenCL error handling,
 * including translation of error codes into human-readable strings and a
 * standardised failure reporting mechanism.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <string_view>
#ifdef __APPLE__
#include <OpenCL/opencl.hpp>
#else
#include <CL/cl.hpp>
#endif
/// \endcond

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

/*!
 * \def __FILENAME__
 * \brief Retrieves the current source file name without the full path
 *
 * This macro extracts the base file name from the standard `__FILE__` macro.
 * - On Windows systems, it searches for the last occurrence of the backslash (`\\`).
 * - On UNIX-like systems, it searches for the last occurrence of the forward slash (`/`).
 *
 * The resulting pointer is suitable for logging or diagnostic output.
 */
#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/*!
 * \namespace ggocl
 * \brief Namespace containing utility functions for OpenCL in GGEMS
 *
 * Provides common functions for error handling, logging, and reporting
 * OpenCL failures in a standardised format.
 */
namespace ggocl {
  /*!
   * \brief Translate an OpenCL error code into a human-readable string
   * \param error_code - The OpenCL error code to translate
   * \return A string describing the error code
   *
   * This function can be used to log or report OpenCL errors in a
   * more understandable manner than numeric codes.
   */
  std::string const GetErrorString(cl_int error_code);

  /*!
   * \brief Report an OpenCL failure and throw an exception
   * \param filename - The source file where the failure occurred
   * \param function_name - The function where the failure occurred
   * \param line - The line number where the failure occurred
   * \param error_code - The OpenCL error code
   *
   * This function formats a detailed diagnostic message and throws
   * a GGEMS-specific exception to signal OpenCL failures.
   */
  void Failure(std::string_view filename, std::string_view function_name, int line, cl_int error_code);

  /*!
   * \def GGOCL_ERROR(error)
   * \brief Macro for concise OpenCL error reporting
   *
   * This macro invokes `ggocl::Failure` with standard diagnostic information:
   * the current file, function, line, and the OpenCL error code.
   *
   * Example usage:
   * \code
   * cl_int err = clSomeOpenCLFunction(...);
   * if (err != CL_SUCCESS) {
   *     GGOCL_ERROR(err);
   * }
   *
   * // Or inline:
   * GGOCL_ERROR(clSomeOpenCLFunction(...));
   * \endcode
   */
  #define GGOCL_ERROR(error) (ggocl::Failure(__FILENAME__, __PRETTY_FUNCTION__, __LINE__, error));
}
