import argparse
import json
import subprocess
from pathlib import Path
from typing import Protocol, cast

# Direct script entry points: Python adds this directory to sys.path.
from cases import (  # pyright: ignore[reportImplicitRelativeImport]
    CASES,
    CENTER_MM,
    DEFAULT_INTEGRATED_PRIMARIES,
    DEFAULT_PAIR_PRIMARIES,
    DEFAULT_SEED,
    DEFAULT_WORKERS,
    FRAME_DIRECTION,
    FRAME_UP,
    LINE_ENERGIES_KEV,
    LINE_WEIGHTS,
    PAIR_WORKERS,
    REGULAR_CENTERS_KEV,
    REGULAR_WEIGHTS,
    REGULAR_WIDTH_KEV,
    Configuration,
)


class Arguments(Protocol):
    exporter: Path
    device: str
    cases: list[str]
    integrated_primaries: int
    pair_primaries: int
    workers: int
    seed: int
    output_dir: Path
    no_plots: bool


def nonnegative_integer(value: str) -> int:
    number = int(value)
    if number < 0:
        raise argparse.ArgumentTypeError("Expected a nonnegative integer.")
    return number


def positive_integer(value: str) -> int:
    number = nonnegative_integer(value)
    if number == 0:
        raise argparse.ArgumentTypeError("Expected a positive integer.")
    return number


def parse_arguments() -> Arguments:
    parser = argparse.ArgumentParser(
        description="Run I1 integrated Source sampling and exact RNG-budget pairs."
    )
    _ = parser.add_argument("--exporter", type=Path, required=True)
    _ = parser.add_argument(
        "--device", required=True, help="Forwarded unchanged to GGEMS SelectDevices."
    )
    _ = parser.add_argument(
        "--cases",
        nargs="+",
        choices=[case.name for case in CASES],
        default=[case.name for case in CASES],
    )
    _ = parser.add_argument(
        "--integrated-primaries",
        type=positive_integer,
        default=DEFAULT_INTEGRATED_PRIMARIES,
    )
    _ = parser.add_argument(
        "--pair-primaries", type=positive_integer, default=DEFAULT_PAIR_PRIMARIES
    )
    _ = parser.add_argument(
        "--workers",
        type=positive_integer,
        default=DEFAULT_WORKERS,
        help="Integrated case only; exact pairs always use one worker and require one device.",
    )
    _ = parser.add_argument("--seed", type=nonnegative_integer, default=DEFAULT_SEED)
    _ = parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "results" / "integration",
    )
    _ = parser.add_argument("--no-plots", action="store_true")
    return cast(Arguments, cast(object, parser.parse_args()))


def checkout_commit() -> str | None:
    try:
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=Path(__file__).resolve().parents[3],
            check=True,
            capture_output=True,
            text=True,
            encoding="utf-8",
        )
    except (OSError, subprocess.CalledProcessError):
        return None
    return result.stdout.strip() or None


def export_run(
    exporter: Path,
    configuration: Configuration,
    name: str,
    output: Path,
    args: Arguments,
    commit: str | None,
    *,
    paired: bool,
) -> None:
    from analyze import load_metadata  # pyright: ignore[reportImplicitRelativeImport]

    count = args.pair_primaries if paired else args.integrated_primaries
    workers = PAIR_WORKERS if paired else args.workers
    regular = configuration.energy == "regular-spectrum"
    energies = REGULAR_CENTERS_KEV if regular else LINE_ENERGIES_KEV
    weights = REGULAR_WEIGHTS if regular else LINE_WEIGHTS
    output.mkdir(parents=True, exist_ok=True)
    command = [
        str(exporter),
        "--device",
        args.device,
        "--case-name",
        name,
        "--geometry",
        configuration.geometry,
        "--angular",
        configuration.angular,
        "--energy-mode",
        configuration.energy,
        "--chronology",
        "static",
        "--primaries",
        str(count),
        "--workers",
        str(workers),
        "--seed",
        str(args.seed),
        "--output",
        str(output / "samples.csv"),
        "--metadata",
        str(output / "metadata.json"),
        "--energy-values-kev",
        ",".join(map(str, energies)),
        "--energy-weights",
        ",".join(map(str, weights)),
    ]
    for axis, dimension, coordinate in zip(
        "xyz", configuration.dimensions_mm, CENTER_MM, strict=True
    ):
        command.extend(
            [
                f"--size-{axis}-mm",
                str(dimension),
                f"--center-{axis}-mm",
                str(coordinate),
            ]
        )
    for vector, values in (("direction", FRAME_DIRECTION), ("up", FRAME_UP)):
        for axis, value in zip("xyz", values, strict=True):
            command.extend([f"--frame-{vector}-{axis}", str(value)])
    if configuration.bounds_deg is not None:
        for flag, value in zip(
            ("theta-min", "theta-max", "phi-min", "phi-max"),
            configuration.bounds_deg,
            strict=True,
        ):
            command.extend([f"--{flag}-deg", str(value)])
    if regular:
        command.extend(["--energy-bin-width-kev", str(REGULAR_WIDTH_KEV)])

    print(
        f"{name}: extracting {count} Source primaries; workers={workers}; selector={args.device}",
        flush=True,
    )
    with (output / "export.log").open("w", encoding="utf-8") as log:
        try:
            _ = subprocess.run(
                command, cwd=output, stdout=log, stderr=subprocess.STDOUT, check=True
            )
        except (OSError, subprocess.CalledProcessError) as error:
            raise RuntimeError(
                f"{name}: exporter failed; see {output / 'export.log'}"
            ) from error

    # Single-device validation uses the actual runtime selection, not a second
    # selector parser. Reject an unsuitable pair before launching its next member.
    path = output / "metadata.json"
    metadata = load_metadata(path, configuration, name, paired=paired)
    for key, expected in (
        ("primary_count", count),
        ("worker_count", workers),
        ("seed", args.seed),
        ("device_selector", args.device),
    ):
        if metadata.raw[key] != expected:
            raise ValueError(f"Exporter did not execute the requested {key}.")
    if commit is not None:
        metadata.raw["git_commit"] = commit
        _ = path.write_text(
            json.dumps(metadata.raw, indent=2, allow_nan=False) + "\n", encoding="utf-8"
        )
    print(f"  actual devices: {metadata.raw['device_names']}", flush=True)


def main() -> int:
    args = parse_arguments()
    from analyze import analyze_case  # pyright: ignore[reportImplicitRelativeImport]

    exporter = args.exporter.resolve()
    if not exporter.is_file():
        raise FileNotFoundError(f"Source sample exporter not found: {exporter}")
    output_root = args.output_dir.resolve()
    commit = checkout_commit()

    for case in CASES:
        if case.name not in args.cases:
            continue
        output = output_root / case.name
        # A failed capture must not inherit old samples, summaries or figures.
        output.mkdir(parents=True, exist_ok=False)
        if case.comparison is None:
            export_run(
                exporter, case.reference, case.name, output, args, commit, paired=False
            )
        else:
            print("  Exact pair: one worker and one device are mandatory.", flush=True)
            export_run(
                exporter,
                case.reference,
                case.name + "_reference",
                output / "reference",
                args,
                commit,
                paired=True,
            )
            export_run(
                exporter,
                case.comparison,
                case.name + "_comparison",
                output / "comparison",
                args,
                commit,
                paired=True,
            )
        _ = analyze_case(case, output, output, make_figures=not args.no_plots)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
