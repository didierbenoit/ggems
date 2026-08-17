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
 * \brief Defines the GGEMS exception hierarchy and termination handler.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <concepts>
#include <cstdint>
#include <format>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
/// \endcond

/*!
 * \namespace ggems::core
 * \brief Provides the core runtime components of GGEMS.
 */
namespace ggems::core {
/*!
 * \brief Base class for structured GGEMS exceptions.
 *
 * Captures the exception category, message, and source location and exposes a
 * formatted diagnostic through what().
 */
class GGEMSExceptionBase : public std::runtime_error {
public:
  /*!
   * \brief Constructs a GGEMS exception with source diagnostics.
   *
   * \param[in] msg Message associated with the exception.
   * \param[in] cat Exception category.
   * \param[in] loc Source location where the exception is created.
   */
  explicit GGEMSExceptionBase(
      std::string msg, std::string cat,
      std::source_location loc = std::source_location::current())
      : std::runtime_error(msg), file_(loc.file_name()),
        function_(loc.function_name()), category_(std::move(cat)),
        line_(static_cast<std::int32_t>(loc.line())) {
    try {
      full_ = std::format("\n[GGEMS Exception]\n"
                          "  Type     : {}\n"
                          "  File     : {}\n"
                          "  Line     : {}\n"
                          "  Function : {}\n"
                          "  Message  : {}\n",
                          category_, file_, line_, function_, msg);
    } catch (...) {
      full_ = "\n[GGEMSException] (formatting failed): " + msg + "\n";
    }
  }

  /*!
   * \brief Returns the formatted GGEMS exception diagnostic.
   *
   * \return Null-terminated formatted diagnostic string.
   */
  [[nodiscard]] auto what() const noexcept -> char const * override {
    return full_.c_str();
  }

private:
  std::string full_; /*!< Formatted diagnostic returned by what(). */
  char const *file_; /*!< Source file where the exception was created. */
  char const *function_; /*!< Function where the exception was created. */
  std::string category_; /*!< Exception category. */
  std::int32_t line_{0}; /*!< Source line where the exception was created. */
};

/*!
 * \brief Represents a recoverable GGEMS exception.
 */
class GGEMSRecoverable : public GGEMSExceptionBase {
public:
  /*!
   * \brief Constructs a recoverable GGEMS exception.
   *
   * \param[in] msg Message associated with the exception.
   * \param[in] loc Source location where the exception is created.
   */
  explicit GGEMSRecoverable(
      std::string msg,
      std::source_location loc = std::source_location::current())
      : GGEMSExceptionBase(std::move(msg), "Recoverable", loc) {}
};

/*!
 * \brief Represents an internal GGEMS exception.
 */
class GGEMSInternal : public GGEMSExceptionBase {
public:
  /*!
   * \brief Constructs an internal GGEMS exception.
   *
   * \param[in] msg Message associated with the exception.
   * \param[in] loc Source location where the exception is created.
   */
  explicit GGEMSInternal(std::string msg, std::source_location loc =
                                              std::source_location::current())
      : GGEMSExceptionBase(std::move(msg), "Internal", loc) {}
};

/*!
 * \brief Represents a fatal GGEMS exception.
 */
class GGEMSFatal : public GGEMSExceptionBase {
public:
  /*!
   * \brief Constructs a fatal GGEMS exception.
   *
   * \param[in] msg Message associated with the exception.
   * \param[in] loc Source location where the exception is created.
   */
  explicit GGEMSFatal(std::string msg, std::source_location loc =
                                           std::source_location::current())
      : GGEMSExceptionBase(std::move(msg), "Fatal", loc) {}
};

/*!
 * \brief Restricts a type to the GGEMS exception hierarchy.
 *
 * \tparam T Type to validate.
 */
template <typename T>
concept GGEMSExceptionType = std::derived_from<T, GGEMSExceptionBase>;

/*!
 * \brief Reports the active exception and aborts the process.
 *
 * Writes an emergency diagnostic to standard error for the active GGEMS,
 * standard-library, or unknown exception. If no exception is active, reports
 * that condition before terminating.
 */
void TerminateHandler() noexcept;
} // namespace ggems::core
