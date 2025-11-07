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
 * \file GGEMSException.cc
 * \brief Definition of GGEMSException for handling GGEMS-specific error
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <format>
#include <string_view>
#include <utility>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::core
{
  GGEMSExceptionBase::GGEMSExceptionBase(std::string message, std::string_view category, std::source_location loc) noexcept
  : payload_(std::move(message))
  , file_(loc.file_name() ? loc.file_name() : "")
  , function_(loc.function_name() ? loc.function_name() : "")
  , line_(static_cast<int>(loc.line()))
  , category_(category)
  {
    full_ = std::vformat("[{}] {}:{} ({}) : {}",
                         std::make_format_args(category_, file_, line_, function_, payload_));
    GGEMSLogger::GetInstance().Error("Exception", "[{}] {}:{} ({}) : {}",
                                     std::source_location::current(),
                                     category_, file_, line_, function_, payload_);
  }
} // namespace ggems::core
