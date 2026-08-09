#pragma once

#include <concepts>
#include <cstdint>
#include <format>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>

namespace ggems::core {
class GGEMSExceptionBase : public std::runtime_error {
public:
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

  [[nodiscard]] char const *what() const noexcept override {
    return full_.c_str();
  }

  [[nodiscard]] char const *GetFileName() const noexcept { return file_; }

  [[nodiscard]] char const *GetFunctionName() const noexcept {
    return function_;
  }

  [[nodiscard]] char const *GetCategory() const noexcept {
    return category_.c_str();
  }

  [[nodiscard]] std::int32_t GetLine() const noexcept { return line_; }

private:
  std::string full_;
  char const *file_;
  char const *function_;
  std::string category_;
  std::int32_t line_{0};
};

class GGEMSRecoverable : public GGEMSExceptionBase {
public:
  explicit GGEMSRecoverable(
      std::string msg,
      std::source_location loc = std::source_location::current())
      : GGEMSExceptionBase(std::move(msg), "Recoverable", loc) {}
};

class GGEMSInternal : public GGEMSExceptionBase {
public:
  explicit GGEMSInternal(
      std::string msg,
      std::source_location loc = std::source_location::current())
      : GGEMSExceptionBase(std::move(msg), "Internal", loc) {}
};

class GGEMSFatal : public GGEMSExceptionBase {
public:
  explicit GGEMSFatal(
      std::string msg,
      std::source_location loc = std::source_location::current())
      : GGEMSExceptionBase(std::move(msg), "Fatal", loc) {}
};

template <typename T>
concept GGEMSExceptionType = std::derived_from<T, GGEMSExceptionBase>;

void TerminateHandler() noexcept;
} // namespace ggems::core
