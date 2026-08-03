#pragma once

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

#include "GGEMS/core/GGEMSCoreUtils.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMS/render/GGEMSColour.hh"

namespace ggems::core {

enum class LogLevel : std::uint8_t { Debug = 0, Info, Warn, Error };
enum class Encoding : std::uint8_t { Unicode = 0, Ascii };

struct LogRecord {
  std::chrono::system_clock::time_point timestamp;
  LogLevel level{LogLevel::Info};
  std::int32_t depth{0};
  std::string thread_id;
  std::string module;
  std::string message;
  std::string function;
  std::string file;
  std::int32_t line{0};
};

struct RenderedLogLine {
  std::string prefix;
  std::string msg;
  render::ColourKey color{render::DEFAULT_FG};
  LogLevel level{LogLevel::Info};
  std::int32_t depth{0};
  std::string module;
};

class LogSink {
public:
  virtual ~LogSink() = default;
  virtual auto Write(RenderedLogLine &&log_line) -> void = 0;
};

class FileSink final : public LogSink {
public:
  explicit FileSink(std::string path);

  auto Write(RenderedLogLine &&log_line) -> void override;

private:
  std::string path_;
  std::ofstream out_;
  std::mutex mtx_;
};

class StdoutSink : public LogSink {
public:
  StdoutSink() = default;

  auto Write(RenderedLogLine &&log_line) -> void override;

private:
  std::mutex mtx_;
};

class LogFormatter {
public:
  static auto Format(LogRecord const &rec, bool use_color) -> RenderedLogLine;
};

class GGEMSLogger {
public:
  static auto GetInstance() -> GGEMSLogger &;

  GGEMSLogger(GGEMSLogger const &) = delete;
  GGEMSLogger(GGEMSLogger &&) = delete;
  auto operator=(GGEMSLogger const &) -> GGEMSLogger & = delete;
  auto operator=(GGEMSLogger &&) -> GGEMSLogger & = delete;

  auto ClearSinks() noexcept -> void;
  auto AddSink(std::unique_ptr<LogSink> sink) -> void;
  auto SetSink(std::unique_ptr<LogSink> sink) -> void;

  auto SetForceColor(bool force) -> void;
  auto UseColour() const noexcept -> bool;
  auto GetEncoding() const noexcept -> Encoding { return encoding_; }
  auto SetForceEncoding(Encoding encoding) noexcept -> void;

  auto SetDetailLevel(std::int32_t detail_level) noexcept -> void {
    detail_level_.store(detail_level, std::memory_order_relaxed);
  }

  auto Log(LogLevel lvl, std::int32_t depth, std::string_view module,
           std::source_location const &loc = std::source_location::current(),
           std::string_view msg = "") -> void {
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
  auto LogFmt(std::int32_t depth, std::string_view module,
              std::string_view fmt_runtime,
              std::source_location loc = std::source_location::current(),
              Args &&...args) -> void {
    std::int32_t detail_level = detail_level_.load(std::memory_order_relaxed);
    if (depth > detail_level) {
      return;
    }

    std::string line;
    if constexpr (sizeof...(Args) == 0) {
      line = std::string(fmt_runtime);
    } else {
      line = std::vformat(fmt_runtime, std::make_format_args(args...));
    }
    Log(Level, depth, module, loc, line);
  }

private:
  GGEMSLogger() = default;
  void Dispatch(LogRecord const &rec);

#ifdef _WIN32
  auto EnableUnicodeWin32() -> bool;
#else
  auto EnableUnicodeUnix() -> bool;
#endif

  std::atomic<std::int32_t> detail_level_{1};
  mutable std::mutex mtx_;
  std::vector<std::unique_ptr<LogSink>> sinks_;
  std::optional<bool> force_colour_;
  Encoding encoding_{Encoding::Ascii};
};
} // namespace ggems::core
