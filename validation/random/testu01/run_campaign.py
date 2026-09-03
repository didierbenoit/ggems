#!/usr/bin/env python3

import argparse
import json
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Literal, Protocol, cast

# ------------------------------------------------------------------------------

type Engine = Literal["philox", "pcg32", "jkiss"]
type Layout = Literal["interleaved", "worker_major"]
type Battery = Literal["smallcrush", "crush", "bigcrush"]

ENGINES: tuple[Engine, ...] = ("philox", "pcg32", "jkiss")

SEEDS: tuple[int, ...] = (
    741_750_298,
    3_396_186_067,
    1_755_654_540,
    115_123_013,
    2_769_558_782,
    1_129_027_255,
    3_783_463_024,
    2_142_931_497,
    502_399_970,
    3_156_835_739,
    1_516_304_212,
    4_170_739_981,
)

F_OFFSETS: tuple[int, ...] = (1 << 20, 1 << 24, 1 << 28, 1 << 30)

WORKER_COUNT = 1 << 20
SAMPLES_PER_WORKER: dict[Battery, int] = {
    "smallcrush": 256,
    "crush": 34_816,
    "bigcrush": 1 << 20,
}
DEFAULT_TIMEOUT_SECONDS: dict[Battery, int] = {
    "smallcrush": 2 * 60 * 60,
    "crush": 24 * 60 * 60,
    "bigcrush": 7 * 24 * 60 * 60,
}

STREAM_REQUEST_SCHEMA = "ggems_testu01_opencl_pipe_request"
STREAM_REQUEST_SCHEMA_VERSION = 1
STREAM_PROTOCOL = "ggems_testu01_1_2_3_fedora_opencl_pipe_v1"

# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class CampaignCase:
    series: str
    index: int
    engine: Engine
    battery: Battery
    seed: int
    stream_offset: int
    layout: Layout

    @property
    def stem(self) -> str:
        return f"{self.series}{self.index:02d}-{self.engine}-{self.battery}"

    @property
    def samples_per_worker(self) -> int:
        return SAMPLES_PER_WORKER[self.battery]

    @property
    def logical_word_capacity(self) -> int:
        return WORKER_COUNT * self.samples_per_worker

    @property
    def logical_byte_capacity(self) -> int:
        return self.logical_word_capacity * 4


# ------------------------------------------------------------------------------


class Arguments(Protocol):
    list: bool
    case: str | None
    all: bool
    dry_run: bool
    runner: Path
    producer: Path
    consumer: Path
    device: str
    local_size: int
    max_chunk_mib: int
    timeout_seconds: int | None
    force: bool


# ------------------------------------------------------------------------------


def RepositoryRoot() -> Path:
    return Path(__file__).resolve().parents[3]


# ------------------------------------------------------------------------------


def ParseArguments() -> Arguments:
    root = RepositoryRoot()
    parser = argparse.ArgumentParser(
        description="Run the fixed GGEMS TestU01 validation campaign."
    )

    action = parser.add_mutually_exclusive_group()
    _ = action.add_argument(
        "--list",
        action="store_true",
        help="List every fixed TestU01 campaign case.",
    )
    _ = action.add_argument(
        "--case",
        metavar="CASE_ID",
        help="Run exactly one fixed campaign case, for example A01-philox-bigcrush.",
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
        "--runner",
        type=Path,
        default=root / "validation/random/testu01/run_testu01.py",
        help="GGEMS TestU01 single-run wrapper.",
    )
    _ = parser.add_argument(
        "--producer",
        type=Path,
        default=root / "build/validation/random/ggems_random_stream_pipe_producer",
        help="GGEMS OpenCL random stream producer executable.",
    )
    _ = parser.add_argument(
        "--consumer",
        type=Path,
        default=root / "build/validation/random/testu01/ggems_testu01_consumer",
        help="GGEMS TestU01 consumer executable.",
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
        "--max-chunk-mib",
        type=int,
        default=256,
        help="Maximum OpenCL value chunk size in MiB (default: 256).",
    )
    _ = parser.add_argument(
        "--timeout-seconds",
        type=int,
        help="Override the battery-specific watchdog duration.",
    )
    _ = parser.add_argument(
        "--force",
        action="store_true",
        help="Rerun completed cases.",
    )

    return cast(Arguments, cast(object, parser.parse_args()))


