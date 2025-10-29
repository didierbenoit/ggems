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
 * \brief Common utility functions and macros for GGEMS OpenCL error handling.
 *
 * This header defines the core diagnostic and exception-handling mechanisms
 * used across the GGEMS OpenCL subsystem. It provides:
 *  - Translation of OpenCL error codes into human-readable text.
 *  - A unified error reporting mechanism integrating GGEMSException.
 *  - Diagnostic macros for automatic inclusion of file, function, and line information.
 *
 * These utilities ensure consistent and detailed reporting of OpenCL failures,
 * which is critical for debugging heterogeneous computations involving multiple
 * platforms and devices.
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
 *
 * \brief Retrieves the current source file name stripped of its full path.
 *
 * This macro extracts only the base file name from the `__FILE__` macro.
 * It behaves differently based on the host operating system:
 * - On Windows, it searches for the last occurrence of the backslash (`\\`).
 * - On UNIX-like systems, it searches for the last occurrence of the forward slash (`/`).
 *
 * The result is a pointer suitable for logging or diagnostic output.
 */
#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/*!
 * \namespace ggocl
 * \brief Namespace grouping OpenCL utility functions for the GGEMS framework.
 *
 * Provides mechanisms for translating OpenCL error codes, reporting
 * failures, and throwing exceptions in a consistent and traceable manner.
 */
namespace ggocl {
  /*!
   * \brief Converts an OpenCL return value or error code into a human-readable string.
   * 
   * This overload allows direct use of OpenCL C++ API calls without manually
   * storing their return code. It accepts any type implicitly convertible
   * to `cl_int`, making it possible to write:
   *
   * \code
   * std::cerr << ggocl::GetErrorString(cl::Platform::get(&platforms)) << std::endl;
   * \endcode
   *
   * \tparam T Any type implicitly convertible to cl_int.
   * \param error_code The OpenCL error or return value.
   * \return A descriptive string representation of the error.
   */
  template <typename T>
  [[nodiscard]] std::string GetErrorString(T error_code) {
    return GetErrorString(static_cast<cl_int>(error_code));
  }

  /*!
   * \brief Custom terminate handler for unrecoverable GGEMS OpenCL failures.
   *
   * This handler is automatically installed during GGEMS initialisation
   * to provide a consistent and traceable shutdown when a fatal error occurs.
   * It logs the error using GGEMSLogger, flushes output streams, and terminates
   * the process immediately.
   *
   * \note This function does not throw and never returns.
   */
  [[noreturn]] void TerminateHandler() noexcept;

  /*!
   * \brief Converts an OpenCL error code into a human-readable string.
   *
   * \param error_code The numeric OpenCL error code to be translated.
   * \return A descriptive string representation of the error.
   *
   * This function is particularly useful when logging diagnostic messages
   * or creating exception reports, allowing the developer to interpret
   * OpenCL status codes without referring to external documentation.
   *
   * Example:
   * @code
   * GGOCL_CHECK(cl::Platform::get(&platforms));
   * @endcode
   */
  [[nodiscard]] std::string const GetErrorString(cl_int error_code);

  /*!
   * \brief Reports an OpenCL failure and throws a GGEMSException with full context.
   *
   * \param filename      The name of the source file where the failure occurred.
   * \param function_name The name of the function in which the error was raised.
   * \param line          The line number corresponding to the error.
   * \param error_code    The OpenCL error code to report.
   *
   * This function generates a detailed diagnostic message that includes
   * the file name, function name, line number, and translated OpenCL error
   * string. It then throws a GGEMSException with the assembled information.
   *
   * \throws GGEMSException Always throws to indicate a critical OpenCL failure.
   *
   * Example:
   * \code
   * GGOCL_CHECK(queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local))
   * \endcode
   */
  void Failure(std::string_view filename, std::string_view function_name, int line, cl_int error_code);

  /**
   * \def GGOCL_CHECK(error)
   * \brief Macro simplifying OpenCL error handling.
   *
   * This macro invokes `ggocl::Failure()` while automatically inserting
   * contextual metadata (file name, function signature, and line number).
   * It provides a concise syntax for checking and throwing OpenCL exceptions.
   *
   * \param error The OpenCL error code or result expression to evaluate.
   *
   * Example usage:
   * \code
   * GGOCL_CHECK(cl::Platform::get(&platforms));
   * \endcode
   */
  #define GGOCL_CHECK(error) (ggocl::Failure(__FILENAME__, __PRETTY_FUNCTION__, __LINE__, error));
}
