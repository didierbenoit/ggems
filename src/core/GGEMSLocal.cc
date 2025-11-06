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
#include <thread>
#include <utility>
/// \endcond
#include "GGEMS/core/GGEMSLocal.hh"

namespace ggems::core {
  static thread_local LocalState g_local_state{};
  LocalState& local() { return g_local_state; }

  ScopedModule::ScopedModule(std::string module)
  : prev_(g_local_state.module_) {
    g_local_state.module_ = std::move(module);
  }

  ScopedModule::~ScopedModule() {
    g_local_state.module_ = std::move(prev_);
  }

  ScopedIndent::ScopedIndent() {
    ++g_local_state.indent_;
  }

  ScopedIndent::~ScopedIndent() {
    --g_local_state.indent_;
  }
} //namespace ggems::core
