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
#include <source_location>
/// \endcond

namespace ggems::core {
  class GGEMSExceptionBase : public std::exception {
  public:
    explicit GGEMSExceptionBase(std::string message, std::string_view category,
      std::source_location loc = std::source_location::current()) noexcept;

    ~GGEMSExceptionBase() override = default;

    [[nodiscard]] const char* what() const noexcept override { return full_.c_str(); }

    [[nodiscard]] std::string_view Category() const noexcept { return category_; }
    [[nodiscard]] std::string_view File()     const noexcept { return file_; }
    [[nodiscard]] std::string_view Function() const noexcept { return function_; }
    [[nodiscard]] int              Line()     const noexcept { return line_; }
    [[nodiscard]] std::string_view Payload()  const noexcept { return payload_; }

  private:
    std::string payload_;   //!< Unformatted message payload
    std::string full_;      //!< Fully formatted message returned by what()
    std::string file_;      //!< Source file
    std::string function_;  //!< Source function
    int         line_{0};   //!< Source line
    std::string category_;  //!< Exception category label
  };

  class GGEMSRecoverable final : public GGEMSExceptionBase
  {
  public:
    explicit GGEMSRecoverable(std::string message,
      std::source_location loc = std::source_location::current()) noexcept
    : GGEMSExceptionBase(std::move(message), "Recoverable", loc) {}
  };

  class GGEMSInternal final : public GGEMSExceptionBase
  {
  public:
    explicit GGEMSInternal(std::string message,
      std::source_location loc = std::source_location::current()) noexcept
    : GGEMSExceptionBase(std::move(message), "Internal", loc) {}
  };

  template <typename T>
  concept GGEMSExceptionType = std::derived_from<T, GGEMSExceptionBase>;

  template <GGEMSExceptionType E>
  [[noreturn]] inline void Throw(std::string_view message,
      std::source_location loc = std::source_location::current()) {
    throw E(std::string(message), loc);
  }
} // namespace ggems::core
