"""Plot populations, birth times, and emission spectra by group and particle type."""

import argparse
import math
from collections import Counter, defaultdict
from decimal import Decimal
from itertools import pairwise
from pathlib import Path
from typing import cast

import matplotlib
import numpy as np

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from radionuclide_validation.model import (
    JsonObject,
    Reference,
    ReferenceGroup,
    Runtime,
    RuntimeGroup,
    load_json,
    read_csv,
)
from radionuclide_validation.numerics import (
    conditioned_time_cdf,
)


def plot_lines(
    axis: Axes,
    groups: list[RuntimeGroup],
    selected: list[ReferenceGroup],
    data: list[int],
    parent_decays: float,
    scale: int,
) -> None:
    compiled: dict[int, float] = defaultdict(float)
    for group in groups:
        emission_yield = float(group.yield_per_decay)
        if group.kind == "Mono":
            compiled[group.mono] += emission_yield
        else:
            for energy, lower, upper in zip(
                group.energies, (0,) + group.upper_tickets, group.upper_tickets
            ):
                compiled[energy] += (
                    emission_yield * (upper - lower) / group.ticket_space
                )

    reference: dict[int, float] = defaultdict(float)
    for group in selected:
        if group.kind == "Mono":
            quantity = cast(JsonObject, group.distribution["energy"])
            energy = int(Decimal(str(quantity["value"])) * scale)
            reference[energy] += float(group.yield_per_decay)
        elif group.kind == "DiscreteLines":
            lines = cast(list[JsonObject], group.distribution["lines"])
            weights = [Decimal(str(line["weight"])) for line in lines]
            total = sum(weights, Decimal(0))
            for line, weight in zip(lines, weights):
                energy = int(Decimal(str(line["energy_keV"])) * scale)
                reference[energy] += float(group.yield_per_decay * weight / total)

    # Exact same-energy lines are combined before conversion to display keV.
    compiled_lines = sorted((e / scale, y) for e, y in compiled.items() if y > 0)
    reference_lines = sorted((e / scale, y) for e, y in reference.items() if y > 0)
    bottom = min(y for _, y in compiled_lines + reference_lines) / 2
    _ = axis.vlines(
        [e for e, _ in compiled_lines],
        bottom,
        [y for _, y in compiled_lines],
        color="#164b78",
        linewidth=1,
        label="Compiled emission lines (finite tickets)",
    )
    if reference_lines:
        _ = axis.plot(
            [e for e, _ in reference_lines],
            [y for _, y in reference_lines],
            "o",
            markersize=3,
            markerfacecolor="none",
            color="#478c59",
            label="Selected reference lines",
        )
    if data:
        counts = sorted(Counter(data).items())
        _ = axis.errorbar(
            [energy / scale for energy, _ in counts],
            [count / parent_decays for _, count in counts],
            yerr=[math.sqrt(count) / parent_decays for _, count in counts],
            fmt=".",
            markersize=4,
            elinewidth=0.7,
            color="#bb4a32",
            label=f"Sampled primaries (n={len(data)}; √N error bars)",
        )
    else:
        _ = axis.text(
            0.98,
            0.97,
            "No sampled primaries",
            ha="right",
            va="top",
            transform=axis.transAxes,
        )
    _ = axis.set(
        xlabel="Energy (keV)",
        ylabel="Emissions per parent decay (line intensity)",
        yscale="log",
        ylim=(bottom, None),
        xlim=(max(0, axis.get_xlim()[0]), None),
    )
    _ = axis.legend(fontsize="small")


