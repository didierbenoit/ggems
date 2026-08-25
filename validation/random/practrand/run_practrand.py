import argparse
import json
import re
import shutil
import subprocess
from pathlib import Path
from typing import Literal, Protocol, TypedDict, cast

# ------------------------------------------------------------------------------

type PractRandStatus = Literal[
    "passed_no_anomalies",
    "attention_required",
    "failed",
    "tool_error",
]

# ------------------------------------------------------------------------------


class RandomSection(TypedDict):
    engine: str
    seed: int
    worker_count: int
    samples_per_worker: int
    total_samples: int
    byte_count: int
    stream_type: str
    layout: str
    stream_offset: int
    sample_bits: int


# ------------------------------------------------------------------------------


class OutputSection(TypedDict):
    stream_path: str


# ------------------------------------------------------------------------------


class RandomManifest(TypedDict):
    schema_version: int
    random: RandomSection
    output: OutputSection


# ------------------------------------------------------------------------------


class Arguments(Protocol):
    manifest: Path
    max_size: str | None
    rng_test: str
    summary: Path | None
    multithreaded: bool


# ------------------------------------------------------------------------------


def ParseArguments() -> Arguments:
    parser = argparse.ArgumentParser(
        description="Run PractRand on a GGEMS random stream."
    )

    _ = parser.add_argument(
        "--manifest",
        type=Path,
        required=True,
        help="GGEMS random stream manifest.",
    )

    _ = parser.add_argument(
        "--max-size",
        help=(
            "Optional maximum PractRand test size. "
            "By default, the complete GGEMS stream is tested."
        ),
    )

    _ = parser.add_argument(
        "--rng-test",
        default="RNG_test",
        help="Path to PractRand RNG_test, or RNG_test if available in PATH.",
    )

    _ = parser.add_argument(
        "--summary",
        type=Path,
        help="Path to the GGEMS PractRand summary JSON.",
    )

    _ = parser.add_argument(
        "--multithreaded",
        action="store_true",
        help="Enable PractRand multithreaded test execution.",
    )

    return cast(Arguments, cast(object, parser.parse_args()))


# ------------------------------------------------------------------------------


def ResolveExecutable(executable: str) -> Path:
    candidate = Path(executable).expanduser()

    if candidate.is_file():
        return candidate.resolve()

    resolved = shutil.which(executable)

    if resolved is not None:
        return Path(resolved).resolve()

    raise FileNotFoundError(f"PractRand RNG_test executable not found: {executable}")


# ------------------------------------------------------------------------------


def ParsePractRandSize(value: str) -> int:
    match = re.fullmatch(r"([1-9][0-9]*)(KB|MB|GB|TB)", value)

    if match is None:
        raise ValueError(
            f"Invalid PractRand maximum size '{value}'. "
            "Expected for example 4MB, 1GB, or 4GB."
        )

    amount = int(match.group(1))
    unit = match.group(2)

    unit_bytes = {
        "KB": 1024,
        "MB": 1024**2,
        "GB": 1024**3,
        "TB": 1024**4,
    }

    return amount * unit_bytes[unit]


# ------------------------------------------------------------------------------


def FormatPractRandSize(byte_count: int) -> str:
    units = (
        ("TB", 1024**4),
        ("GB", 1024**3),
        ("MB", 1024**2),
        ("KB", 1024),
    )

    for suffix, unit_bytes in units:
        if byte_count % unit_bytes == 0:
            return f"{byte_count // unit_bytes}{suffix}"

    raise ValueError(
        f"GGEMS random stream size {byte_count} bytes cannot be expressed "
        "exactly using PractRand KB/MB/GB/TB units."
    )


# ------------------------------------------------------------------------------


def ResolveMaxSize(
    requested_size: str | None,
    stream_byte_count: int,
) -> str:
    if requested_size is None:
        return FormatPractRandSize(stream_byte_count)

    requested_bytes = ParsePractRandSize(requested_size)

    if requested_bytes > stream_byte_count:
        raise ValueError(
            f"Requested PractRand size {requested_size} exceeds the "
            f"GGEMS stream size of {stream_byte_count} bytes."
        )

    return requested_size


# ------------------------------------------------------------------------------


