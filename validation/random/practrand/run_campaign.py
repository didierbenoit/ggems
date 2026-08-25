#!/usr/bin/env python3

import argparse
import json
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Literal


Engine = Literal["philox", "pcg32", "jkiss"]
StreamType = Literal["raw_uint32", "uniform24_vector4"]
Layout = Literal["interleaved", "worker_major"]


ENGINES: tuple[Engine, ...] = ("philox", "pcg32", "jkiss")

SEEDS: tuple[int, ...] = (
    741750298,
    161750683,
    632032812,
    3368369737,
    136035985,
    3392895365,
    1281213277,
    1200211620,
    1124626131,
    2179693345,
    1705829698,
    2374528848,
)

TARGETED_SEED_INDICES = (0, 5, 11)

WORKER_COUNT = 1 << 20
REFERENCE_STREAM_OFFSET = 0
SHIFTED_STREAM_OFFSET = 1 << 20

# Smallest common stream size at or above 32 GiB while preserving equal
# byte counts and vector4 lane divisibility for lanes 2, 3, and 4.
RAW_SAMPLES_PER_WORKER = 8199
UNIFORM24_SAMPLES_PER_WORKER = 10932

REFERENCE_BYTE_COUNT = WORKER_COUNT * RAW_SAMPLES_PER_WORKER * 4


@dataclass(frozen=True, slots=True)
class CampaignCase:
    case_id: str
    series: str
    engine: Engine
    seed_index: int
    seed: int
    stream_type: StreamType
    lanes_used: int | None
    layout: Layout
    stream_offset: int
    worker_count: int
    samples_per_worker: int

    @property
    def byte_count(self) -> int:
        bytes_per_sample = 4 if self.stream_type == "raw_uint32" else 3
        return self.worker_count * self.samples_per_worker * bytes_per_sample


def ParseArguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run the fixed GGEMS PractRand validation campaign."
    )

    action = parser.add_mutually_exclusive_group()
    action.add_argument(
        "--list",
        action="store_true",
        help="List every fixed PractRand campaign case without running anything.",
    )
    action.add_argument(
        "--case",
        metavar="CASE_ID",
        help="Run exactly one fixed campaign case, for example A01-philox.",
    )
    action.add_argument(
        "--all",
        action="store_true",
        help=(
            "Run all fixed campaign cases sequentially. Completed cases are "
            "skipped unless --force is used."
        ),
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print commands without executing them.",
    )
    parser.add_argument(
        "--generator",
        type=Path,
        help=(
            "Path to ggems_random_stream_generator. "
            "Defaults to build/validation/random/ggems_random_stream_generator."
        ),
    )
    parser.add_argument(
        "--rng-test",
        default="RNG_test",
        help="PractRand RNG_test executable or path (default: RNG_test).",
    )
    parser.add_argument(
        "--device",
        default="gpu",
        help="GGEMS device selector passed to the stream generator (default: gpu).",
    )
    parser.add_argument(
        "--local-size",
        type=int,
        default=64,
        help="OpenCL local work-group size (default: 64).",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help=(
            "Overwrite files from earlier executions. With --all, rerun even "
            "completed cases."
        ),
    )
    parser.add_argument(
        "--keep-stream",
        action="store_true",
        help=(
            "Keep the generated binary stream after a completed single-case "
            "PractRand run. Not valid with --all."
        ),
    )

    return parser.parse_args()


def MakeCase(
    series: str,
    seed_index: int,
    engine: Engine,
    *,
    stream_type: StreamType,
    lanes_used: int | None,
    layout: Layout,
    stream_offset: int,
    samples_per_worker: int,
) -> CampaignCase:
    return CampaignCase(
        case_id=f"{series}{seed_index + 1:02d}-{engine}",
        series=series,
        engine=engine,
        seed_index=seed_index + 1,
        seed=SEEDS[seed_index],
        stream_type=stream_type,
        lanes_used=lanes_used,
        layout=layout,
        stream_offset=stream_offset,
        worker_count=WORKER_COUNT,
        samples_per_worker=samples_per_worker,
    )


