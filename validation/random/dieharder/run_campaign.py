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

ENGINE_DISPLAY_NAMES: dict[str, str] = {
    "philox": "Philox",
    "pcg32": "PCG32",
    "jkiss": "JKISS",
}

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
    generator: Path
    runner: Path
    dieharder: str
    device: str
    keep_streams: bool
    force: bool
    dry_run: bool


# ------------------------------------------------------------------------------


def RepositoryRoot() -> Path:
    return Path(__file__).resolve().parents[3]


# ------------------------------------------------------------------------------


def ParseArguments() -> Arguments:
    repository_root = RepositoryRoot()

    parser = argparse.ArgumentParser(
        description="Run the fixed 13-case GGEMS Dieharder reference campaign."
    )

    _ = parser.add_argument(
        "--generator",
        type=Path,
        default=repository_root
        / "build"
        / "validation"
        / "random"
        / "ggems_random_stream_generator",
        help="GGEMS random stream generator executable.",
    )
    _ = parser.add_argument(
        "--runner",
        type=Path,
        default=repository_root
        / "validation"
        / "random"
        / "dieharder"
        / "run_dieharder.py",
        help="GGEMS Dieharder single-run wrapper.",
    )
    _ = parser.add_argument(
        "--dieharder",
        default="dieharder",
        help="Dieharder executable or path passed to run_dieharder.py.",
    )
    _ = parser.add_argument(
        "--device",
        default="gpu",
        help="GGEMS device selector passed to the random stream generator.",
    )
    _ = parser.add_argument(
        "--keep-streams",
        action="store_true",
        help="Keep temporary 256 GB raw streams after completed runs.",
    )
    _ = parser.add_argument(
        "--force",
        action="store_true",
        help="Rerun completed cases and overwrite stale generation artifacts.",
    )
    _ = parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the fixed campaign plan without generating or testing streams.",
    )

    return cast(Arguments, cast(object, parser.parse_args()))


# ------------------------------------------------------------------------------


def LoadJsonObject(path: Path) -> dict[str, object]:
    parsed = cast(object, json.loads(path.read_text(encoding="utf-8")))

    if not isinstance(parsed, dict):
        raise TypeError(f"Expected a JSON object in {path}.")

    raw_mapping = cast(dict[object, object], parsed)
    result: dict[str, object] = {}

    for key, value in raw_mapping.items():
        if not isinstance(key, str):
            raise TypeError(f"Expected string JSON keys in {path}.")
        result[key] = value

    return result


# ------------------------------------------------------------------------------


def RequireObject(
    mapping: dict[str, object],
    key: str,
    *,
    source: Path,
) -> dict[str, object]:
    value = mapping.get(key)

    if not isinstance(value, dict):
        raise TypeError(f"Expected object field '{key}' in {source}.")

    raw_mapping = cast(dict[object, object], value)
    result: dict[str, object] = {}

    for child_key, child_value in raw_mapping.items():
        if not isinstance(child_key, str):
            raise TypeError(f"Expected string JSON keys in field '{key}' of {source}.")
        result[child_key] = child_value

    return result


# ------------------------------------------------------------------------------


def RequireString(
    mapping: dict[str, object],
    key: str,
    *,
    source: Path,
) -> str:
    value = mapping.get(key)

    if not isinstance(value, str):
        raise TypeError(f"Expected string field '{key}' in {source}.")

    return value


# ------------------------------------------------------------------------------


def RequireInt(
    mapping: dict[str, object],
    key: str,
    *,
    source: Path,
) -> int:
    value = mapping.get(key)

    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"Expected integer field '{key}' in {source}.")

    return value


# ------------------------------------------------------------------------------


def ValidateManifestForCase(path: Path, case: CampaignCase) -> None:
    if not path.is_file():
        raise FileNotFoundError(f"Campaign manifest not found: {path}")

    root = LoadJsonObject(path)
    random = RequireObject(root, "random", source=path)
    expected_engine = ENGINE_DISPLAY_NAMES[case.engine]

    if RequireString(random, "engine", source=path) != expected_engine:
        raise ValueError(f"Manifest engine mismatch for {case.stem}: {path}")
    if RequireInt(random, "seed", source=path) != case.seed:
        raise ValueError(f"Manifest seed mismatch for {case.stem}: {path}")
    if RequireInt(random, "stream_offset", source=path) != case.stream_offset:
        raise ValueError(f"Manifest stream_offset mismatch for {case.stem}: {path}")
    if RequireString(random, "stream_type", source=path) != STREAM_TYPE:
        raise ValueError(f"Manifest stream_type mismatch for {case.stem}: {path}")
    if RequireString(random, "layout", source=path) != case.layout:
        raise ValueError(f"Manifest layout mismatch for {case.stem}: {path}")
    if RequireInt(random, "worker_count", source=path) != WORKER_COUNT:
        raise ValueError(f"Manifest worker_count mismatch for {case.stem}: {path}")
    if RequireInt(random, "samples_per_worker", source=path) != SAMPLES_PER_WORKER:
        raise ValueError(
            f"Manifest samples_per_worker mismatch for {case.stem}: {path}"
        )
    if RequireInt(random, "byte_count", source=path) != STREAM_BYTE_COUNT:
        raise ValueError(f"Manifest byte_count mismatch for {case.stem}: {path}")


