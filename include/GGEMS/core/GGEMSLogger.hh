#pragma once

/// \cond
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <source_location>
#include <fstream>
#include <string_view>
#include <memory>
#include <optional>
#include <vector>
/// \endcond

#include "GGEMS/core/GGEMSCoreUtils.hh"
#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::core {

enum class LogLevel : std::uint8_t { Debug = 0, Info, Warn, Error };
enum class Encoding : std::uint8_t { Utf32 = 0, Ascii };

struct LogRecord {
  std::chrono::system_clock::time_point timestamp{};
  LogLevel level{LogLevel::Info};
  std::int32_t depth{0};
  std::string thread_id{};
  std::string module{};
  std::string message{};
  std::string function{};
  std::string file{};
  std::int32_t line{0};
};

struct RenderedLogLine {
  std::string prefix{};
  std::string msg{};
  render::ColourKey color{render::DEFAULT_FG};
  LogLevel level{LogLevel::Info};
  std::int32_t depth{0};
  std::string module{};
};

class LogSink {
public:
  virtual ~LogSink() = default;
  virtual void Write(RenderedLogLine &&log_line) = 0;
};

class FileSink final : public LogSink {
public:
  explicit FileSink(std::string path);

  void Write(RenderedLogLine &&log_line) override;

private:
  std::string path_{};
  std::ofstream out_;
  std::mutex mtx_;
};

class StdoutSink : public LogSink {
public:
  StdoutSink() = default;

  void Write(RenderedLogLine &&log_line) override;

private:
  std::mutex mtx_;
};

class LogFormatter {
public:
  RenderedLogLine Format(LogRecord const &rec, bool use_color) const;
};

class GGEMSLogger {
public:
  static GGEMSLogger &GetInstance();

  GGEMSLogger(GGEMSLogger const &) = delete;
  GGEMSLogger(GGEMSLogger &&) = delete;
  GGEMSLogger &operator=(GGEMSLogger const &) = delete;
  GGEMSLogger &operator=(GGEMSLogger &&) = delete;

public:
  void ClearSinks() noexcept;
  void AddSink(std::unique_ptr<LogSink> sink);
  void SetSink(std::unique_ptr<LogSink> sink);

  void SetForceColor(bool force);
  bool UseColour() const noexcept;
  inline Encoding GetEncoding() const noexcept { return encoding_; }
  void SetForceEncoding(Encoding encoding) noexcept;

  void SetDetailLevel(std::int32_t d) noexcept {
    detail_level_.store(d, std::memory_order_relaxed);
  }

  void Log(LogLevel lvl, std::int32_t depth, std::string_view module,
           std::source_location const &loc = std::source_location::current(),
           std::string_view msg = "") {
    LogRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.level = lvl;
    rec.depth = depth;
    rec.thread_id = ThreadTag();
    rec.module = std::string(module);
    rec.message = msg;
    rec.function = SimplifyFunctionName(loc.function_name());
    rec.file = loc.file_name();
    rec.line = static_cast<int>(loc.line());
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
    Log(Level, depth, module, loc, s);
  }

private:
  GGEMSLogger() = default;
  void Dispatch(LogRecord const &rec);

#ifdef _WIN32
  bool EnableUtf32Win32();
#else
  bool EnableUtf32Unix();
#endif

private:
  std::atomic<std::int32_t> detail_level_{1};
  mutable std::mutex mtx_{};
  std::vector<std::unique_ptr<LogSink>> sinks_;
  LogFormatter formatter_{};
  std::optional<bool> force_colour_{};
  Encoding encoding_{Encoding::Ascii};
};
} // namespace ggems::core
