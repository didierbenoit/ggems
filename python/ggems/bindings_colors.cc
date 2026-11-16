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

#include <pybind11/pybind11.h>

#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"

namespace py = pybind11;

/*!
 * \brief Registers all normal palette keys without exposing RGB construction.
 * \param[in,out] module Python colors submodule receiving typed constants.
 */
auto BindColors(py::module_ &module) -> void {
  py::class_<ggems::render::ColorKey>(
    module, "ColorKey",
    "Immutable GGEMS palette key; use the named constants in ggems.colors.");

  for (auto const &entry : ggems::render::GetNamedColors()) {
    module.attr(entry.name.c_str()) = py::cast(entry.key);
  }
}
