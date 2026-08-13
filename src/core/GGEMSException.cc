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
 * \brief Implements the GGEMS termination handler.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string_view>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"

namespace {
/*!
 * \brief Writes an emergency diagnostic to standard error.
 *
 * Writes the non-empty prefix and message verbatim, followed by one newline.
 *
 * \param[in] prefix Diagnostic prefix.
 * \param[in] message Diagnostic message.
 */
auto WriteEmergencyDiagnostic(std::string_view prefix,
                              std::string_view message = {}) noexcept -> void {
  if (!prefix.empty()) {
    std::fwrite(prefix.data(), sizeof(char), prefix.size(), stderr);
  }
  if (!message.empty()) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
  }
  std::fputc('\n', stderr);
}
} // namespace

// =============================================================================
// =============================================================================

namespace ggems::core {
void TerminateHandler() noexcept {
  try {
    auto exception = std::current_exception();
    if (exception) {
      try {
        std::rethrow_exception(exception);
      } catch (GGEMSExceptionBase const &caught_exception) {
        WriteEmergencyDiagnostic({}, caught_exception.what());
      } catch (std::exception const &caught_exception) {
        WriteEmergencyDiagnostic("[std::exception] ", caught_exception.what());
      } catch (...) {
        WriteEmergencyDiagnostic("[Unknown exception]");
      }
    } else {
      WriteEmergencyDiagnostic(
          "[GGEMSException] Terminate called with no active exception");
    }
  } catch (...) {
    std::fputs("[GGEMSException] Exception escaped TerminateHandler\n", stderr);
  }
  std::abort();
}
} // namespace ggems::core
