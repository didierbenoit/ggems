#!/usr/bin/env python3

import argparse
import json
import shlex
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

# ------------------------------------------------------------------------------

DIEHARDER_REFERENCE_VERSION = "3.31.1"
EXPECTED_ASSESSMENT_COUNT = 114

WORKER_COUNT = 1_048_576
SAMPLES_PER_WORKER = 61_036
STREAM_BYTE_COUNT = WORKER_COUNT * SAMPLES_PER_WORKER * 4
STREAM_TYPE = "raw_uint32"
MIN_FREE_MARGIN_BYTES = 8 * 1024**3
FREE_SPACE_POLL_SECONDS = 2.0
FREE_SPACE_TIMEOUT_SECONDS = 30.0 * 60.0

# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class CampaignCase:
    case_id: str
    engine: str
    seed: int
    stream_offset: int
    layout: str

    @property
    def stem(self) -> str:
        return f"{self.case_id}-{self.engine}"


# ------------------------------------------------------------------------------

CAMPAIGN_CASES: tuple[CampaignCase, ...] = (
    CampaignCase("A01", "philox", 741_750_298, 0, "interleaved"),
    CampaignCase("A02", "pcg32", 741_750_298, 0, "interleaved"),
    CampaignCase("A03", "jkiss", 741_750_298, 0, "interleaved"),
    CampaignCase("A04", "philox", 3_392_895_365, 0, "interleaved"),
    CampaignCase("A05", "pcg32", 3_392_895_365, 0, "interleaved"),
    CampaignCase("A06", "jkiss", 3_392_895_365, 0, "interleaved"),
    CampaignCase("A07", "philox", 2_374_528_848, 0, "interleaved"),
    CampaignCase("A08", "pcg32", 2_374_528_848, 0, "interleaved"),
    CampaignCase("A09", "jkiss", 2_374_528_848, 0, "interleaved"),
    CampaignCase("E01", "jkiss", 741_750_298, 0, "worker_major"),
    CampaignCase("E06", "jkiss", 3_392_895_365, 0, "worker_major"),
    CampaignCase("E12", "jkiss", 2_374_528_848, 0, "worker_major"),
    CampaignCase("F01", "jkiss", 741_750_298, 1_048_576, "interleaved"),
)

# ------------------------------------------------------------------------------


class Arguments(Protocol):
    list: bool
    case: str | None
    all: bool
    dry_run: bool
    generator: Path
    runner: Path
    dieharder: str
    device: str
    local_size: int
    force: bool
    keep_stream: bool


# ------------------------------------------------------------------------------


def RepositoryRoot() -> Path:
    return Path(__file__).resolve().parents[3]


# ------------------------------------------------------------------------------


def ParseArguments() -> Arguments:
    root = RepositoryRoot()
    parser = argparse.ArgumentParser(
        description="Run the fixed 13-case GGEMS Dieharder reference campaign."
    )

    action = parser.add_mutually_exclusive_group()
    _ = action.add_argument(
        "--list",
        action="store_true",
        help="List every fixed Dieharder campaign case.",
    )
    _ = action.add_argument(
        "--case",
        metavar="CASE_ID",
        help="Run exactly one fixed campaign case, for example A01-philox.",
    )
    _ = action.add_argument(
        "--all",
        action="store_true",
        help="Run all fixed campaign cases sequentially.",
    )

    _ = parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print commands without executing them.",
    )
    _ = parser.add_argument(
        "--generator",
        type=Path,
        default=root / "build/validation/random/ggems_random_stream_generator",
        help="GGEMS random stream generator executable.",
    )
    _ = parser.add_argument(
        "--runner",
        type=Path,
        default=root / "validation/random/dieharder/run_dieharder.py",
        help="GGEMS Dieharder single-run wrapper.",
    )
    _ = parser.add_argument(
        "--dieharder",
        default="dieharder",
        help="Dieharder executable or path.",
    )
    _ = parser.add_argument(
        "--device",
        default="gpu",
        help="GGEMS device selector (default: gpu).",
    )
    _ = parser.add_argument(
        "--local-size",
        type=int,
        default=64,
        help="OpenCL local work-group size (default: 64).",
    )
    _ = parser.add_argument(
        "--force",
        action="store_true",
        help="Rerun completed cases and overwrite existing campaign artifacts.",
    )
    _ = parser.add_argument(
        "--keep-stream",
        "--keep-streams",
        dest="keep_stream",
        action="store_true",
        help="Keep the generated stream after a single-case run; not valid with --all.",
    )

    return cast(Arguments, cast(object, parser.parse_args()))


