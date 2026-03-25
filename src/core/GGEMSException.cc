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
 * \brief Base exception class and specialised GGEMS exception categories.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/core/GGEMSException.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSExceptionBase::Log(std::string const &msg) noexcept {
  try {
    std::fputs(msg.c_str(), stderr);
    std::fputs("\n", stderr);
  } catch (...) {
    std::fputs("[GGEMS Exception]: logging failed\n", stderr);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void TerminateHandler() noexcept {
  try {
    auto ex = std::current_exception();
    if (ex) {
      try {
        std::rethrow_exception(ex);
      } catch (GGEMSExceptionBase const &e) {
        GGEMSExceptionBase::Log(e.what());
      } catch (std::exception const &e) {
        GGEMSExceptionBase::Log(std::string("[std::exception] ") + e.what());
      } catch (...) {
        GGEMSExceptionBase::Log("[Unknown exception]");
      }
    } else {
      GGEMSExceptionBase::Log(
          "[GGEMSException] Terminate called with no active exception");
    }
  } catch (...) {
    std::fputs("[GGEMSException] Exception escaped TerminateHandler\n", stderr);
  }
  std::abort();
}
} // namespace ggems::core
