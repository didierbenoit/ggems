#pragma once

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
#include <cstdint>
#include <string>
#include <chrono>
#include <thread>
#include <optional>
#include <source_location>
#include <string_view>
#include <mutex>
/// \endcond

namespace ggems::core {
  template <typename... Args>
  [[nodiscard]] inline std::string FormatRuntime(std::string_view fmt, Args&&... args) {
    return std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
  }

  enum class LogLevel : std::uint8_t {
    Debug = 0,
    Info,
    Warn,
    Error
  };

  struct LogColorTheme {
    std::string debug_{"\033[36m"};
    std::string info_{"\033[32m"};
    std::string warn_{"\033[33m"};
    std::string error_{"\033[31m"};
    std::string reset_{"\033[0m"};
  };

  struct LogRecord {
    std::chrono::system_clock::time_point timestamp_{};
    LogLevel        level_{LogLevel::Info};
    std::thread::id thread_id_{};
    std::string     module_{};
    std::string     message_{};
    std::string     function_{};
    std::string     file_{};
    int             line_{0};
  };

  class LogSink {
  public:
    virtual ~LogSink() = default;
    virtual void Write(LogRecord const& rec, std::string const& formatted) = 0;
  };

  class ConsoleSink final : public LogSink {
  public:
    void Write(LogRecord const& rec, std::string const& formatted) override;
  };

  class FileSink final : public LogSink {
  public:
    explicit FileSink(std::string path) : path_(std::move(path)) {}

    void Write(LogRecord const& rec, std::string const& formatted) override;
  private:
    std::string path_{};
  };

  class LogFormatter {
  public:
    std::string Format(LogRecord const& rec, LogColorTheme const& theme, bool use_color) const;
  };

  class GGEMSLogger {
  public:
    static GGEMSLogger& GetInstance();

    GGEMSLogger(GGEMSLogger const&) = delete;
    GGEMSLogger& operator=(GGEMSLogger const&) = delete;

  public:
    void AttachSink(std::unique_ptr<LogSink> sink);
    void SetForceColor(std::optional<bool> force);
    void SetDetailLevel(int d) noexcept { std::lock_guard<std::mutex> lock(mtx_); detail_level_ = d; }

    template <typename... Args>
    void Log(LogLevel lvl, std::string_view module, std::format_string<Args...> fmt,
             std::source_location const& loc = std::source_location::current(),
             Args&&... args) {
      LogRecord rec;
      rec.timestamp_ = std::chrono::system_clock::now();
      rec.level_ = lvl;
      rec.thread_id_ = std::this_thread::get_id();
      rec.module_ = std::string(module);
      rec.message_ = std::vformat(fmt.get(), std::make_format_args(std::forward<Args>(args)...));
      rec.function_ = loc.function_name();
      rec.file_ = loc.file_name();
      rec.line_ = static_cast<int>(loc.line());
      Dispatch(rec);
    }

    template <typename... Args>
    void Debug(std::string_view module, std::format_string<Args...> fmt,
               std::source_location const& loc = std::source_location::current(), Args&&... args) {
      Log(LogLevel::Debug, module, fmt, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Info(std::string_view module, std::format_string<Args...> fmt, Args&&... args) {
      auto loc = std::source_location::current();
      Log(LogLevel::Info, module, fmt, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Warn(std::string_view module, std::format_string<Args...> fmt,
              std::source_location const& loc = std::source_location::current(), Args&&... args) {
      Log(LogLevel::Warn, module, fmt, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(
        std::string_view module,
        std::format_string<Args...> fmt,
        std::source_location const& loc = std::source_location::current(), Args&&... args) {
      Log(LogLevel::Error, module, fmt, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void InfoEx(
        int depth,
        std::string_view module,
        std::format_string<Args...> fmt,
        std::source_location const& loc = std::source_location::current(), Args&&... args) {
      if (depth > detail_level_) return;
      Log(LogLevel::Info, module, fmt, loc, std::forward<Args>(args)...);
    }

    void Error( std::string_view module, std::string_view msg,
      std::source_location const& loc = std::source_location::current()
    ) noexcept;

  private:
    GGEMSLogger();

    void Dispatch(LogRecord const& rec);
    bool UseColour() const noexcept;

  private:
    int                                   detail_level_{1};
    mutable std::mutex                    mtx_;
    std::vector<std::unique_ptr<LogSink>> sinks_;
    LogFormatter                          formatter_{};
    LogColorTheme                         theme_{};
    std::optional<bool>                   force_colour_{};
  };
} //namespace ggems::core