# ------------------------------------------------------------------------------


def PrintCampaign(cases: tuple[CampaignCase, ...]) -> None:
    print("GGEMS Dieharder fixed validation campaign")
    print()
    print(f"{'Case':<13}{'Engine':<8}{'Seed':>11}  {'Layout':<13}{'Offset':>10}")
    print("-" * 58)

    for case in cases:
        print(
            f"{case.stem:<13}{case.engine:<8}{case.seed:>11}  "
            f"{case.layout:<13}{case.stream_offset:>10}"
        )

    print()
    print(f"Cases              : {len(cases)}")
    print(f"Workers per case   : {WORKER_COUNT}")
    print(f"Samples per worker : {SAMPLES_PER_WORKER}")
    print(f"GiB per case       : {STREAM_BYTE_COUNT / 1024**3:.8f}")


# ------------------------------------------------------------------------------


def FindCase(case_id: str) -> CampaignCase:
    requested = case_id.lower()
    for case in CAMPAIGN_CASES:
        if case.stem.lower() == requested:
            return case
    raise ValueError(f"Unknown Dieharder campaign case: {case_id}")


# ------------------------------------------------------------------------------


def CasePaths(case: CampaignCase) -> tuple[Path, Path, Path]:
    root = RepositoryRoot()
    stream = root / "validation/random/streams/dieharder" / f"{case.stem}.bin"
    manifest = (
        root
        / "validation/random/results/dieharder/campaign/manifests"
        / f"{case.stem}.json"
    )
    summary = (
        root / "validation/random/results/dieharder/campaign" / f"{case.stem}.json"
    )
    return stream, manifest, summary


# ------------------------------------------------------------------------------


def LoadJsonObject(path: Path) -> dict[str, object] | None:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return None
    return value if isinstance(value, dict) else None


# ------------------------------------------------------------------------------


def ManifestMatches(case: CampaignCase, path: Path) -> bool:
    root = LoadJsonObject(path)
    if root is None:
        return False
    random = root.get("random")
    if not isinstance(random, dict):
        return False

    engine = random.get("engine")
    if not isinstance(engine, str) or engine.lower() != case.engine:
        return False

    expected = {
        "seed": case.seed,
        "stream_offset": case.stream_offset,
        "stream_type": STREAM_TYPE,
        "layout": case.layout,
        "worker_count": WORKER_COUNT,
        "samples_per_worker": SAMPLES_PER_WORKER,
        "byte_count": STREAM_BYTE_COUNT,
    }
    return all(random.get(key) == value for key, value in expected.items())


# ------------------------------------------------------------------------------


def IsCompletedCase(case: CampaignCase) -> bool:
    _, manifest_path, summary_path = CasePaths(case)
    summary = LoadJsonObject(summary_path)
    if summary is None or not ManifestMatches(case, manifest_path):
        return False

    tool = summary.get("tool")
    input_section = summary.get("input")
    output = summary.get("output")
    if not isinstance(tool, dict) or not isinstance(input_section, dict):
        return False
    if not isinstance(output, dict):
        return False

    if tool.get("version") != DIEHARDER_REFERENCE_VERSION:
        return False
    if output.get("status") != "completed":
        return False
    if output.get("assessment_count") != EXPECTED_ASSESSMENT_COUNT:
        return False
    return output.get("stderr") == ""