# ------------------------------------------------------------------------------


def BuildCampaign() -> list[CampaignCase]:
    cases: list[CampaignCase] = []

    # A: interleaved, offset 0.
    for index, seed in enumerate(SEEDS, start=1):
        for engine in ENGINES:
            cases.append(
                CampaignCase("A", index, engine, "smallcrush", seed, 0, "interleaved")
            )
            if index <= 4:
                cases.append(
                    CampaignCase("A", index, engine, "crush", seed, 0, "interleaved")
                )
            if index == 1:
                cases.append(
                    CampaignCase("A", index, engine, "bigcrush", seed, 0, "interleaved")
                )

    # E: worker-major, offset 0, first four seeds.
    for index, seed in enumerate(SEEDS[:4], start=1):
        for engine in ENGINES:
            cases.append(
                CampaignCase("E", index, engine, "smallcrush", seed, 0, "worker_major")
            )
            if index == 1:
                cases.append(
                    CampaignCase("E", index, engine, "crush", seed, 0, "worker_major")
                )

    # F: interleaved, A01 seed, shifted stream identifiers.
    seed = SEEDS[0]
    for index, stream_offset in enumerate(F_OFFSETS, start=1):
        for engine in ENGINES:
            cases.append(
                CampaignCase(
                    "F",
                    index,
                    engine,
                    "smallcrush",
                    seed,
                    stream_offset,
                    "interleaved",
                )
            )
            if index == 1:
                cases.append(
                    CampaignCase(
                        "F",
                        index,
                        engine,
                        "crush",
                        seed,
                        stream_offset,
                        "interleaved",
                    )
                )

    return cases


# ------------------------------------------------------------------------------


def ValidateCampaign(cases: list[CampaignCase]) -> None:
    if len(cases) != 81:
        raise RuntimeError(f"Expected 81 TestU01 battery runs, got {len(cases)}.")
    if len({case.stem for case in cases}) != len(cases):
        raise RuntimeError("TestU01 campaign case identifiers are not unique.")


# ------------------------------------------------------------------------------


def PrintCampaign(cases: list[CampaignCase]) -> None:
    print("GGEMS TestU01 fixed validation campaign")
    print()
    print(
        f"{'Case':<29}{'Engine':<8}{'Battery':<11}{'Seed':>11}  "
        f"{'Layout':<13}{'Offset':>11}  {'D':>8}"
    )
    print("-" * 98)

    for case in cases:
        print(
            f"{case.stem:<29}{case.engine:<8}{case.battery:<11}{case.seed:>11}  "
            f"{case.layout:<13}{case.stream_offset:>11}  "
            f"{case.samples_per_worker:>8}"
        )

    counts = {
        battery: sum(case.battery == battery for case in cases)
        for battery in ("smallcrush", "crush", "bigcrush")
    }

    print()
    print(f"Cases              : {len(cases)}")
    print(f"SmallCrush         : {counts['smallcrush']}")
    print(f"Crush              : {counts['crush']}")
    print(f"BigCrush           : {counts['bigcrush']}")
    print(f"Workers per case   : {WORKER_COUNT}")


# ------------------------------------------------------------------------------


def FindCase(cases: list[CampaignCase], case_id: str) -> CampaignCase:
    requested = case_id.lower()
    for case in cases:
        if case.stem.lower() == requested:
            return case
    raise ValueError(f"Unknown TestU01 campaign case: {case_id}")


# ------------------------------------------------------------------------------


def CasePaths(case: CampaignCase) -> tuple[Path, Path]:
    root = RepositoryRoot()
    campaign = root / "validation/random/results/testu01/campaign"
    request = campaign / "requests" / f"{case.stem}.json"
    summary = campaign / f"{case.stem}.json"
    return request, summary


# ------------------------------------------------------------------------------


def LoadJsonObject(path: Path) -> dict[str, object] | None:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return None
    return value if isinstance(value, dict) else None


# ------------------------------------------------------------------------------


