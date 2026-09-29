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

"""Plot GGEMS Source geometry validation results.

Authors:
    Julien BERT <julien.bert@univ-brest.fr>
    Didier BENOIT <didier.benoit@inserm.fr>
"""

from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.axes import Axes
from numpy.typing import NDArray

type FloatArray = NDArray[np.float64]
type IntArray = NDArray[np.int64]


def _density(
    axis: Axes, horizontal: FloatArray, vertical: FloatArray, labels: tuple[str, str]
) -> None:
    # Automatic limits retain every measured point, including support excursions.
    _, _, _, mesh = axis.hist2d(horizontal, vertical, bins=64)
    axis.figure.colorbar(mesh, ax=axis, label="Source records / bin")
    axis.set(xlabel=labels[0], ylabel=labels[1])
    axis.set_aspect("equal", adjustable="box")


def _uniform_histogram(axis: Axes, variables: dict[str, FloatArray]) -> None:
    for name, values in variables.items():
        finite = values[np.isfinite(values)]
        if finite.size:
            axis.hist(finite, bins=50, density=True, histtype="step", label=name)

    axis.plot([0.0, 1.0], [1.0, 1.0], linestyle="--", label="Ideal Uniform[0,1]")
    axis.set(xlabel="Normalized coordinate", ylabel="Probability density")
    axis.legend()


def _ecdf(axis: Axes, variables: dict[str, FloatArray]) -> None:
    for name, values in variables.items():
        ordered = np.sort(values[np.isfinite(values)])
        if ordered.size:
            probability = (
                np.arange(1, ordered.size + 1, dtype=np.float64) / ordered.size
            )
            axis.step(ordered, probability, where="post", label=f"{name} empirical")

    axis.plot([0.0, 1.0], [0.0, 1.0], linestyle="--", label="Ideal Uniform[0,1]")
    axis.set(xlabel="Normalized coordinate", ylabel="Cumulative probability")
    axis.legend()


def _radial_histogram(axis: Axes, radius: FloatArray, dimension: int) -> None:
    axis.hist(radius, bins=50, density=True, histtype="step", label="GGEMS Source")
    reference = np.linspace(0.0, 1.0, 201)
    axis.plot(
        reference,
        dimension * reference ** (dimension - 1),
        linestyle="--",
        label=f"Ideal {dimension}D radial law",
    )
    axis.set(xlabel="r / R", ylabel="Probability density")
    axis.legend()


def plot_geometry(
    positions: IntArray,
    geometry: str,
    case_name: str,
    dimensions_pm: tuple[int, int, int],
    dimensions_mm: tuple[float, float, float],
    variables: dict[str, FloatArray],
    output_dir: Path,
) -> list[Path]:
    if geometry == "point":
        return []

    # The exporter obtains both units through GGEMS Units. No local unit scale.
    reference_axis = next(index for index, size in enumerate(dimensions_pm) if size > 0)
    mm_per_pm = dimensions_mm[reference_axis] / dimensions_pm[reference_axis]
    xyz_mm = positions.astype(np.float64) * mm_per_pm
    x, y, z = xyz_mm[:, 0], xyz_mm[:, 1], xyz_mm[:, 2]
    figure = plt.figure(figsize=(11, 8), layout="constrained")
    axes = [figure.add_subplot(2, 2, index + 1) for index in range(4)]
    _density(axes[0], x, y, ("X [mm]", "Y [mm]"))
    if geometry == "rectangle":
        _uniform_histogram(axes[1], variables)
        _ecdf(axes[2], {"u_x": variables["u_x"]})
        _ecdf(axes[3], {"u_y": variables["u_y"]})
    elif geometry in ("ellipse", "circle"):
        _uniform_histogram(axes[1], {"q": variables["q"]})
        _ecdf(axes[2], {"q": variables["q"]})
        _uniform_histogram(axes[3], {"u_phi": variables["u_phi"]})
    elif geometry == "box":
        _density(axes[1], x, z, ("X [mm]", "Z [mm]"))
        _density(axes[2], y, z, ("Y [mm]", "Z [mm]"))
        _uniform_histogram(axes[3], variables)
    elif geometry == "sphere":
        radius = np.cbrt(variables["q_r"])
        _radial_histogram(axes[1], radius, 3)
        _ecdf(axes[2], {"q_r": variables["q_r"]})
        _uniform_histogram(
            axes[3], {"u_cos": variables["u_cos"], "u_phi": variables["u_phi"]}
        )
    else:
        _uniform_histogram(axes[1], {"u_z": variables["u_z"]})
        _radial_histogram(axes[2], np.sqrt(variables["q_r"]), 2)
        _ecdf(axes[3], {"q_r": variables["q_r"]})

    figure.suptitle(
        f"{case_name}: canonical Source geometry, N = {positions.shape[0]:,}"
    )
    output_dir.mkdir(parents=True, exist_ok=True)
    paths = [output_dir / f"{geometry}.png", output_dir / f"{geometry}.pdf"]
    try:
        for path in paths:
            figure.savefig(path, dpi=180)
    finally:
        plt.close(figure)

    return paths
