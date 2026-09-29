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

"""Plot GGEMS Source time validation results.

Authors:
    Julien BERT <julien.bert@univ-brest.fr>
    Didier BENOIT <didier.benoit@inserm.fr>
"""

from pathlib import Path

import matplotlib.pyplot as plt
from cases import TimeCase


def plot_chronology(
    case: TimeCase,
    windows_ps: tuple[tuple[int, int], ...],
    display_unit_ps: int,
    output: Path,
) -> list[str]:
    # Called only after exact snapshot, birth-time, and provenance validation.
    # One marker represents ALL births in a Run; this is not a time histogram.
    figure = plt.figure(figsize=(9, 4.5), layout="constrained")
    try:
        axis = figure.add_subplot(1, 1, 1)
        for index, (start_ps, stop_ps) in enumerate(windows_ps):
            start = start_ps / display_unit_ps
            stop = stop_ps / display_unit_ps
            axis.plot(
                [start, stop], [index, index], color="tab:blue", linewidth=5, alpha=0.5
            )
            axis.plot(
                [start],
                [index],
                "o",
                color="black",
                markersize=8,
                label="All Source births = window start" if index == 0 else None,
            )
            axis.plot(
                [stop],
                [index],
                "o",
                markerfacecolor="white",
                markeredgecolor="tab:blue",
                markersize=8,
                label="Excluded chronological stop" if index == 0 else None,
            )
            label = f"[{start:g}, {stop:g}) ns; births = {start:g} ns"
            if case.name == "T1_configured_windows" and index == len(windows_ps) - 1:
                label += " (shortened final window)"
            if case.reset_before_run == index:
                label += " (after ResetTime)"
            axis.text(
                0.02,
                index - 0.16,
                label,
                fontsize=10,
                transform=axis.get_yaxis_transform(),
            )

        axis.set_yticks(
            list(range(len(windows_ps))),
            [f"Run sequence {i}" for i in range(len(windows_ps))],
        )
        axis.set_xlabel("Run chronology [ns]")
        axis.set_title(
            f"{case.name}\nCountDriven birth times follow the committed window start exactly"
        )
        axis.set_xlim(0, max(stop for _, stop in windows_ps) / display_unit_ps + 5)
        axis.set_ylim(len(windows_ps) - 0.5, -0.6)
        axis.grid(axis="x", alpha=0.25)
        axis.legend(
            loc="lower center", bbox_to_anchor=(0.5, -0.32), frameon=False, ncols=2
        )

        paths: list[str] = []
        for extension in ("png", "pdf"):
            path = output / f"chronology.{extension}"
            figure.savefig(path, dpi=180)
            paths.append(str(path.resolve()))

        return paths
    finally:
        plt.close(figure)
