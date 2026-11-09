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
 * \brief Publishes the normal GGEMS palette as typed Python color constants.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cctype>
#include <string>

#include <pybind11/pybind11.h>

#include "GGEMS/render/GGEMSColorBlue.hh"
#include "GGEMS/render/GGEMSColorCyan.hh"
#include "GGEMS/render/GGEMSColorGray.hh"
#include "GGEMS/render/GGEMSColorGreen.hh"
#include "GGEMS/render/GGEMSColorMagenta.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorOrange.hh"
#include "GGEMS/render/GGEMSColorRed.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSColorWhite.hh"
#include "GGEMS/render/GGEMSColorYellow.hh"

namespace py = pybind11;

/*!
 * \brief Registers all normal palette keys without exposing RGB construction.
 * \param[in,out] module Python colors submodule receiving typed constants.
 */
auto BindColors(py::module_ &module) -> void {
  py::class_<ggems::render::ColorKey>(
    module, "ColorKey",
    "Immutable GGEMS palette key; use the named constants in ggems.colors.");

  auto const publish = [&module](std::string name,
                                 ggems::render::ColorKey color) -> void {
    for (auto &character : name) {
      character =
        static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }

    module.attr(name.c_str()) = py::cast(color);
  };

  /// \cond PythonPaletteMacros
#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("gray_" #NAME, ggems::render::GRAY_##NAME);
  GGEMS_COLOR_GRAY_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("red_" #NAME, ggems::render::RED_##NAME);
  GGEMS_COLOR_RED_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("orange_" #NAME, ggems::render::ORANGE_##NAME);
  GGEMS_COLOR_ORANGE_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("yellow_" #NAME, ggems::render::YELLOW_##NAME);
  GGEMS_COLOR_YELLOW_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("green_" #NAME, ggems::render::GREEN_##NAME);
  GGEMS_COLOR_GREEN_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("cyan_" #NAME, ggems::render::CYAN_##NAME);
  GGEMS_COLOR_CYAN_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("blue_" #NAME, ggems::render::BLUE_##NAME);
  GGEMS_COLOR_BLUE_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("magenta_" #NAME, ggems::render::MAGENTA_##NAME);
  GGEMS_COLOR_MAGENTA_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR

#define GGEMS_PYTHON_COLOR(NAME, R, G, B)                                      \
  publish("white_" #NAME, ggems::render::WHITE_##NAME);
  GGEMS_COLOR_WHITE_SHADES(GGEMS_PYTHON_COLOR)
#undef GGEMS_PYTHON_COLOR
  /// \endcond
}
