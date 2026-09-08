from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.axes import Axes
from numpy.typing import NDArray

from cases import AngleCase  # pyright: ignore[reportImplicitRelativeImport]

type FloatArray = NDArray[np.float64]
type IntArray = NDArray[np.int64]


def _uniform_ecdf(axis: Axes, values: FloatArray, name: str) -> None:
    ordered = np.sort(values)
    if ordered.size:
        probability = np.arange(1, ordered.size + 1, dtype=np.float64) / ordered.size
        _ = axis.step(ordered, probability, where="post", label=f"{name} empirical")
    _ = axis.plot([0.0, 1.0], [0.0, 1.0], linestyle="--", label="Ideal Uniform[0,1]")
    _ = axis.set(xlabel=name, ylabel="Cumulative probability")
    _ = axis.legend()


def plot_angle(
    case: AngleCase,
    positions: IntArray,
    dimensions_pm: tuple[int, int, int],
    variables: dict[str, FloatArray],
    exclusions: dict[str, object],
    output_dir: Path,
) -> list[Path]:
    if case.angular == "fixed":
        return []

    count = positions.shape[0]
    bins = max(4, min(40, int(np.sqrt(count))))
    figure = plt.figure(
        figsize=(12, 4) if case.angular == "focused" else (10, 8), layout="constrained"
    )

    try:
        if case.angular == "focused":
            axes = [figure.add_subplot(1, 3, index + 1) for index in range(3)]
            # Both units originate in GGEMS metadata/configuration; no local unit table.
            mm_per_pm = case.dimensions_mm[0] / dimensions_pm[0]
            xy_mm = positions[:, :2].astype(np.float64) * mm_per_pm
            _, _, _, mesh = axes[0].hist2d(xy_mm[:, 0], xy_mm[:, 1], bins=bins)
            _ = figure.colorbar(mesh, ax=axes[0], label="Source records / bin")
            _ = axes[0].set(xlabel="Source X [mm]", ylabel="Source Y [mm]")
            axes[0].set_aspect("equal", adjustable="box")

            _ = axes[1].hist(
                variables["miss_distance_pm"], bins=bins, histtype="step"
            )
            _ = axes[1].set(
                xlabel="Focus miss distance [pm]", ylabel="Source records / bin"
            )
            ordered = np.sort(variables["angular_error_rad"])
            probability = (
                np.arange(1, ordered.size + 1, dtype=np.float64) / ordered.size
            )
            _ = axes[2].step(ordered, probability, where="post")
            _ = axes[2].set(
                xlabel="Line-of-sight angular error [rad]", ylabel="Empirical CDF"
            )
            title = f"{case.name}: 40 x 20 mm Rectangle to global focus (0, 0, 100) mm; N={count:,}"
        else:
            axes = [figure.add_subplot(2, 2, index + 1) for index in range(4)]
            u_cos, u_phi = variables["u_cos"], variables["u_phi"]
            # Automatic limits retain excursions; bins have equal solid angle.
            if u_phi.size:
                _, _, _, mesh = axes[0].hist2d(
                    u_phi, variables["u_cos_for_phi"], bins=bins
                )
                _ = figure.colorbar(mesh, ax=axes[0], label="Source records / bin")
            else:
                _ = axes[0].text(0.5, 0.5, "No defined azimuth", ha="center")
            _ = axes[0].set(
                xlabel="u_phi", ylabel="u_cos", title="Equal-solid-angle coordinates"
            )
            axes[0].set_aspect("equal", adjustable="box")

            for name, values in (("u_cos", u_cos), ("u_phi", u_phi)):
                if values.size:
                    _ = axes[1].hist(
                        values, bins=bins, density=True, histtype="step", label=name
                    )
            _ = axes[1].plot(
                [0.0, 1.0], [1.0, 1.0], linestyle="--", label="Ideal Uniform[0,1]"
            )
            _ = axes[1].set(
                xlabel="Transformed angular variable", ylabel="Probability density"
            )
            _ = axes[1].legend()
            _uniform_ecdf(axes[2], u_cos, "u_cos")
            _uniform_ecdf(axes[3], u_phi, "u_phi")

            domain = "Canonical full sphere"
            if case.bounds_deg is not None:
                theta_min, theta_max, phi_min, phi_max = case.bounds_deg
                domain = f"Requested theta {theta_min:g}..{theta_max:g} deg, phi {phi_min:g}..{phi_max:g} deg"
            title = f"{case.name}: {domain}\nN={count:,}; undefined-phi exclusions={exclusions['phi_exclusion_count']}"

        _ = figure.suptitle(title)
        output_dir.mkdir(parents=True, exist_ok=True)
        paths = [output_dir / "angle.png", output_dir / "angle.pdf"]
        for path in paths:
            figure.savefig(path, dpi=180)
    finally:
        plt.close(figure)

    return paths
