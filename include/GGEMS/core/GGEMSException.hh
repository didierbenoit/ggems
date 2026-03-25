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
 * \brief Base exception class and specialised GGEMS exception categories.
 *
 * This header provides the exception hierarchy used throughout GGEMS.
 * All exceptions ultimately derive from \c GGEMSExceptionBase, which
 * formats an annotated diagnostic message, optionally logs it, and
 * exposes the source location from which it originated.
 *
 * Each exception carries information about:
 *  - the file name where it was thrown,
 *  - the function name,
 *  - the source code line,
 *  - the exception category,
 *  - the message supplied by the caller.
 *
 * The formatting uses \c std::format to build a multi-line summary,
 * suitable for both console display and logs. Derived exceptions simply
 * specialise their category (e.g. "Recoverable", "Internal", "Fatal").
 *
 * A generic helper \c Throw<E>() is also provided to throw any GGEMS
 * exception type with an optional source location. By default, the
 * current caller location is captured via \c std::source_location.
 *
 * \note
 * Logging is performed via the protected static method \c Log(), which
 * is noexcept and implemented in the corresponding source file. Logging
 * can be disabled on a per-exception basis.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::core {
/*!
 * \class GGEMSExceptionBase
 * \brief Base class for all GGEMS exceptions.
 *
 * This class enriches \c std::runtime_error with annotated metadata,
 * including file name, function name, line number and category. A full,
 * pre-formatted diagnostic string is assembled at construction and
 * returned by \c what().
 *
 * If logging is enabled, the formatted block is passed to \c Log(). Any
 * failure during formatting or logging never throws and is silently
 * handled.
 */
class GGEMSExceptionBase : public std::runtime_error {
public:
  /*!
   * \brief Constructs a GGEMS exception with annotated context.
   *
   * \param msg  The descriptive error message supplied by the caller.
   * \param cat  Category label used for classification (e.g. "Fatal").
   * \param loc  Source location automatically captured by default.
   * \param do_log Whether the exception should be written through
   *               the \c Log() mechanism. If \c false, the exception
   *               is fully constructed but not logged.
   */
  explicit GGEMSExceptionBase(
      std::string msg, std::string cat,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
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

    logged_ = do_log;
  }

  /*!
   * \brief Returns the fully formatted diagnostic message.
   *
   * The string includes category, file, function, line number and
   * the original message.
   *
   * \return C-string pointer to the formatted content.
   */
  [[nodiscard]] char const *what() const noexcept override {
    return full_.c_str();
  }

  /*!
   * \brief Returns the file where the exception originated.
   * \return UTF-8 encoded filename stored at construction.
   */
  [[nodiscard]] char const *GetFileName() const noexcept { return file_; }

  /*!
   * \brief Returns the name of the enclosing function.
   * \return UTF-8 encoded function identifier.
   */
  [[nodiscard]] char const *GetFunctionName() const noexcept {
    return function_;
  }

  /*!
   * \brief Returns the category selected for this exception.
   * \return Category string such as "Recoverable", "Internal" or "Fatal".
   */
  [[nodiscard]] char const *GetCategory() const noexcept {
    return category_.c_str();
  }

  /*!
   * \brief Returns the source line where the exception was created.
   * \return Source code line number.
   */
  [[nodiscard]] std::int32_t GetLine() const noexcept { return line_; }

  /*!
   * \brief Reports whether the exception diagnostic was logged.
   * \return \c true if logging succeeded, \c false otherwise.
   */
  [[nodiscard]] bool Logged() const noexcept { return logged_; }

protected:
  /*!
   * \brief Writes the formatted exception block to the logging backend.
   *
   * Any failure during logging is ignored silently. This function is
   * implemented in the \c .cc file to keep dependencies minimal.
   *
   * \param msg Full multi-line diagnostic message.
   */
  static void Log(std::string const &msg) noexcept;

private:
  friend void TerminateHandler() noexcept;

private:
  std::string full_; /*!< Full multi-line formatted diagnostic message. */
  char const *file_; /*!< Pointer to the UTF-8 filename stored internally. */
  char const *function_; /*!< Pointer to the UTF-8 function name extracted from
                            source location. */
  std::string category_; /*!< Exception category string. */
  std::int32_t line_{
      0}; /*!< Source line number extracted from source location. */
  bool logged_{
      false}; /*!< Indicates whether logging was successfully performed. */
};

/*!
 * \class GGEMSRecoverable
 * \brief Exception indicating that execution can continue.
 *
 * This category is used when an operation fails but the overall state
 * of the simulation remains consistent. Typical usage includes I/O
 * failures, invalid runtime parameters or recoverable resource issues.
 */
class GGEMSRecoverable : public GGEMSExceptionBase {
public:
  /*!
   * \brief Constructs a recoverable exception.
   * \param msg Descriptive message.
   * \param loc Optional source location (captured by default).
   * \param do_log Whether to emit the exception to the log backend.
   */
  explicit GGEMSRecoverable(
      std::string msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : GGEMSExceptionBase(std::move(msg), "Recoverable", loc, do_log) {}
};

/*!
 * \class GGEMSInternal
 * \brief Signals an unexpected internal failure.
 *
 * This category denotes programming errors, invariant violations
 * or any inconsistency indicating internal malfunction.
 */
class GGEMSInternal : public GGEMSExceptionBase {
public:
  /*!
   * \brief Constructs an internal exception.
   * \param msg Descriptive message.
   * \param loc Optional source location (captured by default).
   * \param do_log Whether to emit the exception to the log backend.
   */
  explicit GGEMSInternal(
      std::string msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : GGEMSExceptionBase(std::move(msg), "Internal", loc, do_log) {}
};

/*!
 * \class GGEMSFatal
 * \brief Represents a non-recoverable, critical error.
 *
 * After a fatal exception, the system state must be considered
 * undefined; safe recovery cannot be guaranteed.
 */
class GGEMSFatal : public GGEMSExceptionBase {
public:
  /*!
   * \brief Constructs a fatal exception.
   * \param msg Descriptive message.
   * \param loc Optional source location (captured by default).
   * \param do_log Whether to emit the exception to the log backend.
   */
  explicit GGEMSFatal(
      std::string msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : GGEMSExceptionBase(std::move(msg), "Fatal", loc, do_log) {}
};

/*!
 * \brief Concept restricting a type to GGEMS exception classes.
 *
 * Any type conforming to this concept is guaranteed to derive from
 * \c GGEMSExceptionBase.
 */
template <typename T>
concept GGEMSExceptionType = std::derived_from<T, GGEMSExceptionBase>;

/*!
 * \brief Throws a GGEMS exception with contextual metadata.
 *
 * This helper captures the current source location by default and
 * forwards the supplied message to the selected exception type.
 *
 * \tparam E Exception type satisfying \c GGEMSExceptionType.
 *
 * \param msg Descriptive message.
 * \param loc Optional source location (auto-captured by default).
 * \param do_log Whether the exception should be logged immediately.
 *
 * \throws E Always throws the constructed exception.
 */
template <GGEMSExceptionType E>
[[noreturn]] inline void
Throw(std::string msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true) {
  throw E(std::string(msg), loc, do_log);
}

/*!
 * \brief Custom handler installed as \c std::terminate hook.
 *
 * This function is invoked when an unhandled exception results in
 * program termination. It is granted access to internals of the
 * base exception in order to extract diagnostic details.
 */
void TerminateHandler() noexcept;
} // namespace ggems::core
