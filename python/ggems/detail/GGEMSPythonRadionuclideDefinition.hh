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
 * \brief Provides an internal Python handle for immutable GGEMS radionuclide
 * definitions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <memory>
#include <utility>

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::python::detail {

// =============================================================================
// =============================================================================

class GGEMSRadionuclideDefinitionHandle {
public:
  explicit GGEMSRadionuclideDefinitionHandle(
    std::shared_ptr<core::radioactivity::GGEMSRadionuclideDefinition const>
      definition) noexcept
      : definition_{std::move(definition)} {}

  [[nodiscard]] auto GetDefinition() const noexcept
    -> std::shared_ptr<core::radioactivity::GGEMSRadionuclideDefinition const> {
    return definition_;
  }

private:
  std::shared_ptr<core::radioactivity::GGEMSRadionuclideDefinition const>
    definition_;
};

} // namespace ggems::python::detail
