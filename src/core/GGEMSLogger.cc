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

/// \cond
#include <format>
#include <fstream>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif
/// \endcond

#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::core {
static std::string
FormatTimestamp(std::chrono::system_clock::time_point const &tp) {
  using namespace std::chrono;
  auto const t = system_clock::to_time_t(tp);
  auto const ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;
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

void ConsoleSink::Write(LogRecord const &rec, std::string const &formatted) {
  FILE *stream = (rec.level_ == LogLevel::Error) ? stderr : stdout;
  std::fwrite(formatted.data(), 1, formatted.size(), stream);
  std::fputc('\n', stream);
  std::fflush(stream);
}

void FileSink::Write(LogRecord const &, std::string const &formatted) {
  std::ofstream out(path_, std::ios::app);
  out << formatted << '\n';
}

std::string LogFormatter::Format(LogRecord const &rec,
                                 LogColorTheme const &theme,
                                 bool use_colour) const {
  char const *level_str = "INFO";
  char const *col = "";
  char const *reset = "";

  switch (rec.level_) {
  case LogLevel::Debug:
    level_str = "DEBUG";
    col = theme.debug_.c_str();
    break;
  case LogLevel::Info:
    level_str = "INFO";
    col = theme.info_.c_str();
    break;
  case LogLevel::Warn:
    level_str = "WARN";
    col = theme.warn_.c_str();
    break;
  case LogLevel::Error:
    level_str = "ERROR";
    col = theme.error_.c_str();
    break;
  }

  if (!use_colour)
    col = "", reset = "";
  else
    reset = theme.reset_.c_str();

  auto const ts = FormatTimestamp(rec.timestamp_);
  std::string module_part = rec.module_.empty() ? "" : " [" + rec.module_ + "]";

  return std::vformat("{}{} [{}] {{T{}}}{}{} ({}): {}",
                      std::make_format_args(col, ts, level_str, rec.thread_id_,
                                            reset, module_part, rec.function_,
                                            rec.message_));
}

GGEMSLogger &GGEMSLogger::GetInstance() {
  static GGEMSLogger instance;
  return instance;
}

GGEMSLogger::GGEMSLogger() {
#ifdef _WIN32
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
      dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwMode);
    }
  }
#endif
  sinks_.emplace_back(std::make_unique<ConsoleSink>());
}

void GGEMSLogger::AttachSink(std::unique_ptr<LogSink> sink) {
  std::lock_guard<std::mutex> lock(mtx_);
  sinks_.emplace_back(std::move(sink));
}

void GGEMSLogger::SetForceColor(std::optional<bool> force) {
  std::lock_guard<std::mutex> lock(mtx_);
  force_colour_ = force;
}

bool GGEMSLogger::UseColour() const noexcept {
  if (force_colour_.has_value())
    return *force_colour_;
  char const *no_color = std::getenv("NO_COLOR");
  if (no_color && *no_color)
    return false;
  return isatty(fileno(stdout)) != 0;
}

void GGEMSLogger::Dispatch(LogRecord const &rec) {
  std::string line = formatter_.Format(rec, theme_, UseColour());
  std::lock_guard<std::mutex> lock(mtx_);
  for (auto &s : sinks_)
    s->Write(rec, line);
}
} // namespace ggems::core
