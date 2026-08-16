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
 * \brief Scoped logger-encoding helper for GGEMS tests.
 *
 * Provides a noncopyable RAII utility used by tests that must validate output under a specific ASCII or Unicode logger encoding.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::test {
/*!
 * \brief RAII helper that temporarily forces the GGEMS logger encoding.
 *
 * Restores the previous logger encoding when the helper leaves scope.
 */
class ScopedLoggerEncoding final {
public:
  /*!
   * \brief Forces a logger encoding for the lifetime of this helper.
   *
   * \param[in] encoding Encoding to force while the helper is alive.
   */
  explicit ScopedLoggerEncoding(core::Encoding encoding)
      : logger_{core::GGEMSLogger::GetInstance()},
        previous_encoding_{logger_.GetEncoding()} {
    logger_.SetForceEncoding(encoding);
  }

  /*!
   * \brief Restores the logger encoding active at construction time.
   */
  ~ScopedLoggerEncoding() noexcept {
    logger_.SetForceEncoding(previous_encoding_);
  }

  /*! \brief Copy construction is disabled. */
  ScopedLoggerEncoding(ScopedLoggerEncoding const &) = delete;
  /*! \brief Move construction is disabled. */
  ScopedLoggerEncoding(ScopedLoggerEncoding &&) = delete;
  /*! \brief Copy assignment is disabled. */
  auto operator=(ScopedLoggerEncoding const &)
      -> ScopedLoggerEncoding & = delete;
  /*! \brief Move assignment is disabled. */
  auto operator=(ScopedLoggerEncoding &&) -> ScopedLoggerEncoding & = delete;

private:
  /*! \brief GGEMS logger instance whose encoding is temporarily overridden. */
  core::GGEMSLogger &logger_;
  /*! \brief Logger encoding saved for restoration at destruction. */
  core::Encoding previous_encoding_;
};
} // namespace ggems::test
