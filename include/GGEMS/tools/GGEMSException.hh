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
 * \brief Definition of GGEMSException class
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
 * \brief GGEMSException class handling exception from GGEMS library
 *
 * \note
 * This class is used as a singleton defined by:
 * using GGEMSLoggerManager = GGEMSSingletonHolder<GGEMSLogger>
 *
 * \code
 * Examples:
 * try {
 *   ...
 * }
 * catch (GGEMSException const& e) {
 *   gglog::err() << e.what() << gglog::endl;
 * }
 * \endcode
 */
class GGEMSException final : public std::exception {
public:
  /*!
   * \fn GGEMSException(std::string_view filename, std::string_view function_name, int const& line, std::string_view error_name)
   * \brief GGEMSException constructor by default deleted
   * \param filename - Name of file
   * \param function_name - Name of the function
   * \param line - Line error
   * \param error_name - Error description
   */
  GGEMSException(std::string_view filename, std::string_view function_name, int const& line, std::string_view error_name);

  /*!
   * \brief GGEMSException destructor
   * \fn ~GGEMSException(void) noexcept
   */
  virtual ~GGEMSException(void) noexcept = default;

public:
  /*!
   * \fn char const* what(void) const noexcept override
   * \brief Print error message to the terminal
   * \return The error message
   */
  char const* what(void) const noexcept override {
    return error_message_.c_str();
  }

private:
  /*!
   * \fn void BuildErrorMessage(std::string_view filename, std::string_view function_name, int const& line, std::string_view error_name)
   * \brief Build the error message before calling 'what'
   * \param filename - Name of file
   * \param function_name - Name of the function
   * \param line - Line error
   * \param error_name - Error description
  */
  void BuildErrorMessage(std::string_view filename, std::string_view function_name, int const& line, std::string_view error_name);

private:
  std::string error_message_; /*!< The final error message to print to the terminal */
}; // class GGEMSException
