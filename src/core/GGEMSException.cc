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
 */

#include "GGEMS/core/GGEMSException.hh"

namespace ggems::core {
  void TerminateHandler() noexcept {
    try {
      if (auto ex = std::current_exception()) {
        try {
          std::rethrow_exception(ex);
        } catch (GGEMSExceptionBase const& e) {
          if (!e.Logged()) {
            GGEMSLogger::GetInstance().Error("Fatal", e.what());
          }
        } catch (std::exception const& e) {
          GGEMSLogger::GetInstance().Error("Fatal", e.what());
        } catch (...) {
          GGEMSLogger::GetInstance().Error("Fatal", "Unknown non-standard exception");
        }
      } else {
        GGEMSLogger::GetInstance().Error("Fatal", "Terminate called with no active exception");
      }
    } catch (...) {
      std::fputs("GGEMS Fatal Error: logger failed inside TerminateHandler\n", stderr);
    }

    std::abort();
  }
} // ggems::core
