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
type StreamType = Literal["raw_uint32", "uniform24_vector4"]
type Layout = Literal["interleaved", "worker_major"]

ENGINES: tuple[Engine, ...] = ("philox", "pcg32", "jkiss")

SEEDS: tuple[int, ...] = (
    741_750_298,
    161_750_683,
    632_032_812,
    3_368_369_737,
    136_035_985,
    3_392_895_365,
    1_281_213_277,
    1_200_211_620,
    1_124_626_131,
    2_179_693_345,
    1_705_829_698,
    2_374_528_848,
)

TARGETED_SEED_INDICES = (0, 5, 11)

WORKER_COUNT = 1 << 20
REFERENCE_STREAM_OFFSET = 0
SHIFTED_STREAM_OFFSET = 1 << 20
RAW_SAMPLES_PER_WORKER = 8199
UNIFORM24_SAMPLES_PER_WORKER = 10932
REFERENCE_BYTE_COUNT = WORKER_COUNT * RAW_SAMPLES_PER_WORKER * 4

# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class CampaignCase:
    case_id: str
    series: str
    engine: Engine
    seed: int
    stream_type: StreamType
    lanes_used: int | None
    layout: Layout
    stream_offset: int
    samples_per_worker: int

    @property
    def byte_count(self) -> int:
        bytes_per_sample = 4 if self.stream_type == "raw_uint32" else 3
        return WORKER_COUNT * self.samples_per_worker * bytes_per_sample


# ------------------------------------------------------------------------------


class Arguments(Protocol):
    list: bool
    case: str | None
    all: bool
    dry_run: bool
    generator: Path
    runner: Path
    rng_test: str
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
        description="Run the fixed GGEMS PractRand validation campaign."
    )

    action = parser.add_mutually_exclusive_group()
    _ = action.add_argument(
        "--list",
        action="store_true",
        help="List every fixed PractRand campaign case.",
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
        default=root / "validation/random/practrand/run_practrand.py",
        help="GGEMS PractRand single-run wrapper.",
    )
    _ = parser.add_argument(
        "--rng-test",
        default="RNG_test",
        help="PractRand RNG_test executable or path.",
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
        action="store_true",
        help="Keep the generated stream after a single-case run; not valid with --all.",
    )

    return cast(Arguments, cast(object, parser.parse_args()))


# ------------------------------------------------------------------------------


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
        seed=SEEDS[seed_index],
        stream_type=stream_type,
        lanes_used=lanes_used,
        layout=layout,
        stream_offset=stream_offset,
        samples_per_worker=samples_per_worker,
    )


# ------------------------------------------------------------------------------


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


# ------------------------------------------------------------------------------


def ValidateCampaign(cases: list[CampaignCase]) -> None:
    if len(cases) != 102:
        raise RuntimeError(f"Expected 102 PractRand cases, got {len(cases)}.")

    if len({case.case_id for case in cases}) != len(cases):
        raise RuntimeError("PractRand campaign case identifiers are not unique.")

    for case in cases:
        if case.byte_count != REFERENCE_BYTE_COUNT:
            raise RuntimeError(f"{case.case_id}: inconsistent campaign byte count.")
        if (
            case.stream_type == "uniform24_vector4"
            and case.lanes_used is not None
            and case.samples_per_worker % case.lanes_used != 0
        ):
            raise RuntimeError(f"{case.case_id}: invalid vector lane geometry.")


# ------------------------------------------------------------------------------


def PrintCampaign(cases: list[CampaignCase]) -> None:
    print("GGEMS PractRand fixed validation campaign")
    print()
    print(
        f"{'Case':<13}{'Engine':<8}{'Seed':>11}  "
        + f"{'Stream':<19}{'Lanes':>5}  {'Layout':<13}{'Offset':>10}"
    )
    print("-" * 88)

    for case in cases:
        lanes = "-" if case.lanes_used is None else str(case.lanes_used)
        print(
            f"{case.case_id:<13}{case.engine:<8}{case.seed:>11}  "
            + f"{case.stream_type:<19}{lanes:>5}  "
            + f"{case.layout:<13}{case.stream_offset:>10}"
        )

    print()
    print(f"Cases              : {len(cases)}")
    print(f"Workers per case   : {WORKER_COUNT}")
    print(f"GiB per case       : {REFERENCE_BYTE_COUNT / 1024**3:.8f}")


# ------------------------------------------------------------------------------


def FindCase(cases: list[CampaignCase], case_id: str) -> CampaignCase:
    requested = case_id.lower()
    for case in cases:
        if case.case_id.lower() == requested:
            return case
    raise ValueError(f"Unknown PractRand campaign case: {case_id}")


# ------------------------------------------------------------------------------


def CasePaths(case: CampaignCase) -> tuple[Path, Path, Path]:
    root = RepositoryRoot()
    stream = (
        root / "validation/random/streams/practrand_campaign" / f"{case.case_id}.bin"
    )
    manifest = (
        root
        / "validation/random/results/practrand/campaign/manifests"
        / f"{case.case_id}.json"
    )
    summary = (
        root / "validation/random/results/practrand/campaign" / f"{case.case_id}.json"
    )
    return stream, manifest, summary


# ------------------------------------------------------------------------------


def LoadJsonObject(path: Path) -> dict[str, object] | None:
    try:
        value = cast(object, json.loads(path.read_text(encoding="utf-8")))
    except (json.JSONDecodeError, OSError):
        return None

    if not isinstance(value, dict):
        return None

    return cast(dict[str, object], value)


