# *****************************************************************************
# * This file is part of GGEMS.                                               *
# *                                                                           *
# * SPDX-License-Identifier: GPL-3.0-or-later                                 *
# * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
# * Inserm.                                                                   *
# *                                                                           *
# * GGEMS is free software: you can redistribute it and/or modify             *
# * it under the terms of the GNU General Public License as published by      *
# * the Free Software Foundation, either version 3 of the License, or         *
# * (at your option) any later version.                                       *
# *                                                                           *
# * GGEMS is distributed in the hope that it will be useful,                  *
# * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
# * GNU General Public License for more details.                              *
# *                                                                           *
# * You should have received a copy of the GNU General Public License         *
# * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
# *****************************************************************************

"""Define the GGEMS Source angle validation cases.

Authors:
    Julien BERT <julien.bert@univ-brest.fr>
    Didier BENOIT <didier.benoit@inserm.fr>
"""

from dataclasses import dataclass
from typing import Literal

type AngularConfiguration = Literal[
    "fixed", "isotropic", "bounded-isotropic", "focused"
]


@dataclass(frozen=True, slots=True)
class AngleCase:
    name: str
    angular: AngularConfiguration
    geometry: Literal["point", "rectangle"] = "point"
    dimensions_mm: tuple[float, float, float] = (0.0, 0.0, 0.0)
    # Theta min/max, then phi min/max. These are requested degrees, not packed bounds.
    bounds_deg: tuple[float, float, float, float] | None = None
    focus_mm: tuple[float, float, float] | None = None


CASES: tuple[AngleCase, ...] = (
    AngleCase("A1_fixed", "fixed"),
    AngleCase("A1_isotropic_full_sphere", "isotropic"),
    AngleCase(
        "A1_isotropic_bounded",
        "bounded-isotropic",
        bounds_deg=(20.0, 60.0, -45.0, 90.0),
    ),
    AngleCase(
        "A1_focused_rectangle",
        "focused",
        geometry="rectangle",
        dimensions_mm=(40.0, 20.0, 0.0),
        focus_mm=(0.0, 0.0, 100.0),
    ),
)

# Modest development defaults: the current Run still constructs its Observer dump.
DEFAULT_PRIMARIES = 4096
DEFAULT_WORKERS = 4096
DEFAULT_SEED = 77777
