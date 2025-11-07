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
#include <stdexcept>
#include <string>
#include <format>
#include <string_view>
#include <source_location>
/// \endcond

#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::core {
  class GGEMSExceptionBase : public std::runtime_error {
  public:
    explicit GGEMSExceptionBase(std::string const& msg,
                                std::string const& cat,
                                std::source_location loc = std::source_location::current(),
                                bool do_log = true)
      : std::runtime_error(msg),
        file_(loc.file_name()),
        function_(loc.function_name()),
        category_(cat),
        line_(static_cast<int>(loc.line())) {
    full_ = std::vformat("[{}] {}:{} ({}): {}", 
                         std::make_format_args(category_, file_, line_, function_, msg));
    if (do_log) {
      GGEMSLogger::GetInstance().Error("Exception", full_);
      logged_ = true;
    }
  }

    bool Logged() const noexcept { return logged_; }
    [[nodiscard]] std::string const& FullMessage() const noexcept { return full_; }

  protected:
    std::string full_;
    std::string file_;
    std::string function_;
    std::string category_;
    int         line_;
    bool        logged_{false};
  };

  class GGEMSRecoverable final : public GGEMSExceptionBase
  {
  public:
    explicit GGEMSRecoverable(std::string const& msg,
                              std::source_location loc = std::source_location::current(),
                              bool do_log = true)
    : GGEMSExceptionBase(std::move(msg), "Recoverable", loc, do_log) {}
  };

  class GGEMSInternal final : public GGEMSExceptionBase
  {
  public:
    explicit GGEMSInternal(std::string const& msg,
                           std::source_location loc = std::source_location::current(),
                           bool do_log = true)
    : GGEMSExceptionBase(std::move(msg), "Internal", loc, do_log) {}
  };

  class GGEMSFatal final : public GGEMSExceptionBase
  {
  public:
    explicit GGEMSFatal(std::string const& msg,
                        std::source_location loc = std::source_location::current(),
                        bool do_log = true)
    : GGEMSExceptionBase(std::move(msg), "Fatal", loc, do_log) {}
  };

  template <typename T>
  concept GGEMSExceptionType = std::derived_from<T, GGEMSExceptionBase>;

  template <GGEMSExceptionType E>
  [[noreturn]] inline void Throw(std::string_view msg,
                                 std::source_location loc = std::source_location::current(),
                                 bool do_log = true) {
    throw E(std::string(msg), loc, do_log);
  }

  template <typename Enum, typename ToStringFunc>
  [[noreturn]] inline void ThrowCL(Enum code,
                                   ToStringFunc to_string,
                                   std::string_view context,
                                   std::source_location loc = std::source_location::current()) {
    std::string msg = std::format("{} (code {}): {}", context, static_cast<int>(code),
                                  to_string(static_cast<int>(code)));
    Throw<GGEMSRecoverable>(msg, loc, true);
  }

  template <typename Enum, typename ToStringFunc>
  [[noreturn]] inline void ThrowFatalCL(Enum code,
                                        ToStringFunc toString,
                                        std::string_view context,
                                        std::source_location loc = std::source_location::current()) {
    std::string msg = std::format("{} (code {}): {}", context, static_cast<int>(code),
                                  toString(static_cast<int>(code)));
    Throw<GGEMSFatal>(msg, loc, true);
  }

  [[noreturn]] void TerminateHandler() noexcept;
} // namespace ggems::core