def plot_continuum(
    axis: Axes,
    groups: list[RuntimeGroup],
    selected: list[ReferenceGroup],
    data: list[int],
    parent_decays: float,
    scale: int,
) -> None:
    edges = sorted(
        {
            group.lower_edge + index * group.width
            for group in groups
            for index in range(len(group.energies) + 1)
        }
    )
    density: list[float] = []
    for left in edges[:-1]:
        contributions: list[float] = []
        for group in groups:
            if group.lower_edge <= left < group.upper_edge:
                index = (left - group.lower_edge) // group.width
                lower = group.upper_tickets[index - 1] if index else 0
                count = group.upper_tickets[index] - lower
                contributions.append(
                    float(group.yield_per_decay)
                    * count
                    * scale
                    / (group.ticket_space * group.width)
                )
        density.append(math.fsum(contributions))
    _ = axis.stairs(
        density,
        [edge / scale for edge in edges],
        color="#164b78",
        label="Compiled emission density (finite-ticket bin masses)",
    )

    spectra = [
        (group.spectrum, float(group.yield_per_decay))
        for group in selected
        if group.spectrum is not None
    ]
    if spectra:
        knots = sorted(
            {energy for spectrum, _ in spectra for energy in spectrum.energy}
        )
        x: list[float] = []
        y: list[float] = []
        for left, right in pairwise(knots):
            # Each segment uses only references covering that whole interval.
            # Repeated endpoints retain support jumps without extending a curve.
            active = [
                (s, yield_)
                for s, yield_ in spectra
                if s.energy[0] <= left and right <= s.energy[-1]
            ]
            x.extend((left, right))
            y.extend(
                math.fsum(yield_ * s.pdf(energy) for s, yield_ in active)
                for energy in (left, right)
            )
        _ = axis.plot(
            x,
            y,
            color="#478c59",
            linestyle="--",
            label="Selected reference emission density",
        )

    if data:
        counts, bins = np.histogram(data, bins=80, range=(edges[0], edges[-1]))
        widths = np.diff(bins) / scale
        _ = axis.errorbar(
            (bins[:-1] + bins[1:]) / (2 * scale),
            counts / (parent_decays * widths),
            yerr=np.sqrt(counts) / (parent_decays * widths),
            fmt=".",
            markersize=4,
            elinewidth=0.7,
            color="#bb4a32",
            label=f"Sampled primaries (n={len(data)}; √N error bars)",
        )
    else:
        _ = axis.text(
            0.98,
            0.97,
            "No sampled primaries",
            ha="right",
            va="top",
            transform=axis.transAxes,
        )
    _ = axis.set(
        xlabel="Energy (keV)",
        ylabel="Emission density (per parent decay per keV)",
        ylim=(0, None),
        xlim=(0, None),
    )
    _ = axis.legend(fontsize="small")


def plot_particle_spectra(
    plots: Path,
    runtime: Runtime,
    reference: Reference,
    energies: dict[int, list[int]],
    parent_decays: float,
) -> None:
    particles: dict[str, list[RuntimeGroup]] = defaultdict(list)
    for group in runtime.groups:
        particles[group.particle].append(group)
    names = {
        "Alpha": ("alpha", "alpha"),
        "Positron": ("beta_plus", "positron"),
        "Gamma": ("gamma", "gamma"),
        "Electron": ("electron", "electron"),
    }
    for particle, groups in particles.items():
        token, title = names[particle]
        continuous = [group for group in groups if group.kind == "RegularSpectrum"]
        discrete = [group for group in groups if group.kind != "RegularSpectrum"]
        selected = [group for group in reference.groups if group.particle == particle]
        panels = int(bool(continuous)) + int(bool(discrete))
        figure = plt.figure(figsize=(10, 4.5 * panels), layout="constrained")
        _ = figure.suptitle(f"{runtime.name} {title} emission spectrum")
        if continuous:
            axis = figure.add_subplot(panels, 1, 1)
            plot_continuum(
                axis,
                continuous,
                selected,
                [energy for group in continuous for energy in energies[group.index]],
                parent_decays,
                runtime.energy_scale,
            )
        if discrete:
            axis = figure.add_subplot(panels, 1, panels)
            plot_lines(
                axis,
                discrete,
                selected,
                [energy for group in discrete for energy in energies[group.index]],
                parent_decays,
                runtime.energy_scale,
            )
        figure.savefig(plots / f"{runtime.name}_emission_{token}.png", dpi=180)
        plt.close(figure)


# ----------------------------------------------------------------------------