def BuildCampaign() -> list[CampaignCase]:
    cases: list[CampaignCase] = []

    for seed_index in range(len(SEEDS)):
        for engine in ENGINES:
            cases.append(
                MakeCase(
                    "A",
                    seed_index,
                    engine,
                    stream_type="raw_uint32",
                    lanes_used=None,
                    layout="interleaved",
                    stream_offset=REFERENCE_STREAM_OFFSET,
                    samples_per_worker=RAW_SAMPLES_PER_WORKER,
                )
            )

    for seed_index in range(len(SEEDS)):
        for engine in ENGINES:
            cases.append(
                MakeCase(
                    "B",
                    seed_index,
                    engine,
                    stream_type="uniform24_vector4",
                    lanes_used=2,
                    layout="interleaved",
                    stream_offset=REFERENCE_STREAM_OFFSET,
                    samples_per_worker=UNIFORM24_SAMPLES_PER_WORKER,
                )
            )

    for seed_index in TARGETED_SEED_INDICES:
        for engine in ENGINES:
            cases.append(
                MakeCase(
                    "C",
                    seed_index,
                    engine,
                    stream_type="uniform24_vector4",
                    lanes_used=3,
                    layout="interleaved",
                    stream_offset=REFERENCE_STREAM_OFFSET,
                    samples_per_worker=UNIFORM24_SAMPLES_PER_WORKER,
                )
            )

    for seed_index in TARGETED_SEED_INDICES:
        for engine in ENGINES:
            cases.append(
                MakeCase(
                    "D",
                    seed_index,
                    engine,
                    stream_type="uniform24_vector4",
                    lanes_used=4,
                    layout="interleaved",
                    stream_offset=REFERENCE_STREAM_OFFSET,
                    samples_per_worker=UNIFORM24_SAMPLES_PER_WORKER,
                )
            )

    for seed_index in TARGETED_SEED_INDICES:
        for engine in ENGINES:
            cases.append(
                MakeCase(
                    "E",
                    seed_index,
                    engine,
                    stream_type="raw_uint32",
                    lanes_used=None,
                    layout="worker_major",
                    stream_offset=REFERENCE_STREAM_OFFSET,
                    samples_per_worker=RAW_SAMPLES_PER_WORKER,
                )
            )

    for engine in ENGINES:
        cases.append(
            MakeCase(
                "F",
                0,
                engine,
                stream_type="raw_uint32",
                lanes_used=None,
                layout="interleaved",
                stream_offset=SHIFTED_STREAM_OFFSET,
                samples_per_worker=RAW_SAMPLES_PER_WORKER,
            )
        )

    return cases


def ValidateCampaign(cases: list[CampaignCase]) -> None:
    if len(cases) != 102:
        raise RuntimeError(f"Expected 102 campaign cases, got {len(cases)}.")

    case_ids = [case.case_id for case in cases]
    if len(case_ids) != len(set(case_ids)):
        raise RuntimeError("Campaign case identifiers are not unique.")

    for case in cases:
        if case.byte_count != REFERENCE_BYTE_COUNT:
            raise RuntimeError(
                f"{case.case_id}: expected {REFERENCE_BYTE_COUNT} bytes, "
                f"got {case.byte_count}."
            )

        if case.stream_type == "uniform24_vector4":
            if case.lanes_used not in (2, 3, 4):
                raise RuntimeError(
                    f"{case.case_id}: invalid lanes_used={case.lanes_used}."
                )

            if case.samples_per_worker % case.lanes_used != 0:
                raise RuntimeError(
                    f"{case.case_id}: samples_per_worker must be divisible "
                    f"by lanes_used."
                )


def FormatLanes(lanes_used: int | None) -> str:
    return "-" if lanes_used is None else str(lanes_used)


