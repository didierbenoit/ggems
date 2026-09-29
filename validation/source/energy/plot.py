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

"""Plot GGEMS Source energy validation results.

Authors:
    Julien BERT <julien.bert@univ-brest.fr>
    Didier BENOIT <didier.benoit@inserm.fr>
"""

from pathlib import Path

import matplotlib.pyplot as plt
from cases import EnergyCase


def plot_energy(
    case: EnergyCase,
    energies: list[int],
    centers: tuple[int, ...],
    width: int,
    display_unit_micro_ev: int,
    rows: list[dict[str, object]],
    finite: list[dict[str, object]],
    output_dir: Path,
) -> list[Path]:
    if case.mode == "mono":
        return []

    # Exact membership checks precede all display conversions. The unit scale
    # comes from central GGEMS Units in the executed metadata.
    display_centers = [value / display_unit_micro_ev for value in centers]
    expected = [row["expected_probability"] for row in rows]
    observed = [row["observed_probability"] for row in rows]
    residuals = [row["probability_residual"] for row in rows]
    index = list(range(len(centers)))
    regular = case.mode == "regular-spectrum"
    figure = plt.figure(figsize=(10, 8) if regular else (10, 4), layout="constrained")

    try:
        axes = [
            figure.add_subplot(2 if regular else 1, 2, i + 1)
            for i in range(4 if regular else 2)
        ]
        probability_axis = axes[1] if regular else axes[0]
        residual_axis = axes[2] if regular else axes[1]

        probability_axis.bar(
            [i - 0.18 for i in index],
            expected,
            width=0.36,
            label="Exact ticket probability",
        )
        probability_axis.bar(
            [i + 0.18 for i in index], observed, width=0.36, label="GGEMS Source"
        )
        probability_axis.set_xticks(index, [f"{value:g}" for value in display_centers])
        probability_axis.set(
            xlabel="Bin center [keV]" if regular else "Configured line [keV]",
            ylabel="Probability",
        )
        probability_axis.legend()

        residual_axis.bar(index, residuals, width=0.6)
        residual_axis.axhline(0.0, color="black", linewidth=0.8)
        residual_axis.set_xticks(index, [f"{value:g}" for value in display_centers])
        residual_axis.set(
            xlabel="Bin center [keV]" if regular else "Configured line [keV]",
            ylabel="Observed - expected probability",
        )

        if regular:
            edges = [
                centers[0] - width // 2,
                *(center + width // 2 for center in centers),
            ]
            display_edges = [value / display_unit_micro_ev for value in edges]
            # Fine histogram for inspection; the dashed reference expresses
            # exact bin masses as constant density per keV. Integer sub-bin
            # structure is measured separately against the exact finite law.
            histogram_edges = [
                (edges[0] + step * width // 10) / display_unit_micro_ev
                for step in range(10 * len(centers) + 1)
            ]
            axes[0].hist(
                [value / display_unit_micro_ev for value in energies],
                bins=histogram_edges,
                density=True,
                histtype="step",
                label="GGEMS Source",
            )
            axes[0].stairs(
                [
                    probability / (width / display_unit_micro_ev)
                    for probability in expected
                ],
                display_edges,
                linestyle="--",
                label="Exact bin mass / bin width",
            )
            axes[0].set(
                xlabel="Emitted energy [keV]",
                ylabel="Probability density [1/keV]",
                title="Center-defined, piecewise-constant spectrum",
            )
            axes[0].legend()

            deviations = [row["exact_finite_cdf_max_deviation"] for row in finite]
            for i, deviation in enumerate(deviations):
                if deviation is None:
                    axes[3].text(i, 0.0, "No samples", ha="center", va="bottom")
                else:
                    axes[3].bar(i, deviation, width=0.6)
            axes[3].set_xticks(
                index,
                [
                    f"{value:g}\nn={row['sample_count']}"
                    for value, row in zip(display_centers, finite, strict=True)
                ],
            )
            axes[3].set(
                xlabel="Bin center [keV]",
                ylabel="Maximum CDF deviation",
                title="Conditional exact finite ticket law",
            )

        figure.suptitle(
            f"{case.name}: N={len(energies):,}; Philox; no statistical acceptance threshold"
        )
        output_dir.mkdir(parents=True, exist_ok=True)
        paths = [output_dir / "energy.png", output_dir / "energy.pdf"]
        for path in paths:
            figure.savefig(path, dpi=180)
    finally:
        plt.close(figure)

    return paths