# ------------------------------------------------------------------------------


def ManifestMatches(case: CampaignCase, manifest_path: Path) -> bool:
    manifest = LoadJsonObject(manifest_path)
    if manifest is None:
        return False

    random_value = manifest.get("random")
    if not isinstance(random_value, dict):
        return False
    random = cast(dict[str, object], random_value)

    engine = random.get("engine")
    if not isinstance(engine, str) or engine.lower() != case.engine:
        return False

    expected: dict[str, object] = {
        "seed": case.seed,
        "stream_offset": case.stream_offset,
        "stream_type": case.stream_type,
        "layout": case.layout,
        "worker_count": WORKER_COUNT,
        "samples_per_worker": case.samples_per_worker,
        "byte_count": case.byte_count,
    }
    if any(random.get(key) != value for key, value in expected.items()):
        return False

    return case.lanes_used is None or random.get("lanes_used") == case.lanes_used


# ------------------------------------------------------------------------------


def IsCompletedCase(case: CampaignCase) -> bool:
    _, manifest_path, summary_path = CasePaths(case)
    summary = LoadJsonObject(summary_path)
    if summary is None or not ManifestMatches(case, manifest_path):
        return False

    input_value = summary.get("input")
    output_value = summary.get("output")
    tool_value = summary.get("tool")

    if not isinstance(input_value, dict):
        return False
    if not isinstance(output_value, dict):
        return False
    if not isinstance(tool_value, dict):
        return False

    input_section = cast(dict[str, object], input_value)
    output = cast(dict[str, object], output_value)
    tool = cast(dict[str, object], tool_value)

    if tool.get("multithreaded") is not True:
        return False

    engine = input_section.get("engine")
    if not isinstance(engine, str) or engine.lower() != case.engine:
        return False

    return output.get("status") in {
        "passed_no_anomalies",
        "attention_required",
        "failed",
    }


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


def RunCommand(command: list[str], *, dry_run: bool) -> None:
    print(f"$ {shlex.join(command)}")
    if not dry_run:
        _ = subprocess.run(command, check=True)


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
        "--workers",
        str(WORKER_COUNT),
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
        command.extend(["--lanes-used", str(case.lanes_used)])
    if force:
        command.append("--force")
    return command


# ------------------------------------------------------------------------------


def BuildPractRandCommand(
    case: CampaignCase,
    runner: str,
    rng_test: str,
    manifest_path: Path,
    summary_path: Path,
) -> list[str]:
    _ = case
    return [
        sys.executable,
        runner,
        "--manifest",
        str(manifest_path),
        "--rng-test",
        rng_test,
        "--summary",
        str(summary_path),
        "--multithreaded",
    ]


# ------------------------------------------------------------------------------


def RunCase(
    case: CampaignCase,
    *,
    generator: str,
    runner: str,
    rng_test: str,
    device: str,
    local_size: int,
    force: bool,
    keep_stream: bool,
    dry_run: bool,
) -> bool:
    stream_path, manifest_path, summary_path = CasePaths(case)

    print("GGEMS PractRand campaign case")
    print(f"Case               : {case.case_id}")
    print(f"Engine             : {case.engine}")
    print(f"Seed               : {case.seed}")
    print(f"Stream type        : {case.stream_type}")
    print(f"Layout             : {case.layout}")
    print(f"Stream offset      : {case.stream_offset}")
    print(f"Samples per worker : {case.samples_per_worker}")
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
        and stream_path.stat().st_size == case.byte_count
    )

    if can_resume:
        print("Generate           : existing stream and manifest reused")
    else:
        if not force and (stream_path.exists() or manifest_path.exists()):
            raise RuntimeError(
                f"Partial or incompatible generation artifacts for {case.case_id}; "
                + "use --force to regenerate them."
            )
        RunCommand(
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

    RunCommand(
        BuildPractRandCommand(case, runner, rng_test, manifest_path, summary_path),
        dry_run=dry_run,
    )

    if not dry_run and not keep_stream:
        stream_path.unlink(missing_ok=True)

    if not dry_run:
        print(f"Summary            : {summary_path}")
    return True


# ------------------------------------------------------------------------------


def RunAllCases(
    cases: list[CampaignCase],
    *,
    generator: str,
    runner: str,
    rng_test: str,
    device: str,
    local_size: int,
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
            generator=generator,
            runner=runner,
            rng_test=rng_test,
            device=device,
            local_size=local_size,
            force=force,
            keep_stream=False,
            dry_run=dry_run,
        )
        executed += int(ran)
        skipped += int(not ran)

    print()
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
            raise ValueError("--dry-run is not used with --list.")
        PrintCampaign(cases)
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
        rng_test = args.rng_test
    else:
        generator = ResolveExecutable(args.generator, "GGEMS random stream generator")
        runner = ResolveExecutable(args.runner, "GGEMS PractRand runner")
        rng_test = ResolveExecutable(args.rng_test, "PractRand RNG_test")

    if args.case is not None:
        _ = RunCase(
            FindCase(cases, args.case),
            generator=generator,
            runner=runner,
            rng_test=rng_test,
            device=args.device,
            local_size=args.local_size,
            force=args.force,
            keep_stream=args.keep_stream,
            dry_run=args.dry_run,
        )
        return 0

    RunAllCases(
        cases,
        generator=generator,
        runner=runner,
        rng_test=rng_test,
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
        print(f"GGEMS PractRand campaign failed:\n{error}", file=sys.stderr)
        raise SystemExit(1) from None
