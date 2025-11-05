#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * Licensed under the GNU General Public License v3.0.                  *
// ************************************************************************

/*!
 * \file GGEMSException.hh
 * \brief Definition of GGEMSException for handling GGEMS-specific errors.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 3.0
 * \copyright GNU General Public License v3.0
 *
 * GGEMSException is a lightweight, final class derived from `std::exception`
 * providing full contextual diagnostics including filename, function, line,
 * and a human-readable error message. Designed for high-performance HPC code
 * where exceptions should remain informative yet inexpensive to build.
 */

/// \cond
#include <exception>
#include <string>
#include <string_view>
/// \endcond

/*!
 * \class GGEMSException
 * \brief Exception class for structured GGEMS error diagnostics.
 *
 * The class constructs a detailed diagnostic message at runtime including:
 * - file of origin,
 * - function name,
 * - line number,
 * - and an error description.
 *
 * This facilitates consistent logging across OpenCL, Vulkan, and other
 * GGEMS subsystems while maintaining minimal runtime overhead.
 */
class GGEMSException final : public std::exception {
public:
  /*!
   * \brief Constructs a GGEMSException with full diagnostic information.
   * \param filename      Name of the source file where the error occurred.
   * \param function_name Name of the function where the exception was raised.
   * \param line          Line number where the exception originated.
   * \param error_name    Human-readable description of the error.
   *
   * Automatically builds a fully formatted diagnostic string suitable
   * for output through `gglog::err()` or `std::cerr`.
   */
  [[nodiscard]]
  GGEMSException(std::string_view filename, std::string_view function_name, int line, std::string_view error_name);

  /*!
   * \brief Destructor (noexcept, defaulted).
   */
  ~GGEMSException() noexcept override = default;

public:
  /*!
   * \brief Retrieve the full error message as a C-string.
   * \return Null-terminated pointer to the formatted message.
   */
  [[nodiscard]] char const* what() const noexcept override {
    return error_message_.c_str();
  }

private:
  /*!
   * \brief Construct the detailed error message string.
   * \param filename      Name of the source file where the error occurred.
   * \param function_name Name of the function where the exception was raised.
   * \param line          Line number in the source file where the exception occurred.
   * \param error_name    Human-readable description of the error or failure cause.
   *
   * This function is called internally by the constructor to assemble a fully
   * formatted diagnostic message combining file, function, line, and error details.
   * The message is stored in the private member \c error_message_ and later returned
   * by the \c what() accessor.
   */
  void BuildErrorMessage(std::string_view filename, std::string_view function_name, int line, std::string_view error_name);

private:
  std::string error_message_; /*!< Fully formatted diagnostic message. */
};