def RunPractRand(
    command: list[str],
    stream_path: Path,
) -> tuple[int, str]:
    output_lines: list[str] = []

    with stream_path.open("rb") as stream:
        process: subprocess.Popen[str] = subprocess.Popen(
            command,
            stdin=stream,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        stdout = process.stdout

        if stdout is None:
            raise RuntimeError("Failed to capture PractRand output.")

        for line in stdout:
            print(line, end="")
            output_lines.append(line)

        return_code = process.wait()

    return return_code, "".join(output_lines)


# ------------------------------------------------------------------------------


def ClassifyPractRandOutput(
    output: str,
    return_code: int,
) -> PractRandStatus:
    if return_code != 0:
        return "tool_error"

    tool_error_markers = (
        "invalid test length:",
        "internal error",
        "an error occurred, aborting",
        "failed basic check",
    )

    if any(marker in output.lower() for marker in tool_error_markers):
        return "tool_error"

    if re.search(r"\bFAIL(?:\s|!|$)", output) is not None:
        return "failed"

    attention_markers = (
        "very suspicious",
        "mildly suspicious",
        "suspicious",
        "unusual",
    )

    if any(marker in output for marker in attention_markers):
        return "attention_required"

    if (
        re.search(
            r"no anomalies in \d+ test result\(s\)",
            output,
        )
        is not None
    ):
        return "passed_no_anomalies"

    return "tool_error"


# ------------------------------------------------------------------------------


def ExtractTestedLength(output: str) -> str | None:
    tested_length: str | None = None

    for line in output.splitlines():
        stripped = line.strip()

        if stripped.startswith("length="):
            tested_length = stripped

    return tested_length


# ------------------------------------------------------------------------------


def ExtractAnomalies(output: str) -> list[str]:
    anomalies: list[str] = []

    for line in output.splitlines():
        stripped = line.strip()

        if (
            re.search(
                r"\b(unusual|suspicious|FAIL)\b",
                stripped,
                flags=re.IGNORECASE,
            )
            is not None
        ):
            anomalies.append(stripped)

    return anomalies


# ------------------------------------------------------------------------------


def ExtractPractRandVersion(output: str) -> str | None:
    match = re.search(
        r"RNG_test using PractRand version ([^\s]+)",
        output,
    )

    if match is None:
        return None

    return match.group(1)


# ------------------------------------------------------------------------------


def ExtractTestResultCount(
    output: str,
    anomaly_count: int,
) -> int | None:
    matches = re.findall(
        r"(?:no anomalies in|\.\.\.and)\s+(\d+)\s+test result\(s\)",
        output,
    )

    if not matches:
        return None

    result_count = int(matches[-1])

    if "...and" in output:
        result_count += anomaly_count

    return result_count


# ------------------------------------------------------------------------------


def ExtractPractRandConfiguration(
    output: str,
) -> tuple[str | None, str | None]:
    match = re.search(
        r"^test set = ([^,\n]+), folding = (.+)$",
        output,
        flags=re.MULTILINE,
    )

    if match is None:
        return None, None

    test_set = match.group(1).strip()
    folding = match.group(2).strip()

    return test_set, folding


# ------------------------------------------------------------------------------


def LoadManifest(path: Path) -> RandomManifest:
    path = path.expanduser().resolve()

    if not path.is_file():
        raise FileNotFoundError(f"GGEMS random manifest not found: {path}")

    data = cast(
        object,
        json.loads(path.read_text(encoding="utf-8")),
    )

    if not isinstance(data, dict):
        raise ValueError(f"Invalid GGEMS random manifest: {path}")

    return cast(RandomManifest, cast(object, data))


# ------------------------------------------------------------------------------


def ResolveStreamPath(
    manifest: RandomManifest,
) -> Path:
    stream_path = Path(manifest["output"]["stream_path"]).expanduser().resolve()

    if not stream_path.is_file():
        raise FileNotFoundError(f"GGEMS random stream not found: {stream_path}")

    expected_bytes = manifest["random"]["byte_count"]
    actual_bytes = stream_path.stat().st_size

    if actual_bytes != expected_bytes:
        raise ValueError(
            f"GGEMS random stream size mismatch: "
            f"expected {expected_bytes} bytes, got {actual_bytes} bytes."
        )

    return stream_path


# ------------------------------------------------------------------------------


def ResolvePractRandInput(manifest: RandomManifest) -> str:
    random = manifest["random"]
    stream_type = random["stream_type"]
    sample_bits = random["sample_bits"]

    if stream_type == "raw_uint32":
        if sample_bits != 32:
            raise ValueError("raw_uint32 stream must contain 32-bit samples.")

        return "stdin32"

    if stream_type == "uniform24_scalar":
        if sample_bits != 24:
            raise ValueError("uniform24_scalar stream must contain 24-bit samples.")

        return "stdin8"

    if stream_type == "uniform24_vector4":
        if sample_bits != 24:
            raise ValueError("uniform24_vector4 stream must contain 24-bit samples.")

        return "stdin8"

    raise ValueError(f"Unsupported GGEMS random stream type '{stream_type}'.")


# ------------------------------------------------------------------------------


def WriteSummary(
    path: Path,
    *,
    manifest_path: Path,
    manifest: RandomManifest,
    stream_path: Path,
    rng_test: Path,
    command: list[str],
    max_size: str,
    return_code: int,
    version: str | None,
    test_set: str | None,
    folding: str | None,
    status: PractRandStatus,
    tested_length: str | None,
    test_result_count: int | None,
    anomalies: list[str],
    practrand_input: str,
    multithreaded: bool,
) -> None:
    path = path.expanduser().resolve()
    path.parent.mkdir(parents=True, exist_ok=True)

    random = manifest["random"]

    summary: dict[str, object] = {
        "schema_version": 1,
        "tool": {
            "name": "PractRand",
            "version": version,
            "executable": str(rng_test),
            "command": command,
            "return_code": return_code,
            "max_size": max_size,
            "test_set": test_set,
            "folding": folding,
            "multithreaded": multithreaded,
        },
        "input": {
            "manifest_path": str(manifest_path),
            "stream_path": str(stream_path),
            "engine": random["engine"],
            "seed": random["seed"],
            "stream_offset": random["stream_offset"],
            "worker_count": random["worker_count"],
            "samples_per_worker": random["samples_per_worker"],
            "total_samples": random["total_samples"],
            "byte_count": random["byte_count"],
            "stream_type": random["stream_type"],
            "sample_bits": random["sample_bits"],
            "layout": random["layout"],
            "practrand_input": practrand_input,
        },
        "output": {
            "status": status,
            "tested_length": tested_length,
            "test_result_count": test_result_count,
            "anomalies": anomalies,
        },
    }

    path.write_text(
        json.dumps(summary, indent=2) + "\n",
        encoding="utf-8",
    )


# ------------------------------------------------------------------------------


def main() -> int:
    args = ParseArguments()

    manifest_path = args.manifest.expanduser().resolve()
    manifest = LoadManifest(manifest_path)
    stream_path = ResolveStreamPath(manifest)
    rng_test = ResolveExecutable(args.rng_test)
    random = manifest["random"]

    practrand_input = ResolvePractRandInput(manifest)

    max_size = ResolveMaxSize(
        args.max_size,
        random["byte_count"],
    )

    command = [
        str(rng_test),
        practrand_input,
        "-tlmax",
        max_size,
    ]

    if args.multithreaded:
        command.append("-multithreaded")

    print("GGEMS PractRand validation")
    print(f"Manifest    : {manifest_path}")
    print(f"Stream      : {stream_path}")
    print(f"Stream type : {random['stream_type']}")
    print(f"Input       : {practrand_input}")
    print(f"Engine      : {random['engine']}")
    print(f"Seed        : {random['seed']}")
    print(f"RNG_test    : {rng_test}")
    print(f"Max size    : {max_size}")
    print(f"Multithread : {'yes' if args.multithreaded else 'no'}")
    print()

    return_code, output = RunPractRand(command, stream_path)

    status = ClassifyPractRandOutput(output, return_code)
    tested_length = ExtractTestedLength(output)
    anomalies = ExtractAnomalies(output)
    version = ExtractPractRandVersion(output)
    test_result_count = ExtractTestResultCount(
        output,
        len(anomalies),
    )
    test_set, folding = ExtractPractRandConfiguration(output)

    if args.summary is not None:
        WriteSummary(
            args.summary,
            manifest_path=manifest_path,
            manifest=manifest,
            stream_path=stream_path,
            rng_test=rng_test,
            command=command,
            max_size=max_size,
            return_code=return_code,
            version=version,
            test_set=test_set,
            folding=folding,
            status=status,
            tested_length=tested_length,
            test_result_count=test_result_count,
            anomalies=anomalies,
            practrand_input=practrand_input,
            multithreaded=args.multithreaded,
        )

    print()
    print(f"Status   : {status}")

    if tested_length is not None:
        print(f"Tested   : {tested_length}")

    print(f"Anomalies: {len(anomalies)}")

    if args.summary is not None:
        print(f"Summary  : {args.summary.expanduser().resolve()}")

    if status == "tool_error":
        return 1

    return 0


# ------------------------------------------------------------------------------

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, RuntimeError, ValueError) as error:
        print(f"GGEMS PractRand validation failed:\n{error}")
        raise SystemExit(1) from None
