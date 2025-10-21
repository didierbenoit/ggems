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
 * \brief Definition of GGEMSOpenCLCommons useful methods
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
 * \brief Returns the current source file name without its full path.
 *
 * This macro derives the file name from the standard `__FILE__` macro.
 * - On Windows systems, it searches for the last occurrence of the backslash (`\\`).
 * - On UNIX-like systems, it searches for the last forward slash (`/`).
 *
 * The result is a pointer to the base file name, suitable for logging or diagnostic output.
 */
#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/*!
 * \namespace ggocl
 * \brief Namespace storing GGEMS OpenCL useful methods
 */
namespace ggocl {
  /*!
   * \fn std::string const GetErrorString(cl_int const& error_code)
   * \param error_code - Error code for OpenCL
   * \brief Return the error name
   * \return Error message in string format
   */
  std::string const GetErrorString(cl_int const& error_code);

  /*!
   * \fn void Failure(std::string_view filename, std::string_view function_name, int const& line, cl_int const& error_code)
    \param filename - Filename where the exception is thrown
    \param function_name - Function name where the exception is thrown
    \param line - Fine in the function where the exception is thrown
    \param error_code - OpenCL error code
    \brief throw an GGEMS exception
  */
  void Failure(std::string_view filename, std::string_view function_name, int const& line, cl_int const& error_code);

  /*!
   * \def GGOCL_ERROR(error)
   * \brief Constructs an OpenCL failure object with diagnostic information.
   *
   * It is intended for concise reporting of OpenCL errors in a standardised format.
   *
   * Example usage:
   * \code
   * cl_int err = OpenCLAPIFunction(...);
   * if (err != CL_SUCCESS) {
   *     GGOCL_ERROR(err);
   * }
   * GGOCL_ERROR(OpenCLAPIFunction(...))
   * \endcode
   */
  #define GGOCL_ERROR(error) (ggocl::Failure(__FILENAME__, __PRETTY_FUNCTION__, __LINE__, error));
} // ggocl namespace
