from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from numpy.typing import NDArray

type FloatArray = NDArray[np.float64]


def plot_integrated(
    case_name: str,
    local_mm: FloatArray,
    common: dict[str, FloatArray],
    energies_kev: FloatArray,
    edges_kev: FloatArray,
    expected_density: FloatArray,
    correlation_matrix: FloatArray,
    output_dir: Path,
) -> list[str]:
    figure = plt.figure(figsize=(10, 8), layout="constrained")
    try:
        position = figure.add_subplot(2, 2, 1)
        _ = position.hist2d(local_mm[:, 0], local_mm[:, 1], bins=40)
        _ = position.set(
            xlabel="Recovered local X (mm)",
            ylabel="Recovered local Y (mm)",
            title="Rectangle position",
            aspect="equal",
        )

        angular = figure.add_subplot(2, 2, 2)
        _ = angular.hist2d(common["u_phi"], common["u_cos"], bins=40)
        _ = angular.set(
            xlabel=r"Recovered $u_\phi$",
            ylabel=r"Recovered $u_{\cos\theta}$",
            title="Equal-solid-angle coordinates",
            aspect="equal",
        )

        energy = figure.add_subplot(2, 2, 3)
        _ = energy.hist(
            energies_kev,
            bins=[
                float(edge)
                for edge in np.linspace(float(edges_kev[0]), float(edges_kev[-1]), 81)
            ],
            density=True,
            histtype="step",
            label="GGEMS",
        )
        _ = energy.stairs(expected_density, edges_kev, label="Exact bin mass / width")
        _ = energy.set(
            xlabel="Energy (keV)",
            ylabel="Density (1/keV)",
            title="RegularSpectrum (finite CDF in JSON)",
        )
        _ = energy.legend()

        correlation = figure.add_subplot(2, 2, 4)
        names = list(common)
        # Undefined correlations stay missing; no fabricated zero or corrcoef
        # division warning for a caller-selected one-primary smoke.
        shown = correlation.imshow(
            np.ma.masked_invalid(correlation_matrix), vmin=-1.0, vmax=1.0
        )
        _ = correlation.set(
            xticks=range(4),
            yticks=range(4),
            xticklabels=names,
            yticklabels=names,
            title="Pearson correlation (descriptive)",
        )
        for row in range(4):
            for column in range(4):
                value = correlation_matrix[row, column]
                label = f"{value:.3f}" if np.isfinite(value) else "undefined"
                _ = correlation.text(
                    column, row, label, ha="center", va="center", fontsize=9
                )
        _ = figure.colorbar(shown, ax=correlation, shrink=0.8)
        _ = figure.suptitle(f"{case_name} — N={energies_kev.size}, Philox")

        paths: list[str] = []
        for extension in ("png", "pdf"):
            path = output_dir / f"integration.{extension}"
            figure.savefig(path, dpi=180)
            paths.append(str(path.resolve()))
        return paths
    finally:
        plt.close(figure)
