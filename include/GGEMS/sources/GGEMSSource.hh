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

/*#include <pybind11/pybind11.h>

#include <string>

class Animal {
public:
  virtual ~Animal() {}
  virtual std::string Go(int const& n_times) = 0;
};

class PyAnimal : public Animal, public pybind11::trampoline_self_life_support {
public:
  using Animal::Animal;
  std::string Go(int const& n_times) override {
    PYBIND11_OVERRIDE_PURE(*/
  //    std::string, /* Return type */
  //    Animal,      /* Parent class */
  //    Go,          /* Name of function in C++ (must match Python name) */
  //    n_times      /* Argument(s) */
/*    );
  }
};

class Dog : public Animal {
public:
  std::string Go(int const& n_times) override;
};
*/
