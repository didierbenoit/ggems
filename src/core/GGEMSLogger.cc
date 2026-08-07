#include <iostream>
#include <format>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <array>
#include <optional>
#include <cstdlib>
#include <mutex>
#include <utility>
#include <memory>
#include <vector>
#include <string_view>
#include <cstdint>
#include <source_location>

#ifdef _WIN32
#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"
#else
#include <unistd.h>
#endif

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/core/detail/GGEMSLoggerMetadata.hh"

namespace ggems::core {

// =============================================================================
// =============================================================================

static auto LogLevelColor(LogLevel lvl) -> render::ColorKey {
  switch (lvl) {
  case LogLevel::Debug:
    return render::CYAN_Cryo;
  case LogLevel::Info:
    return render::GREEN_Acid;
  case LogLevel::Warn:
    return render::YELLOW_MotherAmber;
  case LogLevel::Error:
    return render::RED_XenoBlood;
  }
  return render::GREEN_Acid;
}

// =============================================================================
// =============================================================================

static auto LogLevelName(LogRecord const &rec) -> std::string {
  switch (rec.level) {
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    if (rec.depth > 0) {
      return std::format("INFO{}", rec.depth);
    }
    return "INFO";
  case LogLevel::Warn:
    return "WARN";
  case LogLevel::Error:
    return "ERROR";
  }
  return "unknown";
}

// =============================================================================
// =============================================================================

static auto
FormatTimestamp(std::chrono::system_clock::time_point const &time_point)
    -> std::string {
  using namespace std::chrono;
  auto time = system_clock::to_time_t(time_point);
  auto m_sec =
      duration_cast<milliseconds>(time_point.time_since_epoch()) % 1000;
  std::tm tm_buf{};
#ifdef _WIN32
  localtime_s(&tm_buf, &time);
#else
  localtime_r(&time, &tm_buf);
#endif
  std::array<char, 64> buf;
  std::snprintf(buf.data(), buf.size(), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
                (int)m_sec.count());

  return std::string{buf.data()};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetEnvVar(const char *name) noexcept
    -> std::optional<std::string> {
#if defined(_WIN32)
  char *buffer = nullptr;
  std::size_t len = 0;
  if (_dupenv_s(&buffer, &len, name) == 0 && buffer != nullptr) {
    std::string value(buffer);
    std::free(buffer);
    return value;
  }
  return std::nullopt;
#else
  if (const char *value = std::getenv(name))
    return std::string(value);
  return std::nullopt;
#endif
}

// =============================================================================
// =============================================================================

void StdoutSink::Write(RenderedLogLine &&log_line) {
  std::scoped_lock lock(mtx_);

  bool use_color = GGEMSLogger::GetInstance().UseColor();

  if (!log_line.prefix.empty()) {
    if (use_color) {
      std::cout << render::AnsiColor(log_line.color) << log_line.prefix
                << render::AnsiControlCode(render::AnsiControl::ResetColor)
                << ' ';
    } else {
      std::cout << log_line.prefix << ' ';
    }
  }

  std::cout << log_line.msg << '\n';
}

// =============================================================================
// =============================================================================

FileSink::FileSink(std::string path)
    : path_(std::move(path)), out_(path_, std::ios::out | std::ios::trunc) {
  GGEMS_CHECK_FATAL(out_, "Cannot open log file: " + path_);
}

// -----------------------------------------------------------------------------

auto FileSink::Write(RenderedLogLine &&log_line) -> void {
  std::scoped_lock lock(mtx_);

  if (!log_line.prefix.empty()) {
    out_ << log_line.prefix << ' ';
  }

  out_ << log_line.msg << '\n';
}

// =============================================================================
// =============================================================================

auto LogFormatter::Format(LogRecord const &rec, bool use_color)
    -> RenderedLogLine {
  RenderedLogLine log_line;
  log_line.msg = rec.message;
  log_line.level = rec.level;
  log_line.depth = rec.depth;
  log_line.module = rec.module;

  if (use_color) {
    log_line.color = LogLevelColor(rec.level);
  }

  auto const time_stamp = FormatTimestamp(rec.timestamp);
  std::string module_part = rec.module.empty() ? "" : " [" + rec.module + "]";
  std::string level_name = LogLevelName(rec);

  log_line.prefix =
      std::format("{} [{}] {{{}}}{} ({}):", time_stamp, level_name,
                  rec.thread_id, module_part, rec.function);
  return log_line;
}

// =============================================================================
// =============================================================================

auto GGEMSLogger::GetInstance() -> GGEMSLogger & {
  static GGEMSLogger instance;
  return instance;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::Log(LogLevel lvl, std::int32_t depth, std::string_view module,
                      std::source_location const &loc, std::string_view msg)
    -> void {
  LogRecord rec;
  rec.timestamp = std::chrono::system_clock::now();
  rec.level = lvl;
  rec.depth = depth;
  rec.thread_id = logging::detail::ThreadTag();
  rec.module = std::string(module);
  rec.message = msg;
  rec.function = logging::detail::SimplifyFunctionName(loc.function_name());
  rec.file = loc.file_name();
  rec.line = static_cast<int>(loc.line());
  Dispatch(rec);
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::ClearSinks() noexcept -> void {
  std::vector<std::unique_ptr<LogSink>> sinks_to_delete;

  {
    std::scoped_lock lock(mtx_);
    sinks_to_delete = std::move(sinks_);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::AddSink(std::unique_ptr<LogSink> sink) -> void {
  if (!sink) {
    GGEMS_FATAL("Log sink is null.");
  }

  std::scoped_lock lock(mtx_);
  sinks_.push_back(std::move(sink));
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::SetSink(std::unique_ptr<LogSink> sink) -> void {
  ClearSinks();
  AddSink(std::move(sink));
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::SetForceColor(bool force) -> void {
  std::scoped_lock lock(mtx_);
  force_color_ = force;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::SetForceEncoding(Encoding encoding) noexcept -> void {
  std::scoped_lock lock(mtx_);
  encoding_ = encoding;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::UseColor() const noexcept -> bool {
  if (force_color_.has_value()) {
    return *force_color_;
  }

  if (auto no_color = GetEnvVar("NO_COLOR"); no_color && !no_color->empty()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::Dispatch(LogRecord const &rec) -> void {
  RenderedLogLine log_line = LogFormatter::Format(rec, UseColor());

  std::scoped_lock lock(mtx_);

  for (std::unique_ptr<LogSink> const &sink : sinks_) {
    sink->Write(RenderedLogLine{log_line});
  }
}
} // namespace ggems::core
