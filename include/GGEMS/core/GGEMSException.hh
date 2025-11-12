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
#include <format>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
/// \endcond

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::core {
class GGEMSExceptionBase : public std::runtime_error {
public:
  explicit GGEMSExceptionBase(
      std::string const &msg, std::string const &cat,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : std::runtime_error(msg), file_(loc.file_name()),
        function_(loc.function_name()), category_(cat),
        line_(static_cast<int>(loc.line())) {
    full_ = std::format("\n[GGEMS Exception]\n"
                        "  Type     : {}\n"
                        "  File     : {}\n"
                        "  Line     : {}\n"
                        "  Function : {}\n"
                        "  Message  : {}\n",
                        category_, file_, line_, function_, msg);
    if (do_log) {
      try {
        GGEMS_ERROR("Exception", full_);
        logged_ = true;
      } catch (...) {
        std::fputs("GGEMS: logging failed in GGEMSExceptionBase\n", stderr);
      }
    }
  }

  bool Logged() const noexcept { return logged_; }

protected:
  std::string full_;
  std::string file_;
  std::string function_;
  std::string category_;
  int line_;
  bool logged_{false};
};

class GGEMSRecoverable final : public GGEMSExceptionBase {
public:
  explicit GGEMSRecoverable(
      std::string const msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : GGEMSExceptionBase(std::move(msg), "Recoverable", loc, do_log) {}
};

class GGEMSInternal final : public GGEMSExceptionBase {
public:
  explicit GGEMSInternal(
      std::string const msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : GGEMSExceptionBase(std::move(msg), "Internal", loc, do_log) {}
};

class GGEMSFatal final : public GGEMSExceptionBase {
public:
  explicit GGEMSFatal(
      std::string const msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : GGEMSExceptionBase(std::move(msg), "Fatal", loc, do_log) {}
};

template <typename T>
concept GGEMSExceptionType = std::derived_from<T, GGEMSExceptionBase>;

template <GGEMSExceptionType E>
[[noreturn]] inline void
Throw(std::string_view msg,
      std::source_location loc = std::source_location::current(),
      bool do_log = true) {
  throw E(std::string(msg), loc, do_log);
}

[[noreturn]] void TerminateHandler() noexcept;
} // namespace ggems::core