def PrintCampaign(cases: list[CampaignCase]) -> None:
    print("GGEMS PractRand fixed validation campaign")
    print()
    print(
        f"{'Case':<13}"
        f"{'Engine':<8}"
        f"{'Seed':>11}  "
        f"{'Stream':<19}"
        f"{'Lanes':>5}  "
        f"{'Layout':<13}"
        f"{'Offset':>8}"
    )
    print("-" * 82)

    for case in cases:
        print(
            f"{case.case_id:<13}"
            f"{case.engine:<8}"
            f"{case.seed:>11}  "
            f"{case.stream_type:<19}"
            f"{FormatLanes(case.lanes_used):>5}  "
            f"{case.layout:<13}"
            f"{case.stream_offset:>8}"
        )

    series_counts = {
        series: sum(case.series == series for case in cases)
        for series in ("A", "B", "C", "D", "E", "F")
    }

    total_bytes = sum(case.byte_count for case in cases)

    print()
    print("Series:")
    print(f"  A  raw_uint32 / interleaved / 12 seeds       : {series_counts['A']:3d}")
    print(f"  B  uniform24_vector4 L2 / interleaved        : {series_counts['B']:3d}")
    print(f"  C  uniform24_vector4 L3 / targeted seeds     : {series_counts['C']:3d}")
    print(f"  D  uniform24_vector4 L4 / targeted seeds     : {series_counts['D']:3d}")
    print(f"  E  raw_uint32 / worker_major / targeted seeds: {series_counts['E']:3d}")
    print(f"  F  raw_uint32 / shifted stream IDs           : {series_counts['F']:3d}")
    print()
    print(f"Workers per case       : {WORKER_COUNT}")
    print(f"Bytes per case         : {REFERENCE_BYTE_COUNT}")
    print(f"MiB per case           : {REFERENCE_BYTE_COUNT / 1024**2:.0f}")
    print(f"GiB per case           : {REFERENCE_BYTE_COUNT / 1024**3:.8f}")
    print(f"Campaign cases         : {len(cases)}")
    print(f"Total PractRand input  : {total_bytes / 1024**3:.6f} GiB")


def FindCase(cases: list[CampaignCase], case_id: str) -> CampaignCase:
    requested = case_id.lower()

    for case in cases:
        if case.case_id.lower() == requested:
            return case

    raise ValueError(f"Unknown PractRand campaign case: {case_id}")


def ProjectRoot() -> Path:
    return Path(__file__).resolve().parents[3]


def CasePaths(case: CampaignCase) -> tuple[Path, Path, Path]:
    project_root = ProjectRoot()
    stream_path = (
        project_root
        / "validation"
        / "random"
        / "streams"
        / "practrand_campaign"
        / f"{case.case_id}.bin"
    )
    manifest_path = (
        project_root
        / "validation"
        / "random"
        / "results"
        / "practrand"
        / "campaign"
        / "manifests"
        / f"{case.case_id}.json"
    )
    summary_path = (
        project_root
        / "validation"
        / "random"
        / "results"
        / "practrand"
        / "campaign"
        / f"{case.case_id}.json"
    )

    return stream_path, manifest_path, summary_path


def LoadJsonObject(path: Path) -> dict[str, object] | None:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return None

    return value if isinstance(value, dict) else None


