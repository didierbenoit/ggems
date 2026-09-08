from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from cases import FrameCase  # pyright: ignore[reportImplicitRelativeImport]
from matplotlib.axes import Axes
from numpy.typing import NDArray

type FloatArray = NDArray[np.float64]
type IntArray = NDArray[np.int64]


def density(axis: Axes, x: FloatArray, y: FloatArray, xlabel: str, ylabel: str) -> None:
    if x.size:
        _ = axis.hist2d(x, y, bins=40)
    else:
        _ = axis.text(
            0.5, 0.5, "No defined azimuths", ha="center", transform=axis.transAxes
        )
    _ = axis.set(xlabel=xlabel, ylabel=ylabel)
    axis.set_aspect("equal", adjustable="box")


def empirical_cdf(axis: Axes, values: FloatArray, label: str) -> None:
    if not values.size:
        return
    distinct, counts = np.unique(values, return_counts=True)
    probability = np.cumsum(counts, dtype=np.float64) / values.size
    _ = axis.step(
        np.concatenate((distinct[:1], distinct)),
        np.concatenate((np.array([0.0]), probability)),
        where="post",
        label=label,
    )


def uniform_panels(
    density_axis: Axes, cdf_axis: Axes, variables: dict[str, FloatArray]
) -> None:
    for name, values in variables.items():
        if values.size:
            # Automatic data bounds include excursions; do not limit the data
            # range to [0,1] or renormalize after discarding observations.
            _ = density_axis.hist(
                values, bins=30, density=True, histtype="step", label=name
            )
            empirical_cdf(cdf_axis, values, name)
    _ = density_axis.plot([0.0, 1.0], [1.0, 1.0], "--", label="Ideal U(0,1)")
    _ = cdf_axis.plot([0.0, 1.0], [0.0, 1.0], "--", label="Ideal U(0,1)")
    _ = density_axis.set(
        xlabel="Recovered local uniform coordinate", ylabel="Probability density"
    )
    _ = cdf_axis.set(
        xlabel="Recovered local uniform coordinate", ylabel="Cumulative probability"
    )
    _ = density_axis.legend()
    _ = cdf_axis.legend()


def plot_frame(
    case: FrameCase,
    positions: IntArray,
    local: FloatArray,
    variables: dict[str, FloatArray],
    display_unit_pm: int,
    output: Path,
) -> list[str]:
    # Only the three descriptive oblique cases need figures. Exact cases are
    # assessed by provenance-paired JSON comparisons.
    if not case.statistical:
        return []
    geometry_only = case.name == "G2_rectangle_oblique"
    figure = plt.figure(
        figsize=(13, 8) if geometry_only else (13, 4.5), layout="constrained"
    )
    try:
        axes = [
            figure.add_subplot(2 if geometry_only else 1, 3, index + 1)
            for index in range(6 if geometry_only else 3)
        ]
        local_mm = local / display_unit_pm
        if geometry_only:
            global_mm = positions.astype(np.float64) / display_unit_pm
            for axis, (left, right) in zip(
                axes[:3], ((0, 1), (0, 2), (1, 2)), strict=True
            ):
                density(
                    axis,
                    global_mm[:, left],
                    global_mm[:, right],
                    f"Global {'XYZ'[left]} [mm]",
                    f"Global {'XYZ'[right]} [mm]",
                )
            density(
                axes[3],
                local_mm[:, 0],
                local_mm[:, 1],
                "Recovered local X [mm]",
                "Recovered local Y [mm]",
            )
            uniform_panels(
                axes[4], axes[5], {"u_x": variables["u_x"], "u_y": variables["u_y"]}
            )
        elif case.name == "A2_bounded_oblique":
            density(
                axes[0],
                variables["u_phi"],
                variables["u_cos_for_phi"],
                "Recovered local u_phi",
                "Recovered local u_cos",
            )
            _ = axes[0].set_title("Equal solid angle coordinates")
            uniform_panels(
                axes[1],
                axes[2],
                {"u_cos": variables["u_cos"], "u_phi": variables["u_phi"]},
            )
        else:
            density(
                axes[0],
                local_mm[:, 0],
                local_mm[:, 1],
                "Recovered local X [mm]",
                "Recovered local Y [mm]",
            )
            _ = axes[1].hist(variables["miss_distance_pm"], bins=30, histtype="step")
            _ = axes[1].set(
                xlabel="Global focus miss distance [pm]", ylabel="Source primary count"
            )
            empirical_cdf(axes[2], variables["angular_error_rad"], "Observed error")
            _ = axes[2].set(
                xlabel="Direction angular error [rad]",
                ylabel="Empirical cumulative probability",
            )
        _ = figure.suptitle(
            f"{case.name}: N={positions.shape[0]:,}; actual packed frame; no statistical threshold"
        )
        paths: list[str] = []
        for extension in ("png", "pdf"):
            path = output / f"frame.{extension}"
            figure.savefig(path, dpi=180)
            paths.append(str(path.resolve()))
        return paths
    finally:
        plt.close(figure)
