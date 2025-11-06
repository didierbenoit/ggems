#pragma once

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

#include <string>

namespace ggems::core {
  struct LocalState {
    std::string module_{};
    int indent_{0};
  };

  LocalState& local();

  class ScopedModule {
  public:
    explicit ScopedModule(std::string module);
    ~ScopedModule();

    ScopedModule(const ScopedModule&) = delete;
    ScopedModule& operator=(const ScopedModule&) = delete;
    ScopedModule(ScopedModule&&) = delete;
    ScopedModule& operator=(ScopedModule&&) = delete;

  private:
    std::string prev_{};
  };

  class ScopedIndent {
  public:
    ScopedIndent();
    ~ScopedIndent();

    ScopedIndent(const ScopedIndent&) = delete;
    ScopedIndent& operator=(const ScopedIndent&) = delete;
    ScopedIndent(ScopedIndent&&) = delete;
    ScopedIndent& operator=(ScopedIndent&&) = delete;
  };
} // namespace ggems::core
