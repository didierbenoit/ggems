import argparse
import json
import subprocess
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
        description="Run exact T1 CountDriven chronology cases through production GGEMS Source."
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
        "--primaries",
        type=positive_integer,
        default=DEFAULT_PRIMARIES,
        help="Primaries per logical Run; 256 is sufficient for exact time equality.",
    )
    _ = parser.add_argument("--workers", type=positive_integer, default=DEFAULT_WORKERS)
    _ = parser.add_argument("--seed", type=nonnegative_integer, default=DEFAULT_SEED)
    _ = parser.add_argument(
        "--no-plots", action="store_true", help="Use only the Python standard library."
    )
    _ = parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "results" / "time",
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
    from analyze import analyze_case  # pyright: ignore[reportImplicitRelativeImport]

    exporter = args.exporter.resolve()
    if not exporter.is_file():
        raise FileNotFoundError(f"Source sample exporter not found: {exporter}")
    output_root = args.output_dir.resolve()
    commit = checkout_commit()

    for case in CASES:
        if case.name not in args.cases:
            continue

        # One exporter process per CASE; repeated Runs remain inside the same
        # initialized GGEMSRun object. A new directory excludes stale results.
        output = output_root / case.name
        output.mkdir(parents=True, exist_ok=False)
        metadata_path = output / "metadata.json"
        command = [
            str(exporter),
            "--device",
            args.device,
            "--geometry",
            "point",
            "--angular",
            "fixed",
            "--energy-mode",
            "mono",
            "--mono-energy-kev",
            "511",
            "--case-name",
            case.name,
            "--chronology",
            case.chronology,
            "--primaries",
            str(args.primaries),
            "--workers",
            str(args.workers),
            "--seed",
            str(args.seed),
            "--sequence-dir",
            str(output / "runs"),
            "--sequence-runs",
            str(len(case.windows_ps)),
            "--metadata",
            str(metadata_path),
        ]
        if case.requested_ns is not None:
            start, stop, step = case.requested_ns
            command.extend(
                [
                    "--time-start-ns",
                    str(start),
                    "--time-stop-ns",
                    str(stop),
                    "--time-step-ns",
                    str(step),
                ]
            )
        if case.reset_before_run is not None:
            command.extend(["--reset-before-run", str(case.reset_before_run)])

        print(
            f"{case.name}: {len(case.windows_ps)} logical Run(s), "
            + f"{args.primaries} Source primaries per Run on {args.device}",
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

        _ = analyze_case(metadata_path, output, make_figures=not args.no_plots)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