def IsCompletedCase(case: CampaignCase) -> bool:
    _, manifest_path, summary_path = CasePaths(case)

    if not manifest_path.is_file() or not summary_path.is_file():
        return False

    manifest = LoadJsonObject(manifest_path)
    summary = LoadJsonObject(summary_path)

    if manifest is None or summary is None:
        return False

    random = manifest.get("random")
    tool = summary.get("tool")
    input_section = summary.get("input")
    output = summary.get("output")

    sections = (random, tool, input_section, output)
    if not all(isinstance(section, dict) for section in sections):
        return False

    assert isinstance(random, dict)
    assert isinstance(tool, dict)
    assert isinstance(input_section, dict)
    assert isinstance(output, dict)

    engine = random.get("engine")
    summary_engine = input_section.get("engine")

    if not isinstance(engine, str) or engine.lower() != case.engine:
        return False

    if not isinstance(summary_engine, str) or summary_engine.lower() != case.engine:
        return False

    expected_random = {
        "seed": case.seed,
        "stream_offset": case.stream_offset,
        "stream_type": case.stream_type,
        "layout": case.layout,
        "worker_count": case.worker_count,
        "samples_per_worker": case.samples_per_worker,
        "byte_count": case.byte_count,
    }

    for key, value in expected_random.items():
        if random.get(key) != value or input_section.get(key) != value:
            return False

    if case.lanes_used is not None and random.get("lanes_used") != case.lanes_used:
        return False

    if tool.get("multithreaded") is not True:
        return False

    return output.get("status") in {
        "passed_no_anomalies",
        "attention_required",
        "failed",
    }


def CaseHasAnyOutput(case: CampaignCase) -> bool:
    return any(path.exists() for path in CasePaths(case))


def ResolveGenerator(project_root: Path, requested: Path | None) -> Path:
    path = (
        requested.expanduser()
        if requested is not None
        else project_root
        / "build"
        / "validation"
        / "random"
        / "ggems_random_stream_generator"
    )

    return path.resolve()


def ResolvePractRandExecutable(executable: str) -> str:
    candidate = Path(executable).expanduser()

    if candidate.is_file():
        return str(candidate.resolve())

    resolved = shutil.which(executable)
    if resolved is not None:
        return str(Path(resolved).resolve())

    return executable


def FormatCommand(command: list[str]) -> str:
    import shlex

    return shlex.join(command)


def RunCommand(command: list[str], *, dry_run: bool) -> None:
    print(f"$ {FormatCommand(command)}")

    if dry_run:
        return

    subprocess.run(command, check=True)


def RunCase(
    case: CampaignCase,
    *,
    generator: Path,
    rng_test: str,
    device: str,
    local_size: int,
    force: bool,
    keep_stream: bool,
    dry_run: bool,
) -> None:
    project_root = ProjectRoot()
    practrand_runner = (
        project_root / "validation" / "random" / "practrand" / "run_practrand.py"
    ).resolve()
    stream_path, manifest_path, summary_path = CasePaths(case)

    if not dry_run:
        if not generator.is_file():
            raise FileNotFoundError(
                f"GGEMS random stream generator not found: {generator}"
            )

        if not practrand_runner.is_file():
            raise FileNotFoundError(
                f"GGEMS PractRand runner not found: {practrand_runner}"
            )

        existing = [
            path
            for path in (stream_path, manifest_path, summary_path)
            if path.exists()
        ]

        if existing and not force:
            paths = "\n".join(f"  {path}" for path in existing)
            raise FileExistsError(
                "Campaign output already exists. Use --force to overwrite:\n"
                f"{paths}"
            )

        stream_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        summary_path.parent.mkdir(parents=True, exist_ok=True)

    generator_command = [
        str(generator),
        "--engine",
        case.engine,
        "--seed",
        str(case.seed),
        "--stream-offset",
        str(case.stream_offset),
        "--workers",
        str(case.worker_count),
        "--samples-per-worker",
        str(case.samples_per_worker),
        "--local-size",
        str(local_size),
        "--device",
        device,
        "--layout",
        case.layout,
        "--stream-type",
        case.stream_type,
        "--output",
        str(stream_path),
        "--manifest",
        str(manifest_path),
    ]

    if case.lanes_used is not None:
        generator_command.extend(["--lanes-used", str(case.lanes_used)])

    if force:
        generator_command.append("--force")

    practrand_command = [
        sys.executable,
        str(practrand_runner),
        "--manifest",
        str(manifest_path),
        "--rng-test",
        ResolvePractRandExecutable(rng_test),
        "--summary",
        str(summary_path),
        "--multithreaded",
    ]

    print("GGEMS PractRand campaign case")
    print(f"Case               : {case.case_id}")
    print(f"Engine             : {case.engine}")
    print(f"Seed               : {case.seed}")
    print(f"Stream type        : {case.stream_type}")
    print(f"Lanes used         : {FormatLanes(case.lanes_used)}")
    print(f"Layout             : {case.layout}")
    print(f"Stream offset      : {case.stream_offset}")
    print(f"Worker count       : {case.worker_count}")
    print(f"Samples per worker : {case.samples_per_worker}")
    print(f"Bytes              : {case.byte_count}")
    print(f"GiB                : {case.byte_count / 1024**3:.8f}")
    print()
    print("Generate:")
    RunCommand(generator_command, dry_run=dry_run)
    print()
    print("PractRand:")
    RunCommand(practrand_command, dry_run=dry_run)

    if not dry_run and not keep_stream:
        stream_path.unlink()

    if not dry_run:
        print()
        if keep_stream:
            print(f"Stream   : {stream_path}")
        else:
            print(f"Stream   : removed ({stream_path})")
        print(f"Manifest : {manifest_path}")
        print(f"Summary  : {summary_path}")