# ------------------------------------------------------------------------------


def ResolveExecutable(value: str | Path, description: str) -> str:
    candidate = Path(value).expanduser()
    if candidate.is_file():
        return str(candidate.resolve())

    resolved = shutil.which(str(value))
    if resolved is None:
        raise FileNotFoundError(f"{description} not found: {value}")
    return str(Path(resolved).resolve())


# ------------------------------------------------------------------------------


def RunCommand(command: list[str], *, dry_run: bool) -> float:
    print(f"$ {shlex.join(command)}")
    if dry_run:
        return 0.0

    started = time.perf_counter()
    subprocess.run(command, check=True)
    return time.perf_counter() - started


# ------------------------------------------------------------------------------


def WaitForFreeSpace(stream_directory: Path, stream_path: Path) -> None:
    required_bytes = STREAM_BYTE_COUNT + MIN_FREE_MARGIN_BYTES
    deadline = time.monotonic() + FREE_SPACE_TIMEOUT_SECONDS

    while True:
        disk = shutil.disk_usage(stream_directory)
        reclaimable = stream_path.stat().st_size if stream_path.is_file() else 0
        usable = disk.free + reclaimable
        if usable >= required_bytes:
            return
        if time.monotonic() >= deadline:
            raise RuntimeError(
                "Timed out waiting for enough disk space for the next Dieharder stream."
            )
        print(
            f"Waiting for disk space: {usable / 1024**3:.2f} GiB available, "
            f"{required_bytes / 1024**3:.2f} GiB required."
        )
        time.sleep(FREE_SPACE_POLL_SECONDS)


# ------------------------------------------------------------------------------


def BuildGeneratorCommand(
    case: CampaignCase,
    generator: str,
    stream_path: Path,
    manifest_path: Path,
    *,
    device: str,
    local_size: int,
    force: bool,
) -> list[str]:
    command = [
        generator,
        "--engine",
        case.engine,
        "--seed",
        str(case.seed),
        "--stream-offset",
        str(case.stream_offset),
        "--stream-type",
        STREAM_TYPE,
        "--workers",
        str(WORKER_COUNT),
        "--samples-per-worker",
        str(SAMPLES_PER_WORKER),
        "--local-size",
        str(local_size),
        "--device",
        device,
        "--output",
        str(stream_path),
        "--manifest",
        str(manifest_path),
        "--layout",
        case.layout,
    ]
    if force:
        command.append("--force")
    return command


# ------------------------------------------------------------------------------


def BuildDieharderCommand(
    runner: str,
    dieharder: str,
    manifest_path: Path,
    summary_path: Path,
) -> list[str]:
    return [
        sys.executable,
        runner,
        "--manifest",
        str(manifest_path),
        "--dieharder",
        dieharder,
        "--summary",
        str(summary_path),
    ]


# ------------------------------------------------------------------------------


def RunCase(
    case: CampaignCase,
    *,
    generator: str,
    runner: str,
    dieharder: str,
    device: str,
    local_size: int,
    force: bool,
    keep_stream: bool,
    dry_run: bool,
) -> bool:
    stream_path, manifest_path, summary_path = CasePaths(case)

    print("GGEMS Dieharder campaign case")
    print(f"Case               : {case.stem}")
    print(f"Engine             : {case.engine}")
    print(f"Seed               : {case.seed}")
    print(f"Layout             : {case.layout}")
    print(f"Stream offset      : {case.stream_offset}")
    print()

    if not force and IsCompletedCase(case):
        print("Status             : completed result found, skipped")
        if not dry_run and stream_path.exists() and not keep_stream:
            stream_path.unlink()
        return False

    if not dry_run:
        stream_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        summary_path.parent.mkdir(parents=True, exist_ok=True)

    can_resume = (
        not force
        and stream_path.is_file()
        and manifest_path.is_file()
        and ManifestMatches(case, manifest_path)
        and stream_path.stat().st_size == STREAM_BYTE_COUNT
    )

    if can_resume:
        print("Generate           : existing stream and manifest reused")
    else:
        if not force and (stream_path.exists() or manifest_path.exists()):
            raise RuntimeError(
                f"Partial or incompatible generation artifacts for {case.stem}; "
                "use --force to regenerate them."
            )
        if not dry_run:
            WaitForFreeSpace(stream_path.parent, stream_path)

        elapsed = RunCommand(
            BuildGeneratorCommand(
                case,
                generator,
                stream_path,
                manifest_path,
                device=device,
                local_size=local_size,
                force=force,
            ),
            dry_run=dry_run,
        )
        if not dry_run:
            print(f"Generation elapsed : {elapsed:.3f} s")

    elapsed = RunCommand(
        BuildDieharderCommand(runner, dieharder, manifest_path, summary_path),
        dry_run=dry_run,
    )
    if not dry_run:
        print(f"Validation elapsed : {elapsed:.3f} s")

    if not dry_run and not keep_stream:
        stream_path.unlink(missing_ok=True)

    if not dry_run:
        print(f"Summary            : {summary_path}")
    return True


