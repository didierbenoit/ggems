#pragma once

/// \cond
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
/// \endcond

#include "GGEMS/core/GGEMSCoreUtils.hh"

namespace ggems::core {
enum class LogLevel : std::uint8_t { Debug = 0, Info, Warn, Error };
enum class Encoding : std::uint8_t { Utf32 = 0, Ascii };

struct LogColorTheme {
  std::string debug_{"\033[36m"};
  std::string info_{"\033[32m"};
  std::string warn_{"\033[33m"};
  std::string error_{"\033[31m"};
  std::string reset_{"\033[0m"};
};

struct LogRecord {
  std::chrono::system_clock::time_point timestamp_{};
  LogLevel level_{LogLevel::Info};
  std::string thread_id_{};
  std::string module_{};
  std::string message_{};
  std::string function_{};
  std::string file_{};
  std::int32_t line_{0};
};

class LogSink {
public:
  virtual ~LogSink() = default;
  virtual void Write(LogRecord const &rec, std::string const &formatted) = 0;
};

class ConsoleSink final : public LogSink {
public:
  void Write(LogRecord const &rec, std::string const &formatted) override;
};

class FileSink final : public LogSink {
public:
  explicit FileSink(std::string path) : path_(std::move(path)) {}

  void Write(LogRecord const &rec, std::string const &formatted) override;

private:
  std::string path_{};
};

class LogFormatter {
public:
  std::string Format(LogRecord const &rec, LogColorTheme const &theme,
                     bool use_color) const;
};

class GGEMSLogger {
public:
  static GGEMSLogger &GetInstance();

  GGEMSLogger(GGEMSLogger const &) = delete;
  GGEMSLogger(GGEMSLogger const &&) = delete;
  GGEMSLogger &operator=(GGEMSLogger const &) = delete;
  GGEMSLogger &operator=(GGEMSLogger const &&) = delete;

public:
  void AttachSink(std::unique_ptr<LogSink> sink);
  inline void ClearSinks() {
    std::lock_guard<std::mutex> lock(mtx_);
    sinks_.clear();
  }

  void SetForceColor(std::optional<bool> force);
  bool UseColour() const noexcept;
  inline Encoding GetEncoding() const noexcept { return encoding_; }
  void SetForceEncoding(Encoding encoding) noexcept;

  void SetDetailLevel(std::int32_t d) noexcept {
    detail_level_.store(d, std::memory_order_relaxed);
  }

  template <typename... Args>
  void Log(LogLevel lvl, std::string_view module,
           std::source_location const &loc = std::source_location::current(),
           std::string_view msg = "") {
    LogRecord rec;
    rec.timestamp_ = std::chrono::system_clock::now();
    rec.level_ = lvl;
    rec.thread_id_ = ThreadTag();
    rec.module_ = std::string(module);
    rec.message_ = msg;
    rec.function_ = SimplifyFunctionName(loc.function_name());
    rec.file_ = loc.file_name();
    rec.line_ = static_cast<int>(loc.line());
    Dispatch(rec);
  }

  template <LogLevel Level, typename... Args>
  void LogFmt(std::int32_t depth, std::string_view module,
              std::string_view fmt_runtime,
              std::source_location loc = std::source_location::current(),
              Args &&...args) {
    std::int32_t dl = detail_level_.load(std::memory_order_relaxed);
    if (depth > dl)
      return;
    std::string s;
    if constexpr (sizeof...(Args) == 0) {
      s = std::string(fmt_runtime);
    } else {
      s = std::vformat(fmt_runtime, std::make_format_args(args...));
    }
    Log(Level, module, loc, s);
  }

private:
  GGEMSLogger();

  void Dispatch(LogRecord const &rec);

#ifdef _WIN32
  bool EnableUtf32Win32();
#else
  bool EnableUtf32Unix();
#endif

private:
  std::atomic<std::int32_t> detail_level_{1};
  mutable std::mutex mtx_;
  std::vector<std::unique_ptr<LogSink>> sinks_;
  LogFormatter formatter_{};
  LogColorTheme theme_{};
  std::optional<bool> force_colour_{};
  Encoding encoding_;
};
} // namespace ggems::core
