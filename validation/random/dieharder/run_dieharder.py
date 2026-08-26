import argparse
import ctypes
import json
import math
import os
import platform
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from collections import Counter
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path
from typing import Literal, Protocol, TypedDict, cast

# ------------------------------------------------------------------------------

type Assessment = Literal["PASSED", "WEAK", "FAILED"]
type Reliability = Literal["Good", "Suspect", "Do Not Use"]
type RunStatus = Literal["completed", "tool_error"]

# ------------------------------------------------------------------------------

DIEHARDER_REFERENCE_VERSION = "3.31.1"
DIEHARDER_GENERATOR_ID = 200
MIN_REFERENCE_BYTE_COUNT = 256_000_000_000
EXPECTED_ASSESSMENT_COUNT = 114

EXPECTED_RESULT_COUNTS: dict[int, int] = {
    **{test_id: 1 for test_id in range(15)},
    15: 2,
    16: 2,
    17: 2,
    100: 1,
    101: 1,
    102: 30,
    200: 12,
    201: 4,
    202: 4,
    203: 33,
    204: 1,
    205: 1,
    206: 1,
    207: 2,
    208: 2,
    209: 1,
}

EXPECTED_SWEEPS: dict[int, tuple[int, ...]] = {
    200: tuple(range(1, 13)),
    201: tuple(range(2, 6)),
    202: tuple(range(2, 6)),
    203: tuple(range(33)),
}

IMPLEMENTATION_CAVEAT_IDS = frozenset({0, 206, 207})

VERSION_PATTERN = re.compile(r"\bdieharder version ([0-9]+(?:\.[0-9]+)+)\b")
CATALOG_PATTERN = re.compile(r"^\s*-d\s+(\d+)\s+(.+?)\s+(Do Not Use|Suspect|Good)\s*$")
STDIN_ERROR_PATTERN = re.compile(r"^\s*#\s*stdin_input_raw\(\):\s*Error:", re.MULTILINE)
PREPARING_PATTERN = re.compile(r"^\s*Preparing to run test\s+\d+", re.MULTILINE)

# ------------------------------------------------------------------------------


class RandomSection(TypedDict):
    engine: str
    seed: int
    stream_offset: int
    stream_type: str
    sample_bits: int
    layout: str
    worker_count: int
    samples_per_worker: int
    total_samples: int
    byte_count: int


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
    dieharder: str
    summary: Path


# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class TestDefinition:
    test_id: int
    name: str
    reliability: Reliability


# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class AssessmentRecord:
    test_id: int
    test_name: str
    ntuple: int
    tsamples: int
    psamples: int
    p_value: float
    assessment: Assessment
    reliability: Reliability
    implementation_caveat: bool


# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class ToolInspection:
    version: str
    catalog: dict[int, TestDefinition]
    catalog_output: str


# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class DieharderExecution:
    return_code: int
    stdout: str
    stderr: str
    command: list[str]
    elapsed_seconds: float
    input_file_offset_bytes: int
    remaining_input_bytes: int


# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class RuntimeEnvironment:
    system: str
    release: str
    machine: str
    byteorder: str
    c_unsigned_int_bytes: int
    uint_max: int
    gsl_library: str
    gsl_cblas_library: str
    linked_library_output: str
    package_versions: tuple[str, ...]


# ------------------------------------------------------------------------------