def make_plots(directory: Path) -> None:
    settings = load_json(directory / "settings.json")
    result = load_json(directory / "analysis.json")
    reference = Reference.load(Path(str(settings["reference"])))
    runtime = Runtime.load(directory / "run")
    windows = cast(list[JsonObject], result["windows"])
    parent_decays = math.fsum(
        float(str(window["independent_parent_decays"])) for window in windows
    )
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
    populated = [
        (start, cast(JsonObject, window["birth_times"]))
        for start, window in zip(x, windows)
        if cast(JsonObject, window["birth_times"])["status"] != "insufficient_samples"
    ]
    figure = plt.figure(figsize=(10, 7 if populated else 4.5), layout="constrained")
    axis = figure.add_subplot(211 if populated else 111)
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
    if populated:
        axis = figure.add_subplot(212)
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
        data = energies[group.index]
        data.sort()
        selected = (
            reference.groups[group.index]
            if group.index < len(reference.groups)
            else None
        )
        if group.kind != "RegularSpectrum":
            figure = plt.figure(figsize=(10, 4.5), layout="constrained")
            axis = figure.add_subplot(111)
            plot_lines(
                axis,
                [group],
                [selected] if selected is not None else [],
                data,
                parent_decays,
                runtime.energy_scale,
            )
            _ = axis.set_title(
                f"{runtime.name}: group {group.index} {group.particle} emission spectrum"
            )
            figure.savefig(plots / f"energy_group_{group.index}.png", dpi=180)
            plt.close(figure)
            continue

        comparison = selected is not None and selected.spectrum is not None
        figure = plt.figure(
            figsize=(10, 7 if comparison else 4.5), layout="constrained"
        )
        axis = figure.add_subplot(211 if comparison else 111)
        lower = group.lower_edge / runtime.energy_scale
        upper = group.upper_edge / runtime.energy_scale
        if selected is not None and selected.spectrum is not None:
            lower = min(lower, selected.spectrum.energy[0])
            upper = max(upper, selected.spectrum.energy[-1])

        if data:
            _ = axis.hist(
                [energy / runtime.energy_scale for energy in data],
                bins=80,
                range=(lower, upper),
                density=True,
                histtype="step",
                color="#bb4a32",
                label=f"Generated primaries (n={len(data)})",
            )
        else:
            _ = axis.text(
                0.98,
                0.97,
                "No sampled primaries",
                ha="right",
                va="top",
                transform=axis.transAxes,
            )
        if selected is not None and selected.spectrum is not None:
            spectrum = selected.spectrum
            _ = axis.plot(
                spectrum.energy,
                [value / spectrum.area for value in spectrum.density],
                color="#164b78",
                label="Selected reference (conditional density)",
            )
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
            title=f"{runtime.name}: group {group.index} {group.particle} emission spectrum",
        )
        _ = axis.legend(fontsize="small")
        if selected is not None and selected.spectrum is not None:
            axis = figure.add_subplot(212)
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
                    marker="o" if len(data) == 1 else None,
                    label="Generated ECDF − selected reference CDF",
                )
            grid = [
                (group.lower_edge + index * group.width) / runtime.energy_scale
                for index in range(len(group.energies) + 1)
            ]
            knots = sorted(set(grid + list(spectrum.energy)))
            grid = knots.copy()
            # Use the same quadratic-piece extrema as analysis.grid_comparison.
            # Extra CDF evaluations render the curvature between those extrema.
            for a, b in pairwise(knots):
                grid.extend(a + fraction * (b - a) for fraction in (0.25, 0.5, 0.75))
                slope = (spectrum.pdf(b) - spectrum.pdf(a)) / (b - a)
                grid_density = (
                    group.continuous_cdf(b * runtime.energy_scale)
                    - group.continuous_cdf(a * runtime.energy_scale)
                ) / (b - a)
                if slope != 0:
                    stationary = a + (grid_density - spectrum.pdf(a)) / slope
                    if a < stationary < b:
                        grid.append(stationary)
            grid = sorted(set(grid))
            _ = axis.plot(
                grid,
                [
                    group.continuous_cdf(value * runtime.energy_scale)
                    - spectrum.cdf(value)
                    for value in grid
                ],
                color="#478c59",
                label="Compiled grid CDF - selected reference CDF",
            )
            _ = axis.legend(fontsize="small")
            _ = axis.axhline(0, color="0.5", linewidth=1)
            _ = axis.set(xlabel="Energy (keV)", ylabel="CDF difference")
        figure.savefig(plots / f"energy_group_{group.index}.png", dpi=180)
        plt.close(figure)

    plot_particle_spectra(plots, runtime, reference, energies, parent_decays)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    _ = parser.add_argument(
        "campaign",
        type=Path,
        help="Campaign directory with analysis.json and retained samples. Writes population, birth-time, detailed group and aggregate particle-type emission PNG figures to plots/; no simulation is run.",
    )
    args = cast(dict[str, Path], vars(parser.parse_args()))
    make_plots(args["campaign"])


if __name__ == "__main__":
    main()
