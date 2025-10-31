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

/*!
 * \file GGEMSLogger.cc
 * \brief Implementation of GGEMSLogger and gglog front-end.
 */

/// \cond
#ifdef _WIN32
  #ifdef _MSC_VER
    #define NOMINMAX
  #endif
  #include <Windows.h>
#endif

#include <iterator>
#include <sstream>
#include <iostream>
#include <algorithm>
/// \endcond

#include "GGEMS/tools/GGEMSLocal.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& gglog::io(gglog::Level level, std::string_view class_name, std::string_view method_name) {
  if (!GGEMSLogger::GetInstance().IsVisible(level)) {
    gglog::local.osstream_.setstate(std::ios::badbit);
  } else {
    gglog::local.level_       = level;
    gglog::local.class_name_  = class_name;
    gglog::local.method_name_ = method_name;
  }
  return gglog::local.osstream_;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& gglog::endl(std::ostream& ostream) {
  ostream << '\n';
  if (!gglog::local.osstream_.bad()) {
    gglog::local.WriteMessage();
  }

  // Reset per-thread staging stream and context.
  gglog::local.osstream_.str(std::string());
  gglog::local.osstream_.clear();
  gglog::local.class_name_.clear();
  gglog::local.method_name_.clear();

  return gglog::local.osstream_;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSLogger::LogMessage(gglog::Level level, std::string_view message,
  std::string_view class_name, std::string_view method_name) {
  if (!IsVisible(level)) return;

  std::lock_guard<std::mutex> guard(write_lock_);

  #ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO info{};
  const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleScreenBufferInfo(hConsole, &info);

  auto set_colour = [&](WORD attr) {
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, attr);
  };

  auto restore_colour = [&]() {
    SetConsoleTextAttribute(hConsole, info.wAttributes);
  };

  auto emit_header = [&](WORD colour) {
    set_colour(colour);
    std::cout << "[GGEMS " << gglog::ToString(level) << "] ";
    if (!class_name.empty() || !method_name.empty()) {
      std::cout << "(" << class_name << "::" << method_name << ") ";
    }
    restore_colour();
  };

  if (level == gglog::Level::ERR) {
    emit_header(0x04);
  } else if (level == gglog::Level::WARNING || level == gglog::Level::DEBUG) {
    emit_header(0x06);
  } else {
    emit_header(0x02);
  }
  #else
  auto emit_header = [&](std::string_view code) {
    std::cout << code << "[GGEMS " << gglog::ToString(level) << "] ("
      << class_name << "::" << method_name << ") " << "\033[0m";
  };
  if (level == gglog::Level::ERR) {
    emit_header("\033[31m");
  } else if (level == gglog::Level::WARNING || level == gglog::Level::DEBUG) {
    emit_header("\033[33m");
  } else {
    emit_header("\033[32m");
  }
  #endif

  std::cout << message;
  std::cout.flush();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSLogger::SetLevelInfos(gglog::Level minimum_level) noexcept {
  minimum_level_ = minimum_level;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool GGEMSLogger::IsVisible(gglog::Level level) const noexcept {
  // Preserve previous semantics: the threshold controls INFO tiers,
  // while WARNING/DEBUG/ERR remain visible independent of the threshold.
  return (level <= minimum_level_)
      || level == gglog::Level::ERR
      || level == gglog::Level::WARNING
      || level == gglog::Level::DEBUG;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& stream, std::string_view str) {
  std::copy(str.begin(), str.end(), std::ostream_iterator<char>(stream));
  return stream;
}
