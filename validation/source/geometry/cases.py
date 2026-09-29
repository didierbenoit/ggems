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

"""Define the GGEMS Source geometry validation cases.

Authors:
    Julien BERT <julien.bert@univ-brest.fr>
    Didier BENOIT <didier.benoit@inserm.fr>
"""

from dataclasses import dataclass
from typing import Literal

type Geometry = Literal[
    "point", "rectangle", "ellipse", "circle", "box", "sphere", "cylinder"
]


@dataclass(frozen=True, slots=True)
class GeometryCase:
    geometry: Geometry
    dimensions_mm: tuple[float, float, float]

    @property
    def name(self) -> str:
        return f"G1_{self.geometry}"


# Complete widths/diameters/heights; these are development configurations.
CASES: tuple[GeometryCase, ...] = (
    GeometryCase("point", (0.0, 0.0, 0.0)),
    GeometryCase("rectangle", (40.0, 20.0, 0.0)),
    GeometryCase("ellipse", (40.0, 20.0, 0.0)),
    GeometryCase("circle", (30.0, 30.0, 0.0)),
    GeometryCase("box", (40.0, 20.0, 10.0)),
    GeometryCase("sphere", (30.0, 30.0, 30.0)),
    GeometryCase("cylinder", (30.0, 30.0, 40.0)),
)

# The current Run constructs an Observer dump even when no sink prints it.
# Keep first-use capture modest; article sample sizes remain undecided.
DEFAULT_PRIMARIES = 4096
DEFAULT_WORKERS = 4096
DEFAULT_SEED = 77777