def ParseArguments() -> Arguments:
    parser = argparse.ArgumentParser(
        description="Run the GGEMS Dieharder 3.31.1 reference validation."
    )

    _ = parser.add_argument(
        "--manifest",
        type=Path,
        required=True,
        help="GGEMS raw uint32 stream manifest.",
    )
    _ = parser.add_argument(
        "--dieharder",
        default="dieharder",
        help="Dieharder executable or path (default: dieharder).",
    )
    _ = parser.add_argument(
        "--summary",
        type=Path,
        required=True,
        help="Output path for the complete GGEMS Dieharder summary JSON.",
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

    raise FileNotFoundError(f"Dieharder executable not found: {executable}")


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
        raise TypeError(f"Invalid GGEMS random manifest type: {path}")

    return cast(RandomManifest, cast(object, data))


# ------------------------------------------------------------------------------


def ResolveStreamPath(manifest: RandomManifest) -> Path:
    stream_path = Path(manifest["output"]["stream_path"]).expanduser().resolve()

    if not stream_path.is_file():
        raise FileNotFoundError(f"GGEMS random stream not found: {stream_path}")

    return stream_path


# ------------------------------------------------------------------------------


def ValidateManifest(manifest: RandomManifest, stream_path: Path) -> None:
    random = manifest["random"]

    if random["stream_type"] != "raw_uint32":
        raise ValueError(
            "Dieharder reference validation accepts only raw_uint32 streams."
        )

    if random["sample_bits"] != 32:
        raise ValueError("raw_uint32 Dieharder input must contain 32-bit samples.")

    if random["worker_count"] <= 0 or random["samples_per_worker"] <= 0:
        raise ValueError("GGEMS Dieharder stream dimensions must be positive.")

    expected_total_samples = random["worker_count"] * random["samples_per_worker"]

    if random["total_samples"] != expected_total_samples:
        raise ValueError(
            "GGEMS random manifest total_samples mismatch: "
            + f"expected {expected_total_samples}, got {random['total_samples']}."
        )

    expected_byte_count = random["total_samples"] * 4

    if random["byte_count"] != expected_byte_count:
        raise ValueError(
            "GGEMS random manifest byte_count mismatch: "
            + f"expected {expected_byte_count}, got {random['byte_count']}."
        )

    actual_byte_count = stream_path.stat().st_size

    if actual_byte_count != random["byte_count"]:
        raise ValueError(
            "GGEMS random stream size mismatch: "
            + f"manifest has {random['byte_count']} bytes, file has "
            + f"{actual_byte_count} bytes."
        )

    if actual_byte_count % 4 != 0:
        raise ValueError(
            "GGEMS Dieharder raw stream size must be a multiple of 4 bytes."
        )

    if actual_byte_count < MIN_REFERENCE_BYTE_COUNT:
        raise ValueError(
            "GGEMS Dieharder reference stream is too small: "
            + f"{actual_byte_count} bytes; at least {MIN_REFERENCE_BYTE_COUNT} "
            + "bytes are required by the audited reference protocol."
        )


# ------------------------------------------------------------------------------


def ParseReliability(value: str) -> Reliability:
    if value == "Good":
        return "Good"
    if value == "Suspect":
        return "Suspect"
    if value == "Do Not Use":
        return "Do Not Use"

    raise ValueError(f"Unknown Dieharder reliability classification: {value}")


# ------------------------------------------------------------------------------


def InspectDieharder(executable: Path) -> ToolInspection:
    environment = dict(os.environ)
    environment["LC_ALL"] = "C"

    result = subprocess.run(
        [str(executable), "-l"],
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        env=environment,
    )

    if result.returncode != 0:
        raise RuntimeError(
            "Failed to inspect Dieharder test catalog: "
            + f"exit code {result.returncode}.\n{result.stderr.strip()}"
        )

    version_match = VERSION_PATTERN.search(result.stdout)

    if version_match is None:
        raise RuntimeError("Unable to determine Dieharder version from -l output.")

    version = version_match.group(1)

    if version != DIEHARDER_REFERENCE_VERSION:
        raise RuntimeError(
            "Unsupported Dieharder version for the audited reference protocol: "
            + f"{version}; expected {DIEHARDER_REFERENCE_VERSION}."
        )

    catalog: dict[int, TestDefinition] = {}

    for line in result.stdout.splitlines():
        match = CATALOG_PATTERN.match(line)

        if match is None:
            continue

        test_id = int(match.group(1))
        reliability = ParseReliability(match.group(3))
        catalog[test_id] = TestDefinition(
            test_id=test_id,
            name=match.group(2).strip(),
            reliability=reliability,
        )

    expected_ids = set(EXPECTED_RESULT_COUNTS)

    if set(catalog) != expected_ids:
        missing = sorted(expected_ids - set(catalog))
        unexpected = sorted(set(catalog) - expected_ids)
        raise RuntimeError(
            "Unexpected Dieharder 3.31.1 test catalog: "
            + f"missing={missing}, unexpected={unexpected}."
        )

    reliability_counts = Counter(
        definition.reliability for definition in catalog.values()
    )

    if reliability_counts != Counter({"Good": 27, "Suspect": 3, "Do Not Use": 1}):
        raise RuntimeError(
            "Unexpected Dieharder 3.31.1 reliability classification counts: "
            + str(dict(reliability_counts))
        )

    return ToolInspection(
        version=version,
        catalog=catalog,
        catalog_output=result.stdout,
    )


# ------------------------------------------------------------------------------


def ValidateNativeUInt32ABI() -> tuple[int, int]:
    unsigned_int_bytes = ctypes.sizeof(ctypes.c_uint)
    struct_unsigned_int_bytes = struct.calcsize("I")
    uint_max = ctypes.c_uint(-1).value
    unsigned_char_max = ctypes.c_ubyte(-1).value

    if unsigned_int_bytes != 4 or struct_unsigned_int_bytes != 4:
        raise RuntimeError(
            "Dieharder reference protocol requires a 4-byte native unsigned int."
        )

    if uint_max != 0xFFFFFFFF:
        raise RuntimeError(
            "Dieharder reference protocol requires UINT_MAX == 0xffffffff."
        )

    if unsigned_char_max != 0xFF:
        raise RuntimeError(
            "Dieharder reference protocol requires 8-bit unsigned char semantics."
        )

    if sys.byteorder not in {"little", "big"}:
        raise RuntimeError(f"Unsupported native byte order: {sys.byteorder}")

    return unsigned_int_bytes, uint_max


# ------------------------------------------------------------------------------


def InspectLinkedLibraries(executable: Path) -> tuple[str, str, str]:
    ldd_path = shutil.which("ldd")

    if ldd_path is None:
        raise FileNotFoundError("ldd is required for Dieharder runtime provenance.")

    result = subprocess.run(
        [ldd_path, str(executable)],
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )

    if result.returncode != 0:
        raise RuntimeError(
            "Failed to inspect Dieharder linked libraries: "
            + f"exit code {result.returncode}.\n{result.stderr.strip()}"
        )

    gsl_library: str | None = None
    gsl_cblas_library: str | None = None

    for line in result.stdout.splitlines():
        stripped = line.strip()

        if stripped.startswith("libgsl.so"):
            parts = stripped.split("=>", maxsplit=1)
            if len(parts) == 2:
                gsl_library = parts[1].strip().split()[0]
        elif stripped.startswith("libgslcblas.so"):
            parts = stripped.split("=>", maxsplit=1)
            if len(parts) == 2:
                gsl_cblas_library = parts[1].strip().split()[0]

    if gsl_library is None or gsl_cblas_library is None:
        raise RuntimeError(
            "Dieharder must be dynamically linked to both libgsl and libgslcblas."
        )

    return gsl_library, gsl_cblas_library, result.stdout


# ------------------------------------------------------------------------------


def InspectRpmPackages(paths: tuple[Path, ...]) -> tuple[str, ...]:
    rpm_path = shutil.which("rpm")

    if rpm_path is None:
        return ()

    result = subprocess.run(
        [
            rpm_path,
            "-qf",
            "--qf",
            "%{NAME} %{VERSION}-%{RELEASE}.%{ARCH}\n",
            *[str(path) for path in paths],
        ],
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )

    if result.returncode != 0:
        return ()

    return tuple(
        sorted({line.strip() for line in result.stdout.splitlines() if line.strip()})
    )


# ------------------------------------------------------------------------------


def InspectRuntimeEnvironment(executable: Path) -> RuntimeEnvironment:
    unsigned_int_bytes, uint_max = ValidateNativeUInt32ABI()
    gsl_library, gsl_cblas_library, linked_library_output = InspectLinkedLibraries(
        executable
    )

    package_versions = InspectRpmPackages(
        (
            executable,
            Path(gsl_library),
            Path(gsl_cblas_library),
        )
    )

    return RuntimeEnvironment(
        system=platform.system(),
        release=platform.release(),
        machine=platform.machine(),
        byteorder=sys.byteorder,
        c_unsigned_int_bytes=unsigned_int_bytes,
        uint_max=uint_max,
        gsl_library=gsl_library,
        gsl_cblas_library=gsl_cblas_library,
        linked_library_output=linked_library_output,
        package_versions=package_versions,
    )


# ------------------------------------------------------------------------------


def RunDieharder(
    executable: Path,
    stream_path: Path,
) -> DieharderExecution:
    command = [
        str(executable),
        "-a",
        "-g",
        str(DIEHARDER_GENERATOR_ID),
        "-D",
        "default",
        "-D",
        "show_num",
    ]

    environment = dict(os.environ)
    environment["LC_ALL"] = "C"

    stdout_lines: list[str] = []
    stream_byte_count = stream_path.stat().st_size

    with (
        stream_path.open("rb") as input_stream,
        tempfile.TemporaryFile(
            mode="w+",
            encoding="utf-8",
            errors="replace",
        ) as stderr_stream,
    ):
        start_time = time.perf_counter()

        process: subprocess.Popen[str] = subprocess.Popen(
            command,
            stdin=input_stream,
            stdout=subprocess.PIPE,
            stderr=stderr_stream,
            text=True,
            encoding="utf-8",
            errors="replace",
            env=environment,
        )

        stdout = process.stdout

        if stdout is None:
            raise RuntimeError("Failed to capture Dieharder stdout.")

        stdout_iterable = cast(Iterable[str], cast(object, stdout))

        for line in stdout_iterable:
            print(line, end="")
            stdout_lines.append(line)

        return_code = process.wait()
        elapsed_seconds = time.perf_counter() - start_time
        input_file_offset_bytes = os.lseek(input_stream.fileno(), 0, os.SEEK_CUR)

        if not 0 <= input_file_offset_bytes <= stream_byte_count:
            raise RuntimeError(
                "Observed Dieharder input file offset is outside the stream: "
                + f"offset={input_file_offset_bytes}, size={stream_byte_count}."
            )

        remaining_input_bytes = stream_byte_count - input_file_offset_bytes

        _ = stderr_stream.seek(0)
        stderr_output = stderr_stream.read()

    return DieharderExecution(
        return_code=return_code,
        stdout="".join(stdout_lines),
        stderr=stderr_output,
        command=command,
        elapsed_seconds=elapsed_seconds,
        input_file_offset_bytes=input_file_offset_bytes,
        remaining_input_bytes=remaining_input_bytes,
    )


# ------------------------------------------------------------------------------


def ParseAssessment(value: str) -> Assessment:
    if value == "PASSED":
        return "PASSED"
    if value == "WEAK":
        return "WEAK"
    if value == "FAILED":
        return "FAILED"

    raise ValueError(f"Unknown Dieharder assessment: {value}")


# ------------------------------------------------------------------------------


def ParseAssessmentRows(
    output: str,
    catalog: dict[int, TestDefinition],
) -> list[AssessmentRecord]:
    assessments: list[AssessmentRecord] = []

    for line in output.splitlines():
        fields = [field.strip() for field in line.split("|")]

        if len(fields) != 7:
            continue

        if fields[-1] not in {"PASSED", "WEAK", "FAILED"}:
            continue

        try:
            test_id = int(fields[1])
            ntuple = int(fields[2])
            tsamples = int(fields[3])
            psamples = int(fields[4])
            p_value = float(fields[5])
            assessment = ParseAssessment(fields[6])
        except ValueError as error:
            raise RuntimeError(f"Invalid Dieharder result row: {line}") from error

        if test_id not in catalog:
            raise RuntimeError(f"Unexpected Dieharder test ID in result row: {test_id}")

        if not math.isfinite(p_value) or not 0.0 <= p_value <= 1.0:
            raise RuntimeError(
                "Dieharder produced a non-finite or out-of-range p-value: "
                + f"{p_value} for test {test_id}."
            )

        definition = catalog[test_id]

        assessments.append(
            AssessmentRecord(
                test_id=test_id,
                test_name=fields[0],
                ntuple=ntuple,
                tsamples=tsamples,
                psamples=psamples,
                p_value=p_value,
                assessment=assessment,
                reliability=definition.reliability,
                implementation_caveat=test_id in IMPLEMENTATION_CAVEAT_IDS,
            )
        )

    return assessments


# ------------------------------------------------------------------------------


def ValidateAssessmentStructure(assessments: list[AssessmentRecord]) -> None:
    if len(assessments) != EXPECTED_ASSESSMENT_COUNT:
        raise RuntimeError(
            "Incomplete Dieharder result set: "
            + f"expected {EXPECTED_ASSESSMENT_COUNT} assessments, "
            + f"got {len(assessments)}."
        )

    counts = Counter(record.test_id for record in assessments)

    if dict(counts) != EXPECTED_RESULT_COUNTS:
        raise RuntimeError(
            "Unexpected Dieharder assessment multiplicities: "
            + f"got {dict(sorted(counts.items()))}."
        )

    for test_id, expected_ntuples in EXPECTED_SWEEPS.items():
        actual_ntuples = tuple(
            record.ntuple for record in assessments if record.test_id == test_id
        )

        if actual_ntuples != expected_ntuples:
            raise RuntimeError(
                f"Unexpected Dieharder sweep for test {test_id}: "
                + f"expected {expected_ntuples}, got {actual_ntuples}."
            )


# ------------------------------------------------------------------------------


def ValidateTechnicalCompletion(
    return_code: int,
    stdout: str,
    stderr: str,
    assessments: list[AssessmentRecord],
) -> None:
    if return_code != 0:
        raise RuntimeError(f"Dieharder exited with nonzero status {return_code}.")

    if STDIN_ERROR_PATTERN.search(stderr) is not None:
        raise RuntimeError(
            "Dieharder reported a stdin_input_raw error; the run is incomplete."
        )

    if stderr.strip():
        raise RuntimeError(
            "Dieharder produced unexpected stderr output:\n" + stderr.strip()
        )

    preparing_lines = PREPARING_PATTERN.findall(stdout)

    if len(preparing_lines) != 3:
        raise RuntimeError(
            "Unexpected Dieharder non-tabular preparation-line count: "
            + f"expected 3, got {len(preparing_lines)}."
        )

    ValidateAssessmentStructure(assessments)


# ------------------------------------------------------------------------------


def AssessmentToJson(record: AssessmentRecord) -> dict[str, object]:
    return {
        "test_id": record.test_id,
        "test_name": record.test_name,
        "ntuple": record.ntuple,
        "tsamples": record.tsamples,
        "psamples": record.psamples,
        "p_value": record.p_value,
        "assessment": record.assessment,
        "reliability": record.reliability,
        "implementation_caveat": record.implementation_caveat,
    }


# ------------------------------------------------------------------------------


def RuntimeEnvironmentToJson(environment: RuntimeEnvironment) -> dict[str, object]:
    return {
        "system": environment.system,
        "release": environment.release,
        "machine": environment.machine,
        "byteorder": environment.byteorder,
        "c_unsigned_int_bytes": environment.c_unsigned_int_bytes,
        "uint_max": environment.uint_max,
        "gsl_library": environment.gsl_library,
        "gsl_cblas_library": environment.gsl_cblas_library,
        "linked_library_output": environment.linked_library_output,
        "package_versions": list(environment.package_versions),
    }


# ------------------------------------------------------------------------------


def WriteSummary(
    path: Path,
    *,
    manifest_path: Path,
    manifest: RandomManifest,
    stream_path: Path,
    executable: Path,
    inspection: ToolInspection,
    runtime_environment: RuntimeEnvironment,
    execution: DieharderExecution,
    status: RunStatus,
    assessments: list[AssessmentRecord],
    error_message: str | None,
) -> None:
    path = path.expanduser().resolve()
    path.parent.mkdir(parents=True, exist_ok=True)

    random = manifest["random"]
    assessment_counts = Counter(record.assessment for record in assessments)
    reliability_counts = Counter(record.reliability for record in assessments)
    primary_records = [
        record
        for record in assessments
        if record.reliability == "Good" and not record.implementation_caveat
    ]
    primary_counts = Counter(record.assessment for record in primary_records)

    summary: dict[str, object] = {
        "schema_version": 1,
        "tool": {
            "name": "Dieharder",
            "version": inspection.version,
            "executable": str(executable),
            "command": execution.command,
            "return_code": execution.return_code,
            "catalog_output": inspection.catalog_output,
        },
        "protocol": {
            "name": "ggems_dieharder_3_31_1_reference",
            "generator_id": DIEHARDER_GENERATOR_ID,
            "battery": "all",
            "minimum_reference_byte_count": MIN_REFERENCE_BYTE_COUNT,
            "expected_assessment_count": EXPECTED_ASSESSMENT_COUNT,
            "locale": "C",
            "output_flags": ["default", "show_num"],
            "input_file_offset_semantics": (
                "Observed OS file offset after Dieharder exits; this may include "
                "stdio read-ahead and is not an exact logical RNG word count."
            ),
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
        },
        "environment": RuntimeEnvironmentToJson(runtime_environment),
        "output": {
            "status": status,
            "error_message": error_message,
            "elapsed_seconds": execution.elapsed_seconds,
            "input_file_offset_bytes": execution.input_file_offset_bytes,
            "remaining_input_bytes": execution.remaining_input_bytes,
            "assessment_count": len(assessments),
            "assessment_counts": {
                "PASSED": assessment_counts["PASSED"],
                "WEAK": assessment_counts["WEAK"],
                "FAILED": assessment_counts["FAILED"],
            },
            "reliability_counts": {
                "Good": reliability_counts["Good"],
                "Suspect": reliability_counts["Suspect"],
                "Do Not Use": reliability_counts["Do Not Use"],
            },
            "primary_assessment_counts": {
                "PASSED": primary_counts["PASSED"],
                "WEAK": primary_counts["WEAK"],
                "FAILED": primary_counts["FAILED"],
            },
            "assessments": [AssessmentToJson(record) for record in assessments],
            "stdout": execution.stdout,
            "stderr": execution.stderr,
        },
    }

    _ = path.write_text(
        json.dumps(summary, indent=2) + "\n",
        encoding="utf-8",
    )


# ------------------------------------------------------------------------------


def main() -> int:
    args = ParseArguments()

    manifest_path = args.manifest.expanduser().resolve()
    manifest = LoadManifest(manifest_path)
    stream_path = ResolveStreamPath(manifest)
    ValidateManifest(manifest, stream_path)

    executable = ResolveExecutable(args.dieharder)
    inspection = InspectDieharder(executable)
    runtime_environment = InspectRuntimeEnvironment(executable)

    print("GGEMS Dieharder reference validation")
    print(f"Manifest     : {manifest_path}")
    print(f"Stream       : {stream_path}")
    print(f"Engine       : {manifest['random']['engine']}")
    print(f"Seed         : {manifest['random']['seed']}")
    print(f"Bytes        : {manifest['random']['byte_count']}")
    print(f"GiB          : {manifest['random']['byte_count'] / 1024**3:.8f}")
    print(f"Dieharder    : {executable}")
    print(f"Version      : {inspection.version}")
    print(f"GSL          : {runtime_environment.gsl_library}")
    print(f"GSL CBLAS    : {runtime_environment.gsl_cblas_library}")
    if runtime_environment.package_versions:
        print(f"Packages     : {', '.join(runtime_environment.package_versions)}")
    print(f"Byte order   : {runtime_environment.byteorder}")
    print()
    print("Dieharder:")

    execution = RunDieharder(executable, stream_path)
    assessments = ParseAssessmentRows(execution.stdout, inspection.catalog)

    status: RunStatus = "completed"
    error_message: str | None = None

    try:
        ValidateTechnicalCompletion(
            execution.return_code,
            execution.stdout,
            execution.stderr,
            assessments,
        )
    except RuntimeError as error:
        status = "tool_error"
        error_message = str(error)

    WriteSummary(
        args.summary,
        manifest_path=manifest_path,
        manifest=manifest,
        stream_path=stream_path,
        executable=executable,
        inspection=inspection,
        runtime_environment=runtime_environment,
        execution=execution,
        status=status,
        assessments=assessments,
        error_message=error_message,
    )

    print()
    print(f"Status       : {status}")
    print(f"Assessments  : {len(assessments)}")
    print(
        f"PASSED       : {sum(record.assessment == 'PASSED' for record in assessments)}"
    )
    print(
        f"WEAK         : {sum(record.assessment == 'WEAK' for record in assessments)}"
    )
    print(
        f"FAILED       : {sum(record.assessment == 'FAILED' for record in assessments)}"
    )
    print(f"Elapsed      : {execution.elapsed_seconds:.3f} s")
    print(f"Input offset : {execution.input_file_offset_bytes} bytes")
    print(f"Remaining    : {execution.remaining_input_bytes} bytes")
    print(f"Summary      : {args.summary.expanduser().resolve()}")

    if error_message is not None:
        print()
        print(f"Technical error:\n{error_message}")
        return 1

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
        print(f"GGEMS Dieharder validation failed:\n{error}")
        raise SystemExit(1) from None