def RunAllCases(
    cases: list[CampaignCase],
    *,
    generator: Path,
    rng_test: str,
    device: str,
    local_size: int,
    force: bool,
    dry_run: bool,
) -> None:
    executed = 0
    skipped = 0

    print("GGEMS PractRand full validation campaign")
    print(f"Cases              : {len(cases)}")
    print(f"GiB per case       : {REFERENCE_BYTE_COUNT / 1024**3:.8f}")
    total_gib = sum(case.byte_count for case in cases) / 1024**3

    print(f"Total input        : {total_gib:.6f} GiB")
    print("Stream cleanup     : enabled")
    print("Resume             : enabled")

    for index, case in enumerate(cases, start=1):
        print()
        print("=" * 80)
        print(f"Campaign progress  : {index}/{len(cases)}")
        print("=" * 80)

        if not force and IsCompletedCase(case):
            stream_path, _, _ = CasePaths(case)

            if not dry_run and stream_path.exists():
                stream_path.unlink()

            print(f"Case               : {case.case_id}")
            print("Status             : completed result found, skipped")
            skipped += 1
            continue

        case_force = force or CaseHasAnyOutput(case)

        RunCase(
            case,
            generator=generator,
            rng_test=rng_test,
            device=device,
            local_size=local_size,
            force=case_force,
            keep_stream=False,
            dry_run=dry_run,
        )
        executed += 1

    print()
    print("=" * 80)
    print("GGEMS PractRand campaign completed")
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
            raise ValueError("--dry-run requires --case or --all.")
        PrintCampaign(cases)
        return 0

    generator = ResolveGenerator(ProjectRoot(), args.generator)

    if args.case is not None:
        case = FindCase(cases, args.case)
        RunCase(
            case,
            generator=generator,
            rng_test=args.rng_test,
            device=args.device,
            local_size=args.local_size,
            force=args.force,
            keep_stream=args.keep_stream,
            dry_run=args.dry_run,
        )
        return 0

    if args.all:
        if args.keep_stream:
            raise ValueError(
                "--keep-stream cannot be used with --all; run an individual "
                "case when a stream must be retained."
            )

        RunAllCases(
            cases,
            generator=generator,
            rng_test=args.rng_test,
            device=args.device,
            local_size=args.local_size,
            force=args.force,
            dry_run=args.dry_run,
        )
        return 0

    if args.dry_run:
        raise ValueError("--dry-run requires --case or --all.")

    print("No campaign action requested. Use --list, --case CASE_ID, or --all.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (
        FileExistsError,
        FileNotFoundError,
        RuntimeError,
        subprocess.CalledProcessError,
        ValueError,
    ) as error:
        print(f"GGEMS PractRand campaign failed:\n{error}")
        raise SystemExit(1) from None
