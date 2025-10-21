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
 * \brief GGEMSLogger class redefined C++ standard output (on the terminal)
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-07
 * \copyright GNU General Public License v3.0
 * \version 2.0
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

std::ostream& gglog::io(gglog::Level const& level, std::string_view class_name, std::string_view method_name) {
  if (!gglog::local.IsValidLogLevel(level)) {
    gglog::local.osstream_.setstate(std::ios::badbit);
  }
  else {
    gglog::local.level_ = level;
    gglog::local.class_name_ = class_name;
    gglog::local.method_name_ = method_name;
  }
  return gglog::local.osstream_;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& gglog::endl(std::ostream& ostream) {
  ostream << std::endl;
  if(!gglog::local.osstream_.bad()) {
    gglog::local.WriteMessage();
  }

  // reset stream-data (and state)
  gglog::local.osstream_.str(std::string());
  gglog::local.osstream_.clear();
  gglog::local.class_name_.clear();
  gglog::local.method_name_.clear();

  return gglog::local.osstream_;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSLogger::LogMessage(gglog::Level const& level, std::string_view message, std::string_view class_name, std::string_view method_name) {
  if (!IsValidLogLevel(level)) return;

  std::lock_guard<std::mutex> guard(write_lock_);

  #ifdef _WIN32
  // Get current color of terminal
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  FlushConsoleInputBuffer(hConsole);
  #endif

  if (level == gglog::Level::ERR) {
    #ifdef _WIN32
    SetConsoleTextAttribute(hConsole, 0x04); // 0x04 means red
    std::cout << "[GGEMS " << gglog::ToString(level) << "] ";
    if (!class_name.empty() || !method_name.empty()) {
      std::cout << "(" << class_name << "::" << method_name << ") ";
    }
    SetConsoleTextAttribute(hConsole, info.wAttributes);
    #else
    std::cout << "\033[31m"
              << "[GGEMS " << gglog::ToString(level) << "] ("
              << class_name << "::" << method_name << ") "
              << "\033[0m";
    #endif
  }
  else if (level == gglog::Level::WARNING || level == gglog::Level::DEBUG) {
    #ifdef _WIN32
    SetConsoleTextAttribute(hConsole, 0x06); // 0x06 means yellow
    std::cout << "[GGEMS " << gglog::ToString(level) << "] ";
    if (!class_name.empty() || !method_name.empty()) {
      std::cout << "(" << class_name << "::" << method_name << ") ";
    }
    SetConsoleTextAttribute(hConsole, info.wAttributes);
    #else
    std::cout << "\033[33m"
              << "[GGEMS " << gglog::ToString(level) << "] ("
              << class_name << "::" << method_name << ") "
              << "\033[0m";
    #endif
  }
  else {
    #ifdef _WIN32
    SetConsoleTextAttribute(hConsole, 0x02); // 0x02 means green
    std::cout << "[GGEMS " << gglog::ToString(level) << "] ";
    if (!class_name.empty() || !method_name.empty()) {
      std::cout << "(" << class_name << "::" << method_name << ") ";
    }
    SetConsoleTextAttribute(hConsole, info.wAttributes);
    #else
    std::cout << "\033[32m"
              << "[GGEMS " << gglog::ToString(level) << "] ("
              << class_name << "::" << method_name << ") "
              << "\033[0m";
    #endif
  }

  std::cout << message;
  std::cout.flush();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string gglog::ToString(gglog::Level const& level) {
  switch(level) {
    case gglog::Level::INFO:
      return "INFO";
    case gglog::Level::INFO2:
      return "INFO2";
    case gglog::Level::INFO3:
      return "INFO3";
    case gglog::Level::INFO4:
      return "INFO4";
    case gglog::Level::DEBUG:
      return "DEBUG";
    case gglog::Level::WARNING:
      return "WARNING";
    case gglog::Level::ERR:
      return "ERROR";
    default:
      return "";
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSLogger::SetLevelInfos(gglog::Level const& minimum_level) {
  minimum_level_ = minimum_level;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool GGEMSLogger::IsValidLogLevel(gglog::Level const& level) const {
  return (level <= minimum_level_
    || level == gglog::Level::ERR
    || level == gglog::Level::WARNING
    || level == gglog::Level::DEBUG) ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& stream, std::string_view str)
{
  std::copy(str.begin(), str.end(), std::ostream_iterator<char>(stream));
  return stream;
}
