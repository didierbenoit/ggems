"""Run one generic half-life-scaled campaign without changing GGEMS configuration."""

import argparse
import importlib.metadata
import platform
import subprocess
import sys
from decimal import Decimal
from pathlib import Path
from typing import Protocol, cast

from validation.radioactivity.radionuclide_validation.analysis import analyze_campaign
from validation.radioactivity.radionuclide_validation.model import (
    Reference,
    Runtime,
    write_json,
)
from validation.radioactivity.radionuclide_validation.numerics import design_campaign


class Arguments(Protocol):
    exporter: Path
    reference: Path
    output: Path
    device: str
    workers: int
    seed: int
    windows: int
    horizon_half_lives: Decimal
    target_last_window: int
    population_replicates: int
    family_alpha: Decimal
    source_tree: Path
    revision: str
    timeout_seconds: int


def invoke(command: list[str], log_path: Path, timeout_seconds: int) -> None:
    with log_path.open("w", encoding="utf-8") as log:
        _ = log.write(subprocess.list2cmdline(command) + "\n")
        log.flush()
        _ = subprocess.run(
            command,
            stdout=log,
            stderr=subprocess.STDOUT,
            check=True,
            timeout=timeout_seconds,
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    _ = parser.add_argument("--exporter", type=Path, required=True)
    _ = parser.add_argument("--reference", type=Path, required=True)
    _ = parser.add_argument("--output", type=Path, required=True)
    _ = parser.add_argument(
        "--device",
        default="cpu",
        help="Forwarded unchanged to GGEMSOpenCL.SelectDevices.",
    )
    _ = parser.add_argument("--workers", type=int, default=256)
    _ = parser.add_argument("--timeout-seconds", type=int, default=1800)
    _ = parser.add_argument("--seed", type=int, default=77777)
    _ = parser.add_argument("--windows", type=int, default=32)
    _ = parser.add_argument("--horizon-half-lives", type=Decimal, default=Decimal(4))
    _ = parser.add_argument("--target-last-window", type=int, default=1000)
    _ = parser.add_argument("--population-replicates", type=int, default=128)
    _ = parser.add_argument("--family-alpha", type=Decimal, default=Decimal("0.01"))
    _ = parser.add_argument(
        "--source-tree", type=Path, default=Path(__file__).resolve().parents[2]
    )
    _ = parser.add_argument(
        "--revision",
        required=True,
        help="Reviewed source revision; no Git operation is performed.",
    )
    args = cast(Arguments, cast(object, parser.parse_args()))
    if (
        not 0 < args.family_alpha < 1
        or args.workers < 1
        or not 0 <= args.seed < 2**64
        or args.population_replicates < 0
    ):
        parser.error(
            "Invalid statistical policy, worker count, seed or replication count."
        )
    reference_path = args.reference.resolve()
    reference = Reference.load(reference_path)
    raw_count = reference.verify_raw_files(reference_path)
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    exporter = str(args.exporter.resolve(strict=True))
    describe_command = [
        exporter,
        "--describe",
        "--nuclide",
        reference.name,
        "--output",
        str(output / "definition"),
    ]
    if args.timeout_seconds <= 0:
        parser.error("Timeout must be positive.")
    invoke(describe_command, output / "describe.log", args.timeout_seconds)
    runtime = Runtime.load(output / "definition")
    design = design_campaign(
        runtime, args.horizon_half_lives, args.windows, args.target_last_window
    )
    command = [
        exporter,
        "--nuclide",
        reference.name,
        "--output",
        str(output / "run"),
        "--device",
        args.device,
        "--workers",
        str(args.workers),
        "--seed",
        str(args.seed),
        "--windows",
        str(design.windows),
        "--step-ps",
        str(design.step_ps),
        "--activity-bq",
        str(design.activity_bq),
        "--capacity",
        str(design.capacity),
        "--population-replicates",
        str(args.population_replicates),
    ]
    design_values = {
        "step_ps": design.step_ps,
        "windows": design.windows,
        "activity_bq": str(design.activity_bq),
        "capacity": design.capacity,
        "requested_horizon_half_lives": str(design.requested_horizon_half_lives),
        "actual_horizon_half_lives": str(design.actual_horizon_half_lives),
        "target_last_window": design.target_last_window,
        "horizon_policy": design.horizon_policy,
    }
    # Freeze the complete design and decision thresholds before production draws.
    write_json(
        output / "settings.json",
        {
            "schema_version": 1,
            "reference": str(reference_path),
            "reference_id": reference.raw["reference_id"],
            "verified_raw_files": raw_count,
            "source_tree": str(args.source_tree.resolve()),
            "source_revision": args.revision,
            "design": design_values,
            "family_alpha": str(args.family_alpha),
            "population_replicates": args.population_replicates,
            "seed": args.seed,
            "workers_per_device": args.workers,
            "device_selector": args.device,
            "timeout_seconds": args.timeout_seconds,
            "commands": [describe_command, command],
            "python": sys.version,
            "platform": platform.platform(),
            "dependencies": {
                name: importlib.metadata.version(name)
                for name in ("numpy", "scipy", "matplotlib")
            },
            "rng_contract": "Production Point/Fixed uses one birth-time word followed by the conditional-energy word (Mono uses none). Replay and repeated population experiments use separate host planners and never advance Run streams.",
            "statistical_policy": "Bonferroni m = windows*(2*groups+3)+5*groups+1; exact Poisson prediction intervals, replicated discrete ECDF tests, large-mean two-sided dispersion, conditional-time and energy DKW tests. No thresholds, seeds or retention changes after results.",
        },
    )
    invoke(command, output / "exporter.log", args.timeout_seconds)
    result = analyze_campaign(output, reference, args.source_tree.resolve())
    print(
        f"Completed {reference.name}: {result['sample_count']} primaries; results in {output}"
    )
    print(
        "Scientific discrepancies remain in analysis.json; completion is not a claim that every hypothesis passed."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
