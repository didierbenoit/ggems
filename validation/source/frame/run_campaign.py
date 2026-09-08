import argparse
import json
import subprocess
from pathlib import Path
from typing import Protocol, cast

# Direct script entry points: Python adds this directory to sys.path.
from cases import (  # pyright: ignore[reportImplicitRelativeImport]
    BOUNDS_DEG,
    CASES,
    DEFAULT_EXACT_PRIMARIES,
    DEFAULT_SEED,
    DEFAULT_STATISTICAL_PRIMARIES,
    DEFAULT_WORKERS,
    FOCUS_MM,
    ORIGIN,
    FrameCase,
    orientation_request,
)


class Arguments(Protocol):
    exporter: Path
    device: str
    cases: list[str]
    exact_primaries: int
    statistical_primaries: int
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
        description="Run G2/A2 Source frame cases through the production OpenCL path."
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
        "--exact-primaries", type=positive_integer, default=DEFAULT_EXACT_PRIMARIES
    )
    _ = parser.add_argument(
        "--statistical-primaries",
        type=positive_integer,
        default=DEFAULT_STATISTICAL_PRIMARIES,
    )
    _ = parser.add_argument(
        "--workers",
        type=positive_integer,
        default=DEFAULT_WORKERS,
        help="Workers for unpaired cases. Pairs always use one worker and require one device.",
    )
    _ = parser.add_argument("--seed", type=nonnegative_integer, default=DEFAULT_SEED)
    _ = parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "results" / "frame",
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
    case: FrameCase,
    output: Path,
    args: Arguments,
    commit: str | None,
    *,
    reference: bool,
) -> tuple[Path, Path]:
    from analyze import load_metadata  # pyright: ignore[reportImplicitRelativeImport]

    output.mkdir(parents=True, exist_ok=False)
    samples_path = output / "samples.csv"
    metadata_path = output / "metadata.json"
    name = case.name + ("_reference" if reference else "")
    count = args.statistical_primaries if case.statistical else args.exact_primaries
    workers = 1 if case.paired else args.workers
    center = ORIGIN if reference else case.center_mm
    orientation = orientation_request("identity" if reference else case.frame)
    command = [
        str(exporter),
        "--device",
        args.device,
        "--geometry",
        case.geometry,
        "--angular",
        case.angular,
        "--energy-mode",
        "mono",
        "--mono-energy-kev",
        "511",
        "--chronology",
        "static",
        "--case-name",
        name,
        "--primaries",
        str(count),
        "--workers",
        str(workers),
        "--seed",
        str(args.seed),
        "--output",
        str(samples_path),
        "--metadata",
        str(metadata_path),
    ]
    for axis, dimension, coordinate in zip(
        "xyz", case.dimensions_mm, center, strict=True
    ):
        command.extend(
            [
                f"--size-{axis}-mm",
                str(dimension),
                f"--center-{axis}-mm",
                str(coordinate),
            ]
        )
    if orientation is not None:
        for vector, values in zip(("direction", "up"), orientation, strict=True):
            for axis, value in zip("xyz", values, strict=True):
                command.extend([f"--frame-{vector}-{axis}", str(value)])
    if case.angular == "bounded-isotropic":
        for flag, value in zip(
            ("theta-min", "theta-max", "phi-min", "phi-max"), BOUNDS_DEG, strict=True
        ):
            command.extend([f"--{flag}-deg", str(value)])
    elif case.angular == "focused":
        for axis, value in zip("xyz", FOCUS_MM, strict=True):
            command.extend([f"--focus-{axis}-mm", str(value)])

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

    # Validate actual single-device selection immediately, before the second
    # member of a pair. The runner never parses GGEMS device selectors itself.
    metadata = load_metadata(metadata_path, case, reference=reference)
    if commit is not None:
        metadata.raw["git_commit"] = commit
        _ = metadata_path.write_text(
            json.dumps(metadata.raw, indent=2, allow_nan=False) + "\n", encoding="utf-8"
        )
    print(f"  actual devices: {metadata.raw['device_names']}", flush=True)
    return samples_path, metadata_path


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
        # A failed rerun must never inherit old samples, summaries or figures.
        output.mkdir(parents=True, exist_ok=False)
        reference_samples, reference_metadata = None, None
        if case.paired:
            print(
                "  Paired replay: one worker and one device are mandatory.", flush=True
            )
            reference_samples, reference_metadata = export_run(
                exporter, case, output / "reference", args, commit, reference=True
            )
        samples, metadata = export_run(
            exporter, case, output / "transformed", args, commit, reference=False
        )
        _ = analyze_case(
            samples,
            metadata,
            output,
            reference_samples=reference_samples,
            reference_metadata=reference_metadata,
            make_figures=not args.no_plots,
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
