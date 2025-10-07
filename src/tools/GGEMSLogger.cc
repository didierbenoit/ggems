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
 * \brief ...
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-07
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

#include "GGEMS/tools/GGEMSLogger.hh"

//GGEMSLogger::GGEMSLogger(void)
//  : minimum_log_level_(gglog::Level::INFO),
//  write_lock_() {}

//GGEMSLogger::~GGEMSLogger(void) {}

void GGEMSLogger::SetLevelInfos(gglog::Level const& minimum_log_level) {
  ;
}

void GGEMSLogger::LogMessage(gglog::Level const& log_level, std::string const& message, std::chrono::system_clock::time_point const& time) {
  ;
}

bool GGEMSLogger::IsValidLogLevel(gglog::Level const& log_level) const {
  return false;
}