# ------------------------------------------------------------------------------


def ValidateCompletedSummary(path: Path, case: CampaignCase) -> None:
    if not path.is_file():
        raise FileNotFoundError(f"Campaign summary not found: {path}")

    root = LoadJsonObject(path)
    tool = RequireObject(root, "tool", source=path)
    input_section = RequireObject(root, "input", source=path)
    output = RequireObject(root, "output", source=path)

    if RequireString(tool, "version", source=path) != DIEHARDER_REFERENCE_VERSION:
        raise ValueError(f"Dieharder version mismatch in completed case {case.stem}.")

    expected_engine = ENGINE_DISPLAY_NAMES[case.engine]

    if RequireString(input_section, "engine", source=path) != expected_engine:
        raise ValueError(f"Summary engine mismatch for completed case {case.stem}.")
    if RequireInt(input_section, "seed", source=path) != case.seed:
        raise ValueError(f"Summary seed mismatch for completed case {case.stem}.")
    if RequireInt(input_section, "stream_offset", source=path) != case.stream_offset:
        raise ValueError(
            f"Summary stream_offset mismatch for completed case {case.stem}."
        )
    if RequireString(input_section, "stream_type", source=path) != STREAM_TYPE:
        raise ValueError(
            f"Summary stream_type mismatch for completed case {case.stem}."
        )
    if RequireString(input_section, "layout", source=path) != case.layout:
        raise ValueError(f"Summary layout mismatch for completed case {case.stem}.")
    if RequireInt(input_section, "worker_count", source=path) != WORKER_COUNT:
        raise ValueError(
            f"Summary worker_count mismatch for completed case {case.stem}."
        )
    if (
        RequireInt(input_section, "samples_per_worker", source=path)
        != SAMPLES_PER_WORKER
    ):
        raise ValueError(
            f"Summary samples_per_worker mismatch for completed case {case.stem}."
        )
    if RequireInt(input_section, "byte_count", source=path) != STREAM_BYTE_COUNT:
        raise ValueError(f"Summary byte_count mismatch for completed case {case.stem}.")

    if RequireString(output, "status", source=path) != "completed":
        raise ValueError(f"Summary status is not completed for {case.stem}.")
    if RequireInt(output, "assessment_count", source=path) != EXPECTED_ASSESSMENT_COUNT:
        raise ValueError(
            f"Summary assessment count mismatch for completed case {case.stem}."
        )
    if RequireString(output, "stderr", source=path):
        raise ValueError(f"Completed case {case.stem} contains non-empty stderr.")


# ------------------------------------------------------------------------------


def ResolveExecutable(path: Path, description: str) -> Path:
    expanded = path.expanduser()

    if expanded.is_file():
        return expanded.resolve()

    resolved = shutil.which(str(path))

    if resolved is not None:
        return Path(resolved).resolve()

    raise FileNotFoundError(f"{description} not found: {path}")


# ------------------------------------------------------------------------------


def RunCommand(command: list[str], description: str) -> float:
    print()
    print(description)
    print(f"$ {shlex.join(command)}")
    _ = sys.stdout.flush()

    start_time = time.perf_counter()
    completed = subprocess.run(command, check=False)
    elapsed_seconds = time.perf_counter() - start_time

    if completed.returncode != 0:
        raise RuntimeError(
            f"{description} failed with exit code {completed.returncode}."
        )

    return elapsed_seconds


# ------------------------------------------------------------------------------


def CheckFreeSpace(stream_directory: Path, stream_path: Path) -> None:
    disk = shutil.disk_usage(stream_directory)
    reclaimable_bytes = stream_path.stat().st_size if stream_path.is_file() else 0
    usable_bytes = disk.free + reclaimable_bytes
    required_bytes = STREAM_BYTE_COUNT + MIN_FREE_MARGIN_BYTES

    if usable_bytes < required_bytes:
        raise RuntimeError(
            "Insufficient disk space for the next Dieharder stream: "
            + f"usable={usable_bytes} bytes, required={required_bytes} bytes."
        )


# ------------------------------------------------------------------------------


def BuildGeneratorCommand(
    generator: Path,
    case: CampaignCase,
    stream_path: Path,
    manifest_path: Path,
    *,
    device: str,
    force: bool,
) -> list[str]:
    command = [
        str(generator),
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
    runner: Path,
    dieharder: str,
    manifest_path: Path,
    summary_path: Path,
) -> list[str]:
    return [
        sys.executable,
        str(runner),
        "--manifest",
        str(manifest_path),
        "--dieharder",
        dieharder,
        "--summary",
        str(summary_path),
    ]


# ------------------------------------------------------------------------------


