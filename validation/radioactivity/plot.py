"""Create standalone descriptive figures from retained campaign evidence."""

import argparse
import math
from collections import defaultdict
from pathlib import Path
from typing import cast

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from radionuclide_validation.model import (
    JsonObject,
    Reference,
    Runtime,
    load_json,
    read_csv,
)
from radionuclide_validation.numerics import (
    conditioned_time_cdf,
)


def make_plots(directory: Path) -> None:
    settings = load_json(directory / "settings.json")
    result = load_json(directory / "analysis.json")
    reference = Reference.load(Path(str(settings["reference"])))
    runtime = Runtime.load(directory / "run")
    windows = cast(list[JsonObject], result["windows"])
    plots = directory / "plots"
    plots.mkdir(exist_ok=True)
    x = [float(str(window["start_half_lives"])) for window in windows]
    populations = [cast(JsonObject, window["total_population"]) for window in windows]
    observed = [cast(int, value["observed"]) for value in populations]
    expected = [float(str(value["expected"])) for value in populations]
    low = [cast(list[int], value["interval"])[0] for value in populations]
    high = [cast(list[int], value["interval"])[1] for value in populations]
    figure = plt.figure(figsize=(10, 7), layout="constrained")
    axis = figure.add_subplot(211)
    _ = axis.fill_between(
        x, low, high, color="#ccddec", label="Simultaneous Poisson prediction intervals"
    )
    _ = axis.plot(
        x, expected, color="#164b78", label="Independent integral × sum of yields"
    )
    _ = axis.plot(x, observed, ".", color="#bb4a32", label="GGEMS Source populations")
    _ = axis.set(
        ylabel="Primaries per window",
        title=f"{runtime.name}: ActivityDriven populations",
    )
    _ = axis.legend(fontsize="small")
    axis = figure.add_subplot(212)
    _ = axis.axhline(0, color="0.5", linewidth=1)
    _ = axis.plot(
        x,
        [(a - b) / math.sqrt(b) for a, b in zip(observed, expected)],
        "o-",
        color="#bb4a32",
    )
    _ = axis.set(
        xlabel="Window start / compiled half-life",
        ylabel="(Observed − expected) / √expected",
    )
    figure.savefig(plots / "decay_populations.png", dpi=180)
    plt.close(figure)

    # ----------------------------------------------------------------------------
    # Birth times and spectra use the same retained Source samples.

    energies: dict[int, list[int]] = defaultdict(list)
    uniforms: list[float] = []
    for row in read_csv(directory / "run/samples.csv"):
        window = windows[int(row["window"])]
        begin, end = cast(int, window["start_ps"]), cast(int, window["stop_ps"])
        scaled = float(str(window["scaled_decay_independent"]))
        uniforms.append(
            conditioned_time_cdf((int(row["time_ps"]) - begin) / (end - begin), scaled)
        )
        energies[int(row["group"])].append(int(row["energy_micro_eV"]))
    uniforms.sort()
    figure = plt.figure(figsize=(10, 7), layout="constrained")
    axis = figure.add_subplot(211)
    stride = max(1, len(uniforms) // 2000)
    _ = axis.plot(
        [0, 1], [0, 1], color="0.5", label="Conditioned exponential reference"
    )
    if uniforms:
        _ = axis.plot(
            uniforms[::stride],
            [(index + 1) / len(uniforms) for index in range(0, len(uniforms), stride)],
            color="#164b78",
            label="Pooled transformed birth-time ECDF",
        )
    _ = axis.set(
        xlabel="Conditional exponential CDF at birth",
        ylabel="Empirical CDF",
        title=f"{runtime.name}: birth-time shape",
    )
    _ = axis.legend(fontsize="small")
    axis = figure.add_subplot(212)
    populated = [
        (start, cast(JsonObject, window["birth_times"]))
        for start, window in zip(x, windows)
        if cast(JsonObject, window["birth_times"])["status"] != "insufficient_samples"
    ]
    starts = [start for start, _ in populated]
    _ = axis.plot(
        starts,
        [float(str(test["distance"])) for _, test in populated],
        "o-",
        label="Per-window ECDF distance",
    )
    _ = axis.plot(
        starts,
        [float(str(test["limit"])) for _, test in populated],
        "--",
        label="Preregistered DKW + numerical limit",
    )
    _ = axis.set(xlabel="Window start / compiled half-life", ylabel="CDF distance")
    _ = axis.legend(fontsize="small")
    figure.savefig(plots / "birth_times.png", dpi=180)
    plt.close(figure)

    for group in runtime.groups:
        figure = plt.figure(figsize=(10, 7), layout="constrained")
        axis = figure.add_subplot(211)
        data = energies[group.index]
        data.sort()
        if data:
            _ = axis.hist(
                [energy / runtime.energy_scale for energy in data],
                bins=80,
                density=True,
                histtype="step",
                color="#bb4a32",
                label=f"Generated primaries (n={len(data)})",
            )
        selected = (
            reference.groups[group.index]
            if group.index < len(reference.groups)
            else None
        )
        if selected is not None and selected.spectrum is not None:
            spectrum = selected.spectrum
            _ = axis.plot(
                spectrum.energy,
                [value / spectrum.area for value in spectrum.density],
                color="#164b78",
                label="Selected reference (conditional density)",
            )
        if group.kind == "RegularSpectrum":
            _ = axis.stairs(
                [
                    (b - a) * runtime.energy_scale / (group.ticket_space * group.width)
                    for a, b in zip((0,) + group.upper_tickets, group.upper_tickets)
                ],
                [
                    (group.lower_edge + index * group.width) / runtime.energy_scale
                    for index in range(len(group.energies) + 1)
                ],
                color="#478c59",
                linestyle="--",
                label="Compiled grid / finite-ticket bin masses",
            )
        _ = axis.set(
            xlabel="Energy (keV)",
            ylabel="Conditional density (1/keV)",
            title=f"{runtime.name}: group {group.index} {group.particle}",
        )
        _ = axis.legend(fontsize="small")
        axis = figure.add_subplot(212)
        if selected is not None and selected.spectrum is not None:
            spectrum = selected.spectrum
            stride = max(1, len(data) // 2000)
            if data:
                _ = axis.plot(
                    [value / runtime.energy_scale for value in data[::stride]],
                    [
                        (index + 1) / len(data)
                        - spectrum.cdf(data[index] / runtime.energy_scale)
                        for index in range(0, len(data), stride)
                    ],
                    color="#bb4a32",
                    label="Generated ECDF − selected reference CDF",
                )
            if group.kind == "RegularSpectrum":
                grid = [
                    (group.lower_edge + index * group.width) / runtime.energy_scale
                    for index in range(len(group.energies) + 1)
                ]
                _ = axis.plot(
                    grid,
                    [
                        group.continuous_cdf(value * runtime.energy_scale)
                        - spectrum.cdf(value)
                        for value in grid
                    ],
                    color="#478c59",
                    label="Compiled grid CDF − selected reference CDF",
                )
            _ = axis.legend(fontsize="small")
        _ = axis.axhline(0, color="0.5", linewidth=1)
        _ = axis.set(xlabel="Energy (keV)", ylabel="CDF difference")
        figure.savefig(plots / f"energy_group_{group.index}.png", dpi=180)
        plt.close(figure)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    _ = parser.add_argument(
        "campaign",
        type=Path,
        help="Campaign directory with analysis.json and retained samples; writes population, birth-time and energy PNG figures to plots/.",
    )
    args = cast(dict[str, Path], vars(parser.parse_args()))
    make_plots(args["campaign"])


if __name__ == "__main__":
    main()
