import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Protocol, cast

# Direct script entry points: Python adds this directory to sys.path.
from cases import (  # pyright: ignore[reportImplicitRelativeImport]
    CASES,
    DEFAULT_PRIMARIES,
    DEFAULT_SEED,
    DEFAULT_WORKERS,
)


class Arguments(Protocol):
    exporter: Path
    device: str
    cases: list[str]
    primaries: int
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
        description="Run canonical A1 angular cases through production GGEMS Source."
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
        "--primaries", type=positive_integer, default=DEFAULT_PRIMARIES
    )
    _ = parser.add_argument("--workers", type=positive_integer, default=DEFAULT_WORKERS)
    _ = parser.add_argument("--seed", type=nonnegative_integer, default=DEFAULT_SEED)
    _ = parser.add_argument(
        "--no-plots", action="store_true", help="Measure with NumPy only."
    )
    _ = parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "results" / "angle",
    )
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


def main() -> int:
    args = parse_arguments()
    # Keep --help usable before loading scientific dependencies.
    try:
        from analyze import analyze_case  # pyright: ignore[reportImplicitRelativeImport]
    except ModuleNotFoundError as error:
        print(f"Scientific Python dependency unavailable: {error}.", file=sys.stderr)
        return 1

    exporter = args.exporter.resolve()
    if not exporter.is_file():
        raise FileNotFoundError(f"Source sample exporter not found: {exporter}")

    output_root = args.output_dir.resolve()
    commit = checkout_commit()

    for case in CASES:
        if case.name not in args.cases:
            continue

        output = output_root / case.name
        # Reject reuse so a failed extraction cannot leave a stale scientific result.
        output.mkdir(parents=True, exist_ok=False)
        samples_path = output / "samples.csv"
        metadata_path = output / "metadata.json"
        command = [
            str(exporter),
            "--device",
            args.device,
            "--geometry",
            case.geometry,
            "--case-name",
            case.name,
            "--angular",
            case.angular,
            "--primaries",
            str(args.primaries),
            "--workers",
            str(args.workers),
            "--seed",
            str(args.seed),
            "--output",
            str(samples_path),
            "--metadata",
            str(metadata_path),
        ]
        for axis, size in zip(("x", "y", "z"), case.dimensions_mm, strict=True):
            command.extend([f"--size-{axis}-mm", str(size)])

        if case.bounds_deg is not None:
            for bound, value in zip(
                ("theta-min", "theta-max", "phi-min", "phi-max"),
                case.bounds_deg,
                strict=True,
            ):
                command.extend([f"--{bound}-deg", str(value)])

        if case.focus_mm is not None:
            for axis, value in zip(("x", "y", "z"), case.focus_mm, strict=True):
                command.extend([f"--focus-{axis}-mm", str(value)])

        print(
            f"{case.name}: extracting {args.primaries} Source primaries on {args.device}",
            flush=True,
        )
        with (output / "export.log").open("w", encoding="utf-8") as log:
            try:
                _ = subprocess.run(
                    command,
                    cwd=output,
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    check=True,
                )
            except (OSError, subprocess.CalledProcessError) as error:
                raise RuntimeError(
                    f"{case.name}: exporter failed; see {output / 'export.log'}"
                ) from error

        if commit is not None:
            raw = cast(object, json.loads(metadata_path.read_text(encoding="utf-8")))
            if not isinstance(raw, dict):
                raise TypeError(f"Expected an object in {metadata_path}.")
            metadata = cast(dict[str, object], raw)
            metadata["git_commit"] = commit
            _ = metadata_path.write_text(
                json.dumps(metadata, indent=2, allow_nan=False) + "\n", encoding="utf-8"
            )

        _ = analyze_case(
            samples_path, metadata_path, output, make_figures=not args.no_plots
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