# ------------------------------------------------------------------------------


def RunAllCases(
    *,
    generator: str,
    runner: str,
    dieharder: str,
    device: str,
    local_size: int,
    force: bool,
    dry_run: bool,
) -> None:
    executed = 0
    skipped = 0

    for index, case in enumerate(CAMPAIGN_CASES, start=1):
        print()
        print("=" * 80)
        print(f"Campaign progress  : {index}/{len(CAMPAIGN_CASES)}")
        print("=" * 80)

        ran = RunCase(
            case,
            generator=generator,
            runner=runner,
            dieharder=dieharder,
            device=device,
            local_size=local_size,
            force=force,
            keep_stream=False,
            dry_run=dry_run,
        )
        executed += int(ran)
        skipped += int(not ran)

    print()
    print("GGEMS Dieharder campaign completed")
    print(f"Executed           : {executed}")
    print(f"Skipped            : {skipped}")
    print(f"Total              : {len(CAMPAIGN_CASES)}")


# ------------------------------------------------------------------------------


def main() -> int:
    args = ParseArguments()

    if args.list:
        if args.dry_run:
            raise ValueError("--dry-run is not used with --list.")
        PrintCampaign(CAMPAIGN_CASES)
        return 0

    if args.dry_run and args.case is None and not args.all:
        raise ValueError("--dry-run requires --case or --all.")
    if args.all and args.keep_stream:
        raise ValueError("--keep-stream is only valid with --case.")

    if args.case is None and not args.all:
        print("No campaign action requested. Use --list, --case CASE_ID, or --all.")
        return 0

    if args.dry_run:
        generator = str(args.generator.expanduser())
        runner = str(args.runner.expanduser())
        dieharder = args.dieharder
    else:
        generator = ResolveExecutable(args.generator, "GGEMS random stream generator")
        runner = ResolveExecutable(args.runner, "GGEMS Dieharder runner")
        dieharder = ResolveExecutable(args.dieharder, "Dieharder")

    if args.case is not None:
        _ = RunCase(
            FindCase(args.case),
            generator=generator,
            runner=runner,
            dieharder=dieharder,
            device=args.device,
            local_size=args.local_size,
            force=args.force,
            keep_stream=args.keep_stream,
            dry_run=args.dry_run,
        )
        return 0

    RunAllCases(
        generator=generator,
        runner=runner,
        dieharder=dieharder,
        device=args.device,
        local_size=args.local_size,
        force=args.force,
        dry_run=args.dry_run,
    )
    return 0


# ------------------------------------------------------------------------------

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (
        FileNotFoundError,
        json.JSONDecodeError,
        OSError,
        RuntimeError,
        subprocess.CalledProcessError,
        ValueError,
    ) as error:
        print(f"GGEMS Dieharder campaign failed:\n{error}", file=sys.stderr)
        raise SystemExit(1) from None