def PrintPlan() -> None:
    print("GGEMS Dieharder reference campaign")
    print(f"Cases              : {len(CAMPAIGN_CASES)}")
    print(f"Workers            : {WORKER_COUNT}")
    print(f"Samples per worker : {SAMPLES_PER_WORKER}")
    print(f"Bytes per stream   : {STREAM_BYTE_COUNT}")
    print(f"GiB per stream     : {STREAM_BYTE_COUNT / 1024**3:.8f}")
    print()

    for index, case in enumerate(CAMPAIGN_CASES, start=1):
        print(
            f"{index:02d}/{len(CAMPAIGN_CASES):02d} "
            + f"{case.stem:<11} "
            + f"seed={case.seed:<10} "
            + f"offset={case.stream_offset:<8} "
            + f"layout={case.layout}"
        )


# ------------------------------------------------------------------------------


def main() -> int:
    args = ParseArguments()
    repository_root = RepositoryRoot()

    campaign_directory = (
        repository_root / "validation" / "random" / "results" / "dieharder" / "campaign"
    )
    manifest_directory = campaign_directory / "manifests"
    stream_directory = (
        repository_root / "validation" / "random" / "streams" / "dieharder"
    )

    campaign_directory.mkdir(parents=True, exist_ok=True)
    manifest_directory.mkdir(parents=True, exist_ok=True)
    stream_directory.mkdir(parents=True, exist_ok=True)

    PrintPlan()

    if args.dry_run:
        return 0

    generator = ResolveExecutable(args.generator, "GGEMS random stream generator")
    runner = args.runner.expanduser().resolve()

    if not runner.is_file():
        raise FileNotFoundError(f"GGEMS Dieharder runner not found: {runner}")

    skipped_count = 0
    executed_count = 0

    for index, case in enumerate(CAMPAIGN_CASES, start=1):
        print()
        print("=" * 78)
        print(f"[{index:02d}/{len(CAMPAIGN_CASES):02d}] {case.stem}")
        print("=" * 78)

        manifest_path = manifest_directory / f"{case.stem}.json"
        summary_path = campaign_directory / f"{case.stem}.json"
        stream_path = stream_directory / f"{case.stem}.bin"

        if summary_path.is_file() and not args.force:
            ValidateCompletedSummary(summary_path, case)
            ValidateManifestForCase(manifest_path, case)
            print(f"Completed summary found: {summary_path}")
            print("Case already validated; skipping.")
            skipped_count += 1
            continue

        have_stream = stream_path.is_file()
        have_manifest = manifest_path.is_file()

        if have_stream and have_manifest and not args.force:
            ValidateManifestForCase(manifest_path, case)

            existing_stream_size = stream_path.stat().st_size
            if existing_stream_size != STREAM_BYTE_COUNT:
                raise RuntimeError(
                    f"Existing stream has the wrong size for {case.stem}: "
                    + f"{existing_stream_size} bytes."
                )

            print("Existing stream and manifest found; resuming at Dieharder.")
        else:
            if (have_stream or have_manifest) and not args.force:
                raise RuntimeError(
                    f"Partial generation artifacts found for {case.stem}. "
                    + "Remove them or rerun the campaign with --force."
                )

            CheckFreeSpace(stream_directory, stream_path)

            generation_elapsed = RunCommand(
                BuildGeneratorCommand(
                    generator,
                    case,
                    stream_path,
                    manifest_path,
                    device=args.device,
                    force=args.force,
                ),
                f"Generating {case.stem}",
            )

            print(f"Generation elapsed: {generation_elapsed:.3f} s")

            if not stream_path.is_file() or not manifest_path.is_file():
                raise RuntimeError(
                    f"Generator did not produce both artifacts for {case.stem}."
                )

            generated_stream_size = stream_path.stat().st_size
            if generated_stream_size != STREAM_BYTE_COUNT:
                raise RuntimeError(
                    f"Generated stream has the wrong size for {case.stem}: "
                    + f"{generated_stream_size} bytes."
                )

            ValidateManifestForCase(manifest_path, case)

        validation_elapsed = RunCommand(
            BuildDieharderCommand(
                runner,
                args.dieharder,
                manifest_path,
                summary_path,
            ),
            f"Running Dieharder for {case.stem}",
        )

        print(f"Campaign-side validation elapsed: {validation_elapsed:.3f} s")

        ValidateCompletedSummary(summary_path, case)

        if args.keep_streams:
            print(f"Keeping stream: {stream_path}")
        else:
            stream_path.unlink()
            print(f"Removed completed stream: {stream_path}")

        executed_count += 1

    print()
    print("=" * 78)
    print("GGEMS Dieharder campaign finished")
    print(f"Cases          : {len(CAMPAIGN_CASES)}")
    print(f"Skipped        : {skipped_count}")
    print(f"Executed       : {executed_count}")
    print(f"Campaign dir   : {campaign_directory}")
    print(f"Manifest dir   : {manifest_directory}")
    print("=" * 78)

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
        TypeError,
        ValueError,
    ) as error:
        print(f"GGEMS Dieharder campaign failed:\n{error}")
        raise SystemExit(1) from None
