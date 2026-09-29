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

"""Define the GGEMS Source energy validation cases.

Authors:
    Julien BERT <julien.bert@univ-brest.fr>
    Didier BENOIT <didier.benoit@inserm.fr>
"""

from dataclasses import dataclass
from typing import Literal

type EnergyMode = Literal["mono", "discrete-lines", "regular-spectrum"]


@dataclass(frozen=True, slots=True)
class EnergyCase:
    name: str
    mode: EnergyMode
    values_kev: tuple[int, ...]
    # Explicit canonical E1 fixtures, checked against the executed metadata.
    # These are not a replacement Units converter.
    energies_micro_ev: tuple[int, ...]
    weights: tuple[int, ...] = ()
    bin_width_kev: int | None = None
    bin_width_micro_ev: int = 0


CASES: tuple[EnergyCase, ...] = (
    EnergyCase("E1_mono", "mono", (511,), (511_000_000_000,)),
    EnergyCase(
        "E1_discrete_lines",
        "discrete-lines",
        (20, 40, 60, 80),
        (20_000_000_000, 40_000_000_000, 60_000_000_000, 80_000_000_000),
        (1, 0, 1, 2),
    ),
    EnergyCase(
        "E1_regular_spectrum",
        "regular-spectrum",
        (25, 35, 45, 55),
        (25_000_000_000, 35_000_000_000, 45_000_000_000, 55_000_000_000),
        (1, 1, 2, 4),
        bin_width_kev=10,
        bin_width_micro_ev=10_000_000_000,
    ),
)

# Development inspection only; the current Run still builds its Observer dump.
DEFAULT_PRIMARIES = 4096
DEFAULT_WORKERS = 4096
DEFAULT_SEED = 77777