def IsCompletedCase(case: CampaignCase) -> bool:
    _, summary_path = CasePaths(case)
    summary = LoadJsonObject(summary_path)
    if summary is None:
        return False

    input_section = summary.get("input")
    output = summary.get("output")
    if not isinstance(input_section, dict) or not isinstance(output, dict):
        return False

    status = output.get("status")
    if status not in {
        "statistically_complete_no_testu01_suspects",
        "statistically_complete_with_testu01_suspects",
    }:
        return False

    engine = input_section.get("engine")
    if not isinstance(engine, str) or engine.lower() != case.engine:
        return False

    return (
        input_section.get("seed") == case.seed
        and input_section.get("stream_offset") == case.stream_offset
        and input_section.get("layout") == case.layout
        and input_section.get("worker_count") == WORKER_COUNT
    )


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


def BuildRequest(
    case: CampaignCase,
    *,
    device: str,
    local_size: int,
    max_chunk_mib: int,
) -> dict[str, object]:
    return {
        "schema": STREAM_REQUEST_SCHEMA,
        "schema_version": STREAM_REQUEST_SCHEMA_VERSION,
        "case_id": case.stem,
        "transport": "opencl_pipe",
        "protocol": STREAM_PROTOCOL,
        "random": {
            "engine": case.engine,
            "seed": case.seed,
            "stream_offset": case.stream_offset,
            "stream_type": "raw_uint32",
            "sample_bits": 32,
            "layout": case.layout,
            "worker_count": WORKER_COUNT,
            "samples_per_worker": case.samples_per_worker,
            "logical_word_capacity": case.logical_word_capacity,
            "logical_byte_capacity": case.logical_byte_capacity,
        },
        "opencl": {
            "max_chunk_mib": max_chunk_mib,
            "local_size": local_size,
            "device_selector": device,
        },
        "encoding": "canonical_little_endian_uint32",
    }


# ------------------------------------------------------------------------------


def WriteRequest(
    path: Path,
    request: dict[str, object],
) -> None:
    # Requests are generated execution artifacts under results/. They are
    # recreated from the current campaign definition and runtime options for
    # every execution and are never treated as source-controlled state.
    path.parent.mkdir(parents=True, exist_ok=True)
    _ = path.write_text(json.dumps(request, indent=2) + "\n", encoding="utf-8")


# ------------------------------------------------------------------------------


def BuildTestU01Command(
    case: CampaignCase,
    runner: str,
    producer: str,
    consumer: str,
    request_path: Path,
    summary_path: Path,
    timeout_seconds: int,
) -> list[str]:
    return [
        sys.executable,
        runner,
        "--stream-request",
        str(request_path),
        "--battery",
        case.battery,
        "--producer",
        producer,
        "--consumer",
        consumer,
        "--summary",
        str(summary_path),
        "--timeout-seconds",
        str(timeout_seconds),
    ]


# ------------------------------------------------------------------------------


def RunCommand(command: list[str], *, dry_run: bool) -> None:
    print(f"$ {shlex.join(command)}")
    if not dry_run:
        subprocess.run(command, check=True)


# ------------------------------------------------------------------------------


def RunCase(
    case: CampaignCase,
    *,
    runner: str,
    producer: str,
    consumer: str,
    device: str,
    local_size: int,
    max_chunk_mib: int,
    timeout_override: int | None,
    force: bool,
    dry_run: bool,
) -> bool:
    request_path, summary_path = CasePaths(case)
    timeout_seconds = (
        timeout_override
        if timeout_override is not None
        else DEFAULT_TIMEOUT_SECONDS[case.battery]
    )

    print("GGEMS TestU01 campaign case")
    print(f"Case               : {case.stem}")
    print(f"Engine             : {case.engine}")
    print(f"Battery            : {case.battery}")
    print(f"Seed               : {case.seed}")
    print(f"Layout             : {case.layout}")
    print(f"Stream offset      : {case.stream_offset}")
    print(f"Worker count       : {WORKER_COUNT}")
    print(f"Samples per worker : {case.samples_per_worker}")
    print(f"Logical GiB        : {case.logical_byte_capacity / 1024**3:.2f}")
    print()

    if not force and IsCompletedCase(case):
        print("Status             : completed result found, skipped")
        return False

    request = BuildRequest(
        case,
        device=device,
        local_size=local_size,
        max_chunk_mib=max_chunk_mib,
    )

    if not dry_run:
        summary_path.parent.mkdir(parents=True, exist_ok=True)
        WriteRequest(request_path, request)

    command = BuildTestU01Command(
        case,
        runner,
        producer,
        consumer,
        request_path,
        summary_path,
        timeout_seconds,
    )
    RunCommand(command, dry_run=dry_run)

    if not dry_run:
        print(f"Request            : {request_path}")
        print(f"Summary            : {summary_path}")
    return True


