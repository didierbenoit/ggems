from pathlib import Path
from typing import cast

import matplotlib.pyplot as plt
from cases import EnergyCase  # pyright: ignore[reportImplicitRelativeImport]


def plot_energy(
    case: EnergyCase,
    energies: list[int],
    centers: tuple[int, ...],
    width: int,
    display_unit_mev: int,
    rows: list[dict[str, object]],
    finite: list[dict[str, object]],
    output_dir: Path,
) -> list[Path]:
    if case.mode == "mono":
        return []

    # Exact membership checks precede all display conversions. The unit scale
    # comes from central GGEMS Units in the executed metadata.
    display_centers = [value / display_unit_mev for value in centers]
    expected = [cast(float, row["expected_probability"]) for row in rows]
    observed = [cast(float, row["observed_probability"]) for row in rows]
    residuals = [cast(float, row["probability_residual"]) for row in rows]
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

        _ = probability_axis.bar(
            [i - 0.18 for i in index],
            expected,
            width=0.36,
            label="Exact ticket probability",
        )
        _ = probability_axis.bar(
            [i + 0.18 for i in index], observed, width=0.36, label="GGEMS Source"
        )
        _ = probability_axis.set_xticks(
            index, [f"{value:g}" for value in display_centers]
        )
        _ = probability_axis.set(
            xlabel="Bin center [keV]" if regular else "Configured line [keV]",
            ylabel="Probability",
        )
        _ = probability_axis.legend()

        _ = residual_axis.bar(index, residuals, width=0.6)
        _ = residual_axis.axhline(0.0, color="black", linewidth=0.8)
        _ = residual_axis.set_xticks(index, [f"{value:g}" for value in display_centers])
        _ = residual_axis.set(
            xlabel="Bin center [keV]" if regular else "Configured line [keV]",
            ylabel="Observed - expected probability",
        )

        if regular:
            edges = [
                centers[0] - width // 2,
                *(center + width // 2 for center in centers),
            ]
            display_edges = [value / display_unit_mev for value in edges]
            # Fine histogram for inspection; the dashed reference expresses
            # exact bin masses as constant density per keV. Integer sub-bin
            # structure is measured separately against the exact finite law.
            histogram_edges = [
                (edges[0] + step * width // 10) / display_unit_mev
                for step in range(10 * len(centers) + 1)
            ]
            _ = axes[0].hist(
                [value / display_unit_mev for value in energies],
                bins=histogram_edges,
                density=True,
                histtype="step",
                label="GGEMS Source",
            )
            _ = axes[0].stairs(
                [probability / (width / display_unit_mev) for probability in expected],
                display_edges,
                linestyle="--",
                label="Exact bin mass / bin width",
            )
            _ = axes[0].set(
                xlabel="Emitted energy [keV]",
                ylabel="Probability density [1/keV]",
                title="Center-defined, piecewise-constant spectrum",
            )
            _ = axes[0].legend()

            deviations = [
                cast(float | None, row["exact_finite_cdf_max_deviation"])
                for row in finite
            ]
            for i, deviation in enumerate(deviations):
                if deviation is None:
                    _ = axes[3].text(i, 0.0, "No samples", ha="center", va="bottom")
                else:
                    _ = axes[3].bar(i, deviation, width=0.6)
            _ = axes[3].set_xticks(
                index,
                [
                    f"{value:g}\nn={row['sample_count']}"
                    for value, row in zip(display_centers, finite, strict=True)
                ],
            )
            _ = axes[3].set(
                xlabel="Bin center [keV]",
                ylabel="Maximum CDF deviation",
                title="Conditional exact finite ticket law",
            )

        _ = figure.suptitle(
            f"{case.name}: N={len(energies):,}; Philox; no statistical acceptance threshold"
        )
        output_dir.mkdir(parents=True, exist_ok=True)
        paths = [output_dir / "energy.png", output_dir / "energy.pdf"]
        for path in paths:
            figure.savefig(path, dpi=180)
    finally:
        plt.close(figure)

    return paths
