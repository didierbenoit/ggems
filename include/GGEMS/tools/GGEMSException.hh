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
 * \file GGEMSException.hh
 * \brief Definition of GGEMSException for handling GGEMS-specific errors
 *
 * This file defines the GGEMSException class, a final class derived
 * from `std::exception` that provides detailed error reporting including
 * source filename, function name, and line number.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <exception>
#include <string>
#include <string_view>
/// \endcond

/*!
 * \class GGEMSException
 * \brief Exception class for GGEMS library errors
 *
 * GGEMSException provides detailed diagnostic information for exceptions
 * raised within the GGEMS framework. The error message includes:
 * - Source file name
 * - Function name
 * - Line number
 * - Custom error description
 *
 * \note
 * This class is final and cannot be inherited.
 *
 * \code
 * // Example usage:
 * try {
 *   // Some GGEMS operation
 * } 
 * catch (GGEMSException const& e) {
 *   gglog::err() << e.what() << gglog::endl;
 * }
 * \endcode
 */
class GGEMSException final : public std::exception {
public:
  /*!
   * \brief Construct a GGEMSException with full diagnostic information
   * \param filename - Name of the source file where the exception occurred
   * \param function_name - Name of the function throwing the exception
   * \param line - Line number where the exception was raised
   * \param error_name - Description of the error
   *
   * The constructor internally builds a formatted error message that
   * can be retrieved using `what()`.
   */
  GGEMSException(std::string_view filename, std::string_view function_name, int const& line, std::string_view error_name);

  /*!
   * \brief Destructor
   *
   * Defaulted, noexcept destructor. The exception object cleans up
   * automatically when it goes out of scope.
   */
  virtual ~GGEMSException(void) noexcept = default;

public:
  /*!
   * \brief Retrieve the error message
   * \return Pointer to a null-terminated string containing the formatted error message
   *
   * The message includes filename, function name, line number, and error description.
   */
  char const* what(void) const noexcept override {
    return error_message_.c_str();
  }

private:
  /*!
   * \brief Construct the detailed error message
   * \param filename - Source file name
   * \param function_name - Function name
   * \param line - Line number
   * \param error_name - Description of the error
   *
   * This function is called by the constructor to populate the `error_message_`
   * string with a fully formatted diagnostic message.
   */
  void BuildErrorMessage(std::string_view filename, std::string_view function_name, int const& line, std::string_view error_name);

private:
  std::string error_message_; /*!< Formatted error message to be returned by `what()` */
};
