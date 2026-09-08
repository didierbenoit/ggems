from pathlib import Path

import matplotlib.pyplot as plt
from cases import TimeCase  # pyright: ignore[reportImplicitRelativeImport]


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
            _ = axis.plot(
                [start, stop], [index, index], color="tab:blue", linewidth=5, alpha=0.5
            )
            _ = axis.plot(
                [start],
                [index],
                "o",
                color="black",
                markersize=8,
                label="All Source births = window start" if index == 0 else None,
            )
            _ = axis.plot(
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
            _ = axis.text(
                0.02,
                index - 0.16,
                label,
                fontsize=10,
                transform=axis.get_yaxis_transform(),
            )

        _ = axis.set_yticks(
            list(range(len(windows_ps))),
            [f"Run sequence {i}" for i in range(len(windows_ps))],
        )
        _ = axis.set_xlabel("Run chronology [ns]")
        _ = axis.set_title(
            f"{case.name}\nCountDriven birth times follow the committed window start exactly"
        )
        _ = axis.set_xlim(0, max(stop for _, stop in windows_ps) / display_unit_ps + 5)
        _ = axis.set_ylim(len(windows_ps) - 0.5, -0.6)
        axis.grid(axis="x", alpha=0.25)
        _ = axis.legend(
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
