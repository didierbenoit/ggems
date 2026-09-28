// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/logging/GGEMSLogger.hh"

namespace ggems::test {

class ScopedLoggerEncoding {
public:
  explicit ScopedLoggerEncoding(core::Encoding encoding)
      : logger_{core::GGEMSLogger::GetInstance()},
        previous_encoding_{logger_.GetEncoding()} {
    logger_.SetForceEncoding(encoding);
  }

  ~ScopedLoggerEncoding() noexcept {
    logger_.SetForceEncoding(previous_encoding_);
  }

  ScopedLoggerEncoding(ScopedLoggerEncoding const &) = delete;
  ScopedLoggerEncoding(ScopedLoggerEncoding &&) = delete;
  auto operator=(ScopedLoggerEncoding const &)
    -> ScopedLoggerEncoding & = delete;
  auto operator=(ScopedLoggerEncoding &&) -> ScopedLoggerEncoding & = delete;

private:
  core::GGEMSLogger &logger_;
  core::Encoding previous_encoding_;
};
} // namespace ggems::test
