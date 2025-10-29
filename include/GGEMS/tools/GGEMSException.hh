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
 * This class extends std::exception to deliver rich diagnostic information
 * in case of runtime or system-level errors. It captures the filename,
 * function name, line number, and a human-readable description of the
 * problem that caused the exception.
 *
 * The formatted message can be retrieved through the `what()` interface,
 * and is structured to be informative for both developers and automated
 * logging systems.*
 */
class GGEMSException final : public std::exception {
public:
  /*!
   * \brief Constructs a GGEMSException with full diagnostic information.
   * 
   * \param filename      Name of the source file where the exception occurred.
   * \param function_name Name of the function throwing the exception.
   * \param line          Line number where the exception was raised.
   * \param error_name    Human-readable description of the error.
   * 
   * The constructor automatically builds a formatted message containing all
   * contextual details. The resulting message is accessible via `what()`.
   */
  GGEMSException(std::string_view filename, std::string_view function_name, int line, std::string_view error_name);

  /*!
   * \brief Destructor
   *
   * Defaulted, noexcept destructor. Cleans up resources automatically.
   */
  ~GGEMSException() noexcept = default;

public:
  /*!
   * \brief Retrieves the formatted error message.
   * 
   * \return Pointer to a null-terminated string containing the diagnostic message.
   * 
   * The message includes the file name, function name, line number,
   * and a textual description of the error. The string remains valid
   * throughout the lifetime of the exception object.
   */
  [[nodiscard]] char const* what(void) const noexcept override {
    return error_message_.c_str();
  }

private:
  /*!
   * \brief Builds the detailed diagnostic error message.
   * 
   * \param filename      Name of the source file.
   * \param function_name Name of the function where the exception was thrown.
   * \param line          Line number associated with the error.
   * \param error_name    Human-readable description of the error.
   * 
   * This function concatenates all diagnostic fields into a single
   * formatted message stored in `error_message_`. The format is consistent
   * across all GGEMS components to facilitate structured logging.
   */
  void BuildErrorMessage(std::string_view filename, std::string_view function_name, int line, std::string_view error_name);

private:
  std::string error_message_; /*!< Fully formatted error message. */
};
