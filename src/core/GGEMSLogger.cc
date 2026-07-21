/// \cond
#include <iostream>
#include <format>
#ifdef _WIN32
#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"
#else
#include <unistd.h>
#endif
/// \endcond

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/render/GGEMSColour.hh"
#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::core {

// =============================================================================
// =============================================================================

static auto LogLevelColour(LogLevel lvl) -> render::ColourKey {
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

static std::string LogLevelName(LogRecord const &rec) {
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

static std::string
FormatTimestamp(std::chrono::system_clock::time_point const &tp) {
  using namespace std::chrono;
  auto t = system_clock::to_time_t(tp);
  auto ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;
  std::tm tm_buf{};
#ifdef _WIN32
  localtime_s(&tm_buf, &t);
#else
  localtime_r(&t, &tm_buf);
#endif
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, (int)ms.count());
  return std::string(buf);
}

// =============================================================================
// =============================================================================

[[nodiscard]] inline std::optional<std::string>
GetEnvVar(const char *name) noexcept {
#if defined(_WIN32)
  char *buffer = nullptr;
  std::size_t len = 0;
  if (_dupenv_s(&buffer, &len, name) == 0 && buffer) {
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

  bool use_colour = GGEMSLogger::GetInstance().UseColour();

  if (!log_line.prefix.empty()) {
    if (use_colour) {
      std::cout << render::AnsiColour(log_line.color) << log_line.prefix
                << render::AnsiControlCode(render::AnsiControl::ResetColour)
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
    : path_(path), out_(path_, std::ios::out | std::ios::trunc) {
  GGEMS_CHECK_FATAL(out_, "Cannot open log file: " + path_);
}

// -----------------------------------------------------------------------------

void FileSink::Write(RenderedLogLine &&log_line) {
  std::scoped_lock lock(mtx_);

  if (!log_line.prefix.empty()) {
    out_ << log_line.prefix << ' ';
  }

  out_ << log_line.msg << '\n';
}

// =============================================================================
// =============================================================================

RenderedLogLine LogFormatter::Format(LogRecord const &rec,
                                     bool use_colour) const {
  RenderedLogLine log_line;
  log_line.msg = rec.message;
  log_line.level = rec.level;
  log_line.depth = rec.depth;
  log_line.module = rec.module;

  if (use_colour) {
    log_line.color = LogLevelColour(rec.level);
  }

  auto const ts = FormatTimestamp(rec.timestamp);
  std::string module_part = rec.module.empty() ? "" : " [" + rec.module + "]";
  std::string level_name = LogLevelName(rec);

  log_line.prefix = std::format("{} [{}] {{{}}}{} ({}):", ts, level_name,
                                rec.thread_id, module_part, rec.function);
  return log_line;
}

// =============================================================================
// =============================================================================

GGEMSLogger &GGEMSLogger::GetInstance() {
  static GGEMSLogger instance;
  return instance;
}

// -----------------------------------------------------------------------------

void GGEMSLogger::ClearSinks() noexcept {
  std::vector<std::unique_ptr<LogSink>> sinks_to_delete;

  {
    std::scoped_lock lock(mtx_);
    sinks_to_delete = std::move(sinks_);
  }
}

// -----------------------------------------------------------------------------

void GGEMSLogger::AddSink(std::unique_ptr<LogSink> sink) {
  if (!sink) {
    GGEMS_FATAL("Log sink is null.");
  }

  std::scoped_lock lock(mtx_);
  sinks_.push_back(std::move(sink));
}

// -----------------------------------------------------------------------------

void GGEMSLogger::SetSink(std::unique_ptr<LogSink> sink) {
  ClearSinks();
  AddSink(std::move(sink));
}

// -----------------------------------------------------------------------------

void GGEMSLogger::SetForceColor(bool force) {
  std::scoped_lock lock(mtx_);
  force_colour_ = force;
}

// -----------------------------------------------------------------------------

void GGEMSLogger::SetForceEncoding(Encoding encoding) noexcept {
  std::scoped_lock lock(mtx_);
  encoding_ = encoding;
}

// -----------------------------------------------------------------------------

bool GGEMSLogger::UseColour() const noexcept {
  if (force_colour_.has_value())
    return *force_colour_;

  if (auto no_color = GetEnvVar("NO_COLOR"); no_color && !no_color->empty())
    return false;

  return true;
}

// -----------------------------------------------------------------------------

void GGEMSLogger::Dispatch(LogRecord const &rec) {
  RenderedLogLine log_line = formatter_.Format(rec, UseColour());

  std::scoped_lock lock(mtx_);

  for (std::unique_ptr<LogSink> const &sink : sinks_) {
    sink->Write(RenderedLogLine{log_line});
  }
}
} // namespace ggems::core