# ------------------------------------------------------------------------------


def RunAllCases(
    cases: list[CampaignCase],
    *,
    runner: str,
    producer: str,
    consumer: str,
    device: str,
    local_size: int,
    max_chunk_mib: int,
    timeout_override: int | None,
    force: bool,
    dry_run: bool,
) -> None:
    executed = 0
    skipped = 0

    for index, case in enumerate(cases, start=1):
        print()
        print("=" * 80)
        print(f"Campaign progress  : {index}/{len(cases)}")
        print("=" * 80)

        ran = RunCase(
            case,
            runner=runner,
            producer=producer,
            consumer=consumer,
            device=device,
            local_size=local_size,
            max_chunk_mib=max_chunk_mib,
            timeout_override=timeout_override,
            force=force,
            dry_run=dry_run,
        )
        executed += int(ran)
        skipped += int(not ran)

    print()
    print("GGEMS TestU01 campaign completed")
    print(f"Executed           : {executed}")
    print(f"Skipped            : {skipped}")
    print(f"Total              : {len(cases)}")


# ------------------------------------------------------------------------------


def main() -> int:
    args = ParseArguments()
    cases = BuildCampaign()
    ValidateCampaign(cases)

    if args.list:
        if args.dry_run:
            raise ValueError("--dry-run is not used with --list.")
        PrintCampaign(cases)
        return 0

    if args.dry_run and args.case is None and not args.all:
        raise ValueError("--dry-run requires --case or --all.")
    if args.local_size <= 0:
        raise ValueError("--local-size must be positive.")
    if args.max_chunk_mib <= 0:
        raise ValueError("--max-chunk-mib must be positive.")
    if args.timeout_seconds is not None and args.timeout_seconds <= 0:
        raise ValueError("--timeout-seconds must be positive.")

    if args.case is None and not args.all:
        print("No campaign action requested. Use --list, --case CASE_ID, or --all.")
        return 0

    if args.dry_run:
        runner = str(args.runner.expanduser())
        producer = str(args.producer.expanduser())
        consumer = str(args.consumer.expanduser())
    else:
        runner = ResolveExecutable(args.runner, "GGEMS TestU01 runner")
        producer = ResolveExecutable(args.producer, "GGEMS OpenCL stream producer")
        consumer = ResolveExecutable(args.consumer, "GGEMS TestU01 consumer")

    if args.case is not None:
        _ = RunCase(
            FindCase(cases, args.case),
            runner=runner,
            producer=producer,
            consumer=consumer,
            device=args.device,
            local_size=args.local_size,
            max_chunk_mib=args.max_chunk_mib,
            timeout_override=args.timeout_seconds,
            force=args.force,
            dry_run=args.dry_run,
        )
        return 0

    RunAllCases(
        cases,
        runner=runner,
        producer=producer,
        consumer=consumer,
        device=args.device,
        local_size=args.local_size,
        max_chunk_mib=args.max_chunk_mib,
        timeout_override=args.timeout_seconds,
        force=args.force,
        dry_run=args.dry_run,
    )
    return 0


# ------------------------------------------------------------------------------

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (
        FileExistsError,
        FileNotFoundError,
        json.JSONDecodeError,
        OSError,
        RuntimeError,
        subprocess.CalledProcessError,
        ValueError,
    ) as error:
        print(f"GGEMS TestU01 campaign failed:\n{error}", file=sys.stderr)
        raise SystemExit(1) from None
