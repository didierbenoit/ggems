"""Create standalone descriptive figures from retained campaign evidence."""

import argparse
import math
from collections import defaultdict
from collections.abc import Sequence
from pathlib import Path
from typing import Protocol, cast

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

from validation.radioactivity.radionuclide_validation.model import (
    Reference,
    Runtime,
    array_value,
    decimal_value,
    integer_value,
    load_json,
    object_value,
    read_csv,
    string_value,
)
from validation.radioactivity.radionuclide_validation.numerics import (
    conditioned_time_cdf,
)


class Arguments(Protocol):
    campaign: Path


# Matplotlib's keyword-heavy stubs leave Unknown types on these public methods.
# This boundary describes the small plotting interface actually used here.
class PlotAxes(Protocol):
    def plot(
        self, x: Sequence[float], y: Sequence[float], fmt: str = ..., **style: object
    ) -> object: ...
    def fill_between(
        self,
        x: Sequence[float],
        lower: Sequence[float],
        upper: Sequence[float],
        **style: object,
    ) -> object: ...
    def set(self, **labels: str) -> object: ...
    def legend(self, **style: object) -> object: ...
    def axhline(self, y: float, **style: object) -> object: ...
    def hist(self, values: Sequence[float], **style: object) -> object: ...
    def stairs(
        self, values: Sequence[float], edges: Sequence[float], **style: object
    ) -> object: ...


class PlotFigure(Protocol):
    def add_subplot(self, position: int) -> PlotAxes: ...
    def savefig(self, filename: Path, *, dpi: int) -> None: ...


class Plotting(Protocol):
    def figure(self, *, figsize: tuple[int, int], layout: str) -> PlotFigure: ...
    def close(self, figure: PlotFigure) -> None: ...


def plotting_api(module: object) -> Plotting:
    return cast(Plotting, module)


PLOT = plotting_api(plt)


def make_plots(directory: Path) -> None:
    settings = load_json(directory / "settings.json")
    result = load_json(directory / "analysis.json")
    reference = Reference.load(Path(string_value(settings["reference"])))
    runtime = Runtime.load(directory / "run")
    windows = [object_value(value) for value in array_value(result["windows"])]
    plots = directory / "plots"
    plots.mkdir(exist_ok=True)
    x = [float(decimal_value(window["start_half_lives"])) for window in windows]
    populations = [object_value(window["total_population"]) for window in windows]
    observed = [integer_value(value["observed"]) for value in populations]
    expected = [float(decimal_value(value["expected"])) for value in populations]
    low = [integer_value(array_value(value["interval"])[0]) for value in populations]
    high = [integer_value(array_value(value["interval"])[1]) for value in populations]
    figure = PLOT.figure(figsize=(10, 7), layout="constrained")
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
    PLOT.close(figure)

    rows = read_csv(
        directory / "run/samples.csv",
        (
            "window",
            "group",
            "run_id",
            "global_primary_id",
            "source_local_primary_id",
            "particle_type",
            "time_ps",
            "energy_micro_eV",
        ),
    )
    energies: dict[int, list[int]] = defaultdict(list)
    uniforms: list[float] = []
    for row in rows:
        window = windows[int(row["window"])]
        begin, end = integer_value(window["start_ps"]), integer_value(window["stop_ps"])
        scaled = float(decimal_value(window["scaled_decay_independent"]))
        uniforms.append(
            conditioned_time_cdf((int(row["time_ps"]) - begin) / (end - begin), scaled)
        )
        energies[int(row["group"])].append(int(row["energy_micro_eV"]))
    uniforms.sort()
    figure = PLOT.figure(figsize=(10, 7), layout="constrained")
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
    tests = [object_value(window["birth_times"]) for window in windows]
    _ = axis.plot(
        x,
        [float(decimal_value(test.get("distance", 0))) for test in tests],
        "o-",
        label="Per-window ECDF distance",
    )
    _ = axis.plot(
        x,
        [float(decimal_value(test.get("limit", 1))) for test in tests],
        "--",
        label="Preregistered DKW + numerical limit",
    )
    _ = axis.set(xlabel="Window start / compiled half-life", ylabel="CDF distance")
    _ = axis.legend(fontsize="small")
    figure.savefig(plots / "birth_times.png", dpi=180)
    PLOT.close(figure)

    for group in runtime.groups:
        figure = PLOT.figure(figsize=(10, 7), layout="constrained")
        axis = figure.add_subplot(211)
        data = sorted(energies[group.index])
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
        PLOT.close(figure)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    _ = parser.add_argument("campaign", type=Path)
    args = cast(Arguments, cast(object, parser.parse_args()))
    make_plots(args.campaign)


if __name__ == "__main__":
    main()
