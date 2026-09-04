import argparse
import json
import math
import os
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path
from typing import Literal, Protocol, cast

# ------------------------------------------------------------------------------

type BatteryName = Literal["smallcrush", "crush", "bigcrush"]
type Layout = Literal["interleaved", "worker_major"]
type RunStatus = Literal[
    "technical_error",
    "result_invalid_or_incomplete",
    "statistically_complete_no_testu01_suspects",
    "statistically_complete_with_testu01_suspects",
]
type Classification = Literal["normal", "suspect_low", "suspect_high", "not_applicable"]
type InputTransport = Literal["regular_file", "opencl_pipe"]

CONSUMER_RESULT_SCHEMA = "ggems_testu01_consumer_result"
SUMMARY_SCHEMA = "ggems_testu01_single_case_summary"
SUMMARY_SCHEMA_VERSION = 1
RAW_FILE_PROTOCOL_NAME = "ggems_testu01_1_2_3_fedora_raw_file_v1"
OPENCL_PIPE_PROTOCOL_NAME = "ggems_testu01_1_2_3_fedora_opencl_pipe_v1"
STREAM_REQUEST_SCHEMA = "ggems_testu01_opencl_pipe_request"
STREAM_REQUEST_SCHEMA_VERSION = 1
SUSPECT_LOW = 0.001
SUSPECT_HIGH = 0.999
MAX_TIMEOUT_SECONDS = 2_147_483
WORD_BYTES = 4
TERMINATION_GRACE_SECONDS = 5.0

# ------------------------------------------------------------------------------


class Arguments(Protocol):
    manifest: Path | None
    stream_request: Path | None
    battery: BatteryName
    consumer: str
    producer: str
    summary: Path
    timeout_seconds: int


@dataclass(frozen=True, slots=True)
class BatteryDefinition:
    name: BatteryName
    expected_slot_count: int


@dataclass(frozen=True, slots=True)
class ManifestRecord:
    raw: dict[str, object]
    path: Path
    stream_path: Path
    engine: str
    seed: int
    stream_offset: int
    layout: Layout
    worker_count: int
    samples_per_worker: int
    total_samples: int
    byte_count: int


@dataclass(frozen=True, slots=True)
class StreamingRequestRecord:
    path: Path
    case_id: str | None
    engine: str
    seed: int
    stream_offset: int
    layout: Layout
    worker_count: int
    samples_per_worker: int
    logical_word_capacity: int
    logical_byte_capacity: int
    max_chunk_mib: int
    local_size: int
    device_selector: str


@dataclass(frozen=True, slots=True)
class ConsumerExecution:
    command: tuple[str, ...]
    return_code: int
    elapsed_seconds: float
    stdout: bytes
    stderr: bytes
    result_bytes: bytes | None
    timed_out: bool


@dataclass(frozen=True, slots=True)
class ProducerExecution:
    command: tuple[str, ...]
    return_code: int
    elapsed_seconds: float
    stderr: bytes
    timed_out: bool


@dataclass(frozen=True, slots=True)
class PairExecution:
    producer: ProducerExecution
    consumer: ConsumerExecution | None
    lifecycle_error: str | None


@dataclass(frozen=True, slots=True)
class SlotAssessment:
    index: int
    name: str | None
    p_value_hex: str | None
    p_value_decimal: str | None
    p_value: float | None
    testu01_not_computed: bool
    classification: Classification
    invalid_reason: str | None


@dataclass(frozen=True, slots=True)
class ResultAssessment:
    status: RunStatus
    reasons: tuple[str, ...]
    consumed_word_count: int | None
    consumed_byte_count: int | None
    input_bytes_read: int | None
    reported_slot_count: int | None
    slots: tuple[SlotAssessment, ...]


# ------------------------------------------------------------------------------


def RepositoryRoot() -> Path:
    return Path(__file__).resolve().parents[3]


def DefaultConsumerPath() -> Path:
    return (
        RepositoryRoot()
        / "build"
        / "validation"
        / "random"
        / "testu01"
        / "ggems_testu01_consumer"
    )


def DefaultProducerPath() -> Path:
    return (
        RepositoryRoot()
        / "build"
        / "validation"
        / "random"
        / "ggems_random_stream_pipe_producer"
    )


def ParsePositiveInteger(value: str) -> int:
    try:
        parsed = int(value, 10)
    except ValueError as error:
        raise argparse.ArgumentTypeError("expected a positive integer") from error
    if parsed <= 0 or parsed > MAX_TIMEOUT_SECONDS:
        raise argparse.ArgumentTypeError(
            f"expected an integer in [1, {MAX_TIMEOUT_SECONDS}]"
        )
    return parsed


def ParseArguments() -> Arguments:
    parser = argparse.ArgumentParser(
        description=(
            "Run one TestU01 battery from a GGEMS raw file or OpenCL stream "
            "and write a structured result."
        )
    )
    transport = parser.add_mutually_exclusive_group(required=True)
    _ = transport.add_argument(
        "--manifest", type=Path, help="GGEMS raw uint32 stream manifest."
    )
    _ = transport.add_argument(
        "--stream-request",
        type=Path,
        help="GGEMS OpenCL-pipe stream request.",
    )
    _ = parser.add_argument(
        "--battery",
        choices=("smallcrush", "crush", "bigcrush"),
        required=True,
    )
    _ = parser.add_argument("--consumer", default=str(DefaultConsumerPath()))
    _ = parser.add_argument("--producer", default=str(DefaultProducerPath()))
    _ = parser.add_argument("--summary", type=Path, required=True)
    _ = parser.add_argument(
        "--timeout-seconds", type=ParsePositiveInteger, required=True
    )
    return cast(Arguments, cast(object, parser.parse_args()))


def GetBatteryDefinition(name: BatteryName) -> BatteryDefinition:
    definitions: dict[BatteryName, BatteryDefinition] = {
        "smallcrush": BatteryDefinition("smallcrush", 15),
        "crush": BatteryDefinition("crush", 144),
        "bigcrush": BatteryDefinition("bigcrush", 160),
    }
    return definitions[name]


# ------------------------------------------------------------------------------


def ResolveExecutable(value: str, description: str) -> Path:
    candidate = Path(value).expanduser()
    if candidate.is_file():
        path = candidate.resolve()
    else:
        found = shutil.which(value)
        if found is None:
            raise FileNotFoundError(f"{description} not found: {value}")
        path = Path(found).resolve()
    if not os.access(path, os.X_OK):
        raise PermissionError(f"{description} is not executable: {path}")
    return path


def RequireObject(value: object, description: str) -> dict[str, object]:
    if not isinstance(value, dict):
        raise TypeError(f"{description} must be a JSON object")
    source = cast(dict[object, object], value)
    result: dict[str, object] = {}
    for key, member in source.items():
        if not isinstance(key, str):
            raise TypeError(f"{description} contains a non-string key")
        result[key] = member
    return result


def RequireList(value: object, description: str) -> list[object]:
    if not isinstance(value, list):
        raise TypeError(f"{description} must be a JSON array")
    return cast(list[object], value)


def RequireString(value: object, description: str) -> str:
    if not isinstance(value, str):
        raise TypeError(f"{description} must be a string")
    return value


def RequireInt(value: object, description: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{description} must be an integer")
    return value


def RequireBool(value: object, description: str) -> bool:
    if not isinstance(value, bool):
        raise TypeError(f"{description} must be a Boolean")
    return value


def ParseUnsigned(value: object, description: str) -> int:
    if isinstance(value, bool):
        raise TypeError(f"{description} must be an unsigned integer")
    if isinstance(value, int):
        parsed = value
    elif isinstance(value, str) and value.isascii() and value.isdecimal():
        parsed = int(value, 10)
    else:
        raise TypeError(f"{description} must be an unsigned integer")
    if parsed < 0:
        raise ValueError(f"{description} must not be negative")
    return parsed


def ParseLayout(value: object, description: str) -> Layout:
    text = RequireString(value, description)
    if text == "interleaved":
        return "interleaved"
    if text == "worker_major":
        return "worker_major"
    raise ValueError(f"Unsupported layout: {text}")


def LoadJsonObject(path: Path, description: str) -> dict[str, object]:
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as error:
        raise OSError(f"Cannot read {description}: {path}") from error
    value = cast(object, json.loads(text))
    return RequireObject(value, description)


# ------------------------------------------------------------------------------


def LoadManifest(path: Path) -> ManifestRecord:
    path = path.expanduser().resolve()
    raw = LoadJsonObject(path, "GGEMS manifest")
    random_section = RequireObject(raw.get("random"), "manifest random section")
    output_section = RequireObject(raw.get("output"), "manifest output section")

    engine = RequireString(random_section.get("engine"), "random.engine")
    seed = ParseUnsigned(random_section.get("seed"), "random.seed")
    stream_offset = ParseUnsigned(
        random_section.get("stream_offset"), "random.stream_offset"
    )
    stream_type = RequireString(random_section.get("stream_type"), "random.stream_type")
    sample_bits = RequireInt(random_section.get("sample_bits"), "random.sample_bits")
    layout = ParseLayout(random_section.get("layout"), "random.layout")
    worker_count = ParseUnsigned(random_section.get("worker_count"), "worker_count")
    samples_per_worker = ParseUnsigned(
        random_section.get("samples_per_worker"), "samples_per_worker"
    )
    total_samples = ParseUnsigned(random_section.get("total_samples"), "total_samples")
    byte_count = ParseUnsigned(random_section.get("byte_count"), "byte_count")
    stream_path_text = RequireString(
        output_section.get("stream_path"), "output.stream_path"
    )

    if stream_type != "raw_uint32" or sample_bits != 32:
        raise ValueError("TestU01 requires a raw_uint32 stream")
    if worker_count == 0 or samples_per_worker == 0:
        raise ValueError("GGEMS stream dimensions must be positive")
    if total_samples != worker_count * samples_per_worker:
        raise ValueError("Manifest total_samples is inconsistent")
    if byte_count != total_samples * WORD_BYTES:
        raise ValueError("Manifest byte_count is inconsistent")

    stream_path = Path(stream_path_text).expanduser().resolve()
    if not stream_path.is_file():
        raise FileNotFoundError(f"GGEMS random stream not found: {stream_path}")
    if stream_path.stat().st_size != byte_count:
        raise ValueError("GGEMS random stream size differs from the manifest")

    return ManifestRecord(
        raw=raw,
        path=path,
        stream_path=stream_path,
        engine=engine,
        seed=seed,
        stream_offset=stream_offset,
        layout=layout,
        worker_count=worker_count,
        samples_per_worker=samples_per_worker,
        total_samples=total_samples,
        byte_count=byte_count,
    )


def LoadStreamingRequest(path: Path) -> StreamingRequestRecord:
    path = path.expanduser().resolve()
    raw = LoadJsonObject(path, "OpenCL stream request")

    schema = raw.get("schema")
    if schema is not None and schema != STREAM_REQUEST_SCHEMA:
        raise ValueError("Unsupported OpenCL stream request schema")
    schema_version = raw.get("schema_version")
    if schema_version is not None and schema_version != STREAM_REQUEST_SCHEMA_VERSION:
        raise ValueError("Unsupported OpenCL stream request schema version")
    transport = raw.get("transport")
    if transport is not None and transport != "opencl_pipe":
        raise ValueError("Stream request transport must be opencl_pipe")
    protocol = raw.get("protocol")
    if protocol is not None and protocol != OPENCL_PIPE_PROTOCOL_NAME:
        raise ValueError("Unsupported OpenCL pipe protocol")
    encoding = raw.get("encoding")
    if encoding is not None and encoding != "canonical_little_endian_uint32":
        raise ValueError("Unsupported OpenCL stream encoding")

    case_value = raw.get("case_id")
    case_id = None if case_value is None else RequireString(case_value, "case_id")
    random_section = RequireObject(raw.get("random"), "stream request random")
    opencl_section = RequireObject(raw.get("opencl"), "stream request OpenCL")

    engine = RequireString(random_section.get("engine"), "stream request engine")
    if engine not in {"philox", "pcg32", "jkiss"}:
        raise ValueError("stream request engine must be philox, pcg32, or jkiss")
    seed = ParseUnsigned(random_section.get("seed"), "stream request seed")
    stream_offset = ParseUnsigned(
        random_section.get("stream_offset"), "stream request stream_offset"
    )
    stream_type = random_section.get("stream_type", "raw_uint32")
    sample_bits = random_section.get("sample_bits", 32)
    if stream_type != "raw_uint32" or sample_bits != 32:
        raise ValueError("TestU01 requires a raw_uint32 OpenCL stream")
    layout = ParseLayout(random_section.get("layout"), "stream request layout")
    worker_count = ParseUnsigned(
        random_section.get("worker_count"), "stream request worker_count"
    )
    samples_per_worker = ParseUnsigned(
        random_section.get("samples_per_worker"),
        "stream request samples_per_worker",
    )
    if worker_count == 0 or samples_per_worker == 0:
        raise ValueError("OpenCL stream dimensions must be positive")

    logical_words = worker_count * samples_per_worker
    logical_bytes = logical_words * WORD_BYTES
    declared_words = random_section.get("logical_word_capacity")
    if (
        declared_words is not None
        and ParseUnsigned(declared_words, "logical_word_capacity") != logical_words
    ):
        raise ValueError("logical_word_capacity is inconsistent")
    declared_bytes = random_section.get("logical_byte_capacity")
    if (
        declared_bytes is not None
        and ParseUnsigned(declared_bytes, "logical_byte_capacity") != logical_bytes
    ):
        raise ValueError("logical_byte_capacity is inconsistent")

    max_chunk_mib = ParseUnsigned(opencl_section.get("max_chunk_mib"), "max_chunk_mib")
    local_size = ParseUnsigned(opencl_section.get("local_size"), "local_size")
    device_selector = RequireString(
        opencl_section.get("device_selector"), "device_selector"
    )
    if max_chunk_mib == 0 or local_size == 0 or not device_selector:
        raise ValueError("OpenCL stream parameters must be positive/nonempty")

    return StreamingRequestRecord(
        path=path,
        case_id=case_id,
        engine=engine,
        seed=seed,
        stream_offset=stream_offset,
        layout=layout,
        worker_count=worker_count,
        samples_per_worker=samples_per_worker,
        logical_word_capacity=logical_words,
        logical_byte_capacity=logical_bytes,
        max_chunk_mib=max_chunk_mib,
        local_size=local_size,
        device_selector=device_selector,
    )


# ------------------------------------------------------------------------------


def RunConsumer(
    consumer: Path,
    battery: BatteryDefinition,
    input_path: str,
    timeout_seconds: int,
) -> ConsumerExecution:
    with tempfile.TemporaryDirectory(prefix="ggems-testu01-") as temporary:
        result_path = Path(temporary) / "consumer-result.json"
        command = (
            str(consumer),
            "--battery",
            battery.name,
            "--input",
            input_path,
            "--result",
            str(result_path),
        )
        started = time.monotonic()
        process = subprocess.Popen(
            list(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        timed_out = False
        try:
            stdout, stderr = process.communicate(timeout=timeout_seconds)
        except subprocess.TimeoutExpired:
            timed_out = True
            process.terminate()
            try:
                stdout, stderr = process.communicate(timeout=TERMINATION_GRACE_SECONDS)
            except subprocess.TimeoutExpired:
                process.kill()
                stdout, stderr = process.communicate()
        elapsed = time.monotonic() - started
        result_bytes = result_path.read_bytes() if result_path.is_file() else None
        return_code = process.returncode
        if return_code is None:
            raise RuntimeError("TestU01 consumer did not terminate")

    return ConsumerExecution(
        command=command,
        return_code=return_code,
        elapsed_seconds=elapsed,
        stdout=stdout,
        stderr=stderr,
        result_bytes=result_bytes,
        timed_out=timed_out,
    )


def BuildProducerCommand(
    producer: Path, request: StreamingRequestRecord
) -> tuple[str, ...]:
    return (
        str(producer),
        "--engine",
        request.engine,
        "--seed",
        str(request.seed),
        "--stream-offset",
        str(request.stream_offset),
        "--workers",
        str(request.worker_count),
        "--samples-per-worker",
        str(request.samples_per_worker),
        "--max-chunk-mib",
        str(request.max_chunk_mib),
        "--local-size",
        str(request.local_size),
        "--device",
        request.device_selector,
        "--layout",
        request.layout,
        "--output-word-limit",
        str(request.logical_word_capacity),
    )


def StopProcess(process: subprocess.Popen[bytes] | None) -> None:
    if process is None or process.poll() is not None:
        return
    process.terminate()
    try:
        _ = process.wait(timeout=TERMINATION_GRACE_SECONDS)
    except subprocess.TimeoutExpired:
        process.kill()
        _ = process.wait()


def RunStreamingPair(
    producer: Path,
    consumer: Path,
    request: StreamingRequestRecord,
    battery: BatteryDefinition,
    timeout_seconds: int,
) -> PairExecution:
    with tempfile.TemporaryDirectory(prefix="ggems-testu01-stream-") as temporary:
        temporary_path = Path(temporary)
        consumer_result_path = temporary_path / "consumer-result.json"
        producer_stderr_path = temporary_path / "producer-stderr.txt"
        consumer_stdout_path = temporary_path / "consumer-stdout.txt"
        consumer_stderr_path = temporary_path / "consumer-stderr.txt"

        producer_command = BuildProducerCommand(producer, request)
        consumer_command = (
            str(consumer),
            "--battery",
            battery.name,
            "--input",
            "-",
            "--result",
            str(consumer_result_path),
        )

        started = time.monotonic()
        deadline = started + timeout_seconds
        timed_out = False
        lifecycle_error: str | None = None
        consumer_process: subprocess.Popen[bytes] | None = None

        with (
            producer_stderr_path.open("wb") as producer_stderr,
            consumer_stdout_path.open("wb") as consumer_stdout,
            consumer_stderr_path.open("wb") as consumer_stderr,
        ):
            producer_process = subprocess.Popen(
                list(producer_command),
                stdout=subprocess.PIPE,
                stderr=producer_stderr,
            )
            producer_stdout = producer_process.stdout
            if producer_stdout is None:
                StopProcess(producer_process)
                raise RuntimeError("Producer stdout pipe was not created")

            try:
                consumer_process = subprocess.Popen(
                    list(consumer_command),
                    stdin=producer_stdout,
                    stdout=consumer_stdout,
                    stderr=consumer_stderr,
                )
                producer_stdout.close()

                while (
                    producer_process.poll() is None or consumer_process.poll() is None
                ):
                    if time.monotonic() >= deadline:
                        timed_out = True
                        StopProcess(consumer_process)
                        StopProcess(producer_process)
                        break
                    time.sleep(0.05)
            except (OSError, ValueError, subprocess.SubprocessError) as error:
                lifecycle_error = f"Streaming supervision failed: {error}"
                producer_stdout.close()
                StopProcess(consumer_process)
                StopProcess(producer_process)

        elapsed = time.monotonic() - started
        producer_return_code = producer_process.poll()
        if producer_return_code is None:
            StopProcess(producer_process)
            producer_return_code = producer_process.returncode
        if producer_return_code is None:
            raise RuntimeError("OpenCL producer did not terminate")

        consumer_execution: ConsumerExecution | None = None
        if consumer_process is not None:
            consumer_return_code = consumer_process.poll()
            if consumer_return_code is None:
                StopProcess(consumer_process)
                consumer_return_code = consumer_process.returncode
            if consumer_return_code is not None:
                consumer_execution = ConsumerExecution(
                    command=consumer_command,
                    return_code=consumer_return_code,
                    elapsed_seconds=elapsed,
                    stdout=consumer_stdout_path.read_bytes(),
                    stderr=consumer_stderr_path.read_bytes(),
                    result_bytes=(
                        consumer_result_path.read_bytes()
                        if consumer_result_path.is_file()
                        else None
                    ),
                    timed_out=timed_out,
                )

        producer_execution = ProducerExecution(
            command=producer_command,
            return_code=producer_return_code,
            elapsed_seconds=elapsed,
            stderr=producer_stderr_path.read_bytes(),
            timed_out=timed_out,
        )

    return PairExecution(
        producer=producer_execution,
        consumer=consumer_execution,
        lifecycle_error=lifecycle_error,
    )


# ------------------------------------------------------------------------------


def ParseConsumerResult(data: bytes | None) -> dict[str, object] | None:
    if data is None:
        return None
    value = cast(object, json.loads(data.decode("utf-8")))
    return RequireObject(value, "consumer result")


def AssessSlot(value: object, expected_index: int) -> SlotAssessment:
    try:
        slot = RequireObject(value, f"slot {expected_index}")
        index = RequireInt(slot.get("index"), f"slot {expected_index} index")
        name_value = slot.get("name")
        name = None if name_value is None else RequireString(name_value, "slot name")
        p_value_hex = RequireString(slot.get("p_value_hex"), "p_value_hex")
        p_value_decimal = RequireString(slot.get("p_value_decimal"), "p_value_decimal")
        not_computed = RequireBool(
            slot.get("testu01_not_computed"), "testu01_not_computed"
        )
        p_value = float.fromhex(p_value_hex)
        decimal_value = float(p_value_decimal)
    except (KeyError, TypeError, ValueError, OverflowError) as error:
        return SlotAssessment(
            index=expected_index,
            name=None,
            p_value_hex=None,
            p_value_decimal=None,
            p_value=None,
            testu01_not_computed=False,
            classification="not_applicable",
            invalid_reason=str(error),
        )

    reasons: list[str] = []
    if index != expected_index:
        reasons.append(f"index={index}, expected {expected_index}")
    if name is None or not name:
        reasons.append("missing TestU01 display name")
    if not math.isfinite(p_value) or not math.isfinite(decimal_value):
        reasons.append("p-value is not finite")
    elif p_value != decimal_value:
        reasons.append("hexadecimal and decimal p-values disagree")

    if p_value == -1.0 or not_computed:
        if p_value != -1.0 or not not_computed:
            reasons.append("not-computed marker and -1.0 sentinel disagree")
        return SlotAssessment(
            index=index,
            name=name,
            p_value_hex=p_value_hex,
            p_value_decimal=p_value_decimal,
            p_value=None,
            testu01_not_computed=True,
            classification="not_applicable",
            invalid_reason=(
                "; ".join(reasons) if reasons else "TestU01 did not compute this slot"
            ),
        )

    if not 0.0 <= p_value <= 1.0:
        reasons.append("p-value is outside [0, 1]")

    classification: Classification = "normal"
    if not reasons:
        if p_value < SUSPECT_LOW:
            classification = "suspect_low"
        elif p_value > SUSPECT_HIGH:
            classification = "suspect_high"

    return SlotAssessment(
        index=index,
        name=name,
        p_value_hex=p_value_hex,
        p_value_decimal=p_value_decimal,
        p_value=p_value if not reasons else None,
        testu01_not_computed=False,
        classification=classification if not reasons else "not_applicable",
        invalid_reason="; ".join(reasons) if reasons else None,
    )


def AssessConsumerResult(
    root: dict[str, object] | None, battery: BatteryDefinition
) -> ResultAssessment:
    if root is None:
        return TechnicalAssessment("Consumer result is unavailable")

    try:
        if (
            RequireString(root.get("schema"), "consumer schema")
            != CONSUMER_RESULT_SCHEMA
        ):
            raise ValueError("unsupported consumer result schema")
        if not RequireBool(root.get("terminal"), "consumer terminal"):
            raise ValueError("consumer result is nonterminal")
        if RequireString(root.get("battery"), "consumer battery") != battery.name:
            raise ValueError("consumer battery differs from request")

        consumed_words = ParseUnsigned(
            root.get("consumed_word_count"), "consumed_word_count"
        )
        consumed_bytes = ParseUnsigned(
            root.get("consumed_byte_count"), "consumed_byte_count"
        )
        input_bytes_read = ParseUnsigned(
            root.get("input_bytes_read"), "input_bytes_read"
        )
        if consumed_bytes != consumed_words * WORD_BYTES:
            raise ValueError("consumer word/byte counters disagree")
        if consumed_bytes > input_bytes_read:
            raise ValueError("consumer consumed more bytes than it read")

        consumer_status = RequireString(root.get("consumer_status"), "consumer_status")
        if consumer_status == "technical_error":
            error = RequireObject(root.get("error"), "consumer error")
            kind = RequireString(error.get("kind"), "consumer error kind")
            message = RequireString(error.get("message"), "consumer error message")
            return TechnicalAssessment(
                f"{kind}: {message}",
                consumed_word_count=consumed_words,
                consumed_byte_count=consumed_bytes,
                input_bytes_read=input_bytes_read,
            )
        if consumer_status != "battery_returned":
            raise ValueError(f"unknown consumer status: {consumer_status}")

        reported_slot_count = RequireInt(
            root.get("reported_slot_count"), "reported_slot_count"
        )
        copied_slot_count = RequireInt(
            root.get("copied_slot_count"), "copied_slot_count"
        )
        collection_status = RequireString(
            root.get("result_collection_status"), "result_collection_status"
        )
        slot_values = RequireList(root.get("slots"), "consumer slots")
    except (KeyError, TypeError, ValueError) as error:
        return TechnicalAssessment(f"Consumer result protocol error: {error}")

    reasons: list[str] = []
    expected = battery.expected_slot_count
    if reported_slot_count != expected:
        reasons.append(f"reported {reported_slot_count} slots, expected {expected}")
    if copied_slot_count != len(slot_values):
        reasons.append("copied_slot_count differs from serialized slot count")
    if len(slot_values) != expected:
        reasons.append(f"serialized {len(slot_values)} slots, expected {expected}")
    if collection_status != "complete":
        reasons.append(f"result_collection_status={collection_status}")

    slots = tuple(AssessSlot(value, index) for index, value in enumerate(slot_values))
    for slot in slots:
        if slot.invalid_reason is not None:
            reasons.append(f"slot {slot.index}: {slot.invalid_reason}")

    if reasons:
        status: RunStatus = "result_invalid_or_incomplete"
    elif any(slot.classification in {"suspect_low", "suspect_high"} for slot in slots):
        status = "statistically_complete_with_testu01_suspects"
    else:
        status = "statistically_complete_no_testu01_suspects"

    return ResultAssessment(
        status=status,
        reasons=tuple(reasons),
        consumed_word_count=consumed_words,
        consumed_byte_count=consumed_bytes,
        input_bytes_read=input_bytes_read,
        reported_slot_count=reported_slot_count,
        slots=slots,
    )


def TechnicalAssessment(
    reason: str,
    *,
    consumed_word_count: int | None = None,
    consumed_byte_count: int | None = None,
    input_bytes_read: int | None = None,
) -> ResultAssessment:
    return ResultAssessment(
        status="technical_error",
        reasons=(reason,),
        consumed_word_count=consumed_word_count,
        consumed_byte_count=consumed_byte_count,
        input_bytes_read=input_bytes_read,
        reported_slot_count=None,
        slots=(),
    )


def MarkTechnical(assessment: ResultAssessment, reason: str) -> ResultAssessment:
    return ResultAssessment(
        status="technical_error",
        reasons=(*assessment.reasons, reason),
        consumed_word_count=assessment.consumed_word_count,
        consumed_byte_count=assessment.consumed_byte_count,
        input_bytes_read=assessment.input_bytes_read,
        reported_slot_count=assessment.reported_slot_count,
        slots=assessment.slots,
    )


# ------------------------------------------------------------------------------


def DecodeText(data: bytes) -> str:
    return data.decode("utf-8", errors="replace")


def SlotToJson(slot: SlotAssessment) -> dict[str, object]:
    return {
        "index": slot.index,
        "name": slot.name,
        "p_value_hex": slot.p_value_hex,
        "p_value_decimal": slot.p_value_decimal,
        "p_value": slot.p_value,
        "testu01_not_computed": slot.testu01_not_computed,
        "classification": slot.classification,
        "invalid_reason": slot.invalid_reason,
    }


def BuildOutput(assessment: ResultAssessment) -> dict[str, object]:
    return {
        "status": assessment.status,
        "reasons": list(assessment.reasons),
        "reported_slot_count": assessment.reported_slot_count,
        "observed_slot_count": len(assessment.slots),
        "suspect_low_count": sum(
            slot.classification == "suspect_low" for slot in assessment.slots
        ),
        "suspect_high_count": sum(
            slot.classification == "suspect_high" for slot in assessment.slots
        ),
        "results": [SlotToJson(slot) for slot in assessment.slots],
    }


def WriteSummary(path: Path, summary: dict[str, object]) -> None:
    path = path.expanduser().resolve()
    path.parent.mkdir(parents=True, exist_ok=True)
    _ = path.write_text(
        json.dumps(summary, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
    )


def BaseSummary(
    battery: BatteryDefinition,
    transport: InputTransport,
    assessment: ResultAssessment,
) -> dict[str, object]:
    return {
        "schema": SUMMARY_SCHEMA,
        "schema_version": SUMMARY_SCHEMA_VERSION,
        "created_at_utc": datetime.now(UTC).isoformat(),
        "tool": {
            "name": "TestU01",
            "battery": battery.name,
            "suspect_rule": "p < 0.001 or p > 0.999",
            "interpretation": (
                "TestU01 is a statistical assessment and does not prove randomness."
            ),
        },
        "transport": transport,
        "consumption": {
            "word_count": assessment.consumed_word_count,
            "byte_count": assessment.consumed_byte_count,
            "input_bytes_read": assessment.input_bytes_read,
        },
        "output": BuildOutput(assessment),
    }


# ------------------------------------------------------------------------------


def ExecuteRegularFile(args: Arguments) -> tuple[dict[str, object], RunStatus]:
    if args.manifest is None:
        raise ValueError("--manifest is required for the regular-file backend")

    manifest = LoadManifest(args.manifest)
    battery = GetBatteryDefinition(args.battery)
    consumer = ResolveExecutable(args.consumer, "TestU01 consumer")
    execution = RunConsumer(
        consumer, battery, str(manifest.stream_path), args.timeout_seconds
    )

    raw_result: dict[str, object] | None = None
    reasons: list[str] = []
    if execution.timed_out:
        reasons.append("TestU01 consumer timed out")
    if execution.return_code != 0:
        reasons.append(f"TestU01 consumer exited with {execution.return_code}")
    if execution.result_bytes is None:
        reasons.append("TestU01 consumer result is missing")
    else:
        try:
            raw_result = ParseConsumerResult(execution.result_bytes)
        except (
            UnicodeDecodeError,
            json.JSONDecodeError,
            TypeError,
            ValueError,
        ) as error:
            reasons.append(f"Cannot parse TestU01 consumer result: {error}")

    assessment = AssessConsumerResult(raw_result, battery)
    for reason in reasons:
        assessment = MarkTechnical(assessment, reason)

    summary = BaseSummary(battery, "regular_file", assessment)
    summary["protocol"] = RAW_FILE_PROTOCOL_NAME
    summary["input"] = {
        "manifest_path": str(manifest.path),
        "manifest": manifest.raw,
        "stream_path": str(manifest.stream_path),
        "engine": manifest.engine,
        "seed": manifest.seed,
        "stream_offset": manifest.stream_offset,
        "layout": manifest.layout,
        "worker_count": manifest.worker_count,
        "samples_per_worker": manifest.samples_per_worker,
        "total_samples": manifest.total_samples,
        "byte_count": manifest.byte_count,
    }
    summary["consumer"] = {
        "path": str(consumer),
        "command": list(execution.command),
        "return_code": execution.return_code,
        "elapsed_seconds": execution.elapsed_seconds,
        "timed_out": execution.timed_out,
    }
    summary["consumer_result"] = raw_result
    summary["testu01_stdout"] = DecodeText(execution.stdout)
    summary["testu01_stderr"] = DecodeText(execution.stderr)
    WriteSummary(args.summary, summary)
    return summary, assessment.status


def ExecuteStreaming(args: Arguments) -> tuple[dict[str, object], RunStatus]:
    if args.stream_request is None:
        raise ValueError("--stream-request is required for the OpenCL backend")

    request = LoadStreamingRequest(args.stream_request)
    battery = GetBatteryDefinition(args.battery)
    producer = ResolveExecutable(args.producer, "GGEMS OpenCL stream producer")
    consumer = ResolveExecutable(args.consumer, "TestU01 consumer")
    pair = RunStreamingPair(producer, consumer, request, battery, args.timeout_seconds)

    reasons: list[str] = []
    if pair.lifecycle_error is not None:
        reasons.append(pair.lifecycle_error)
    if pair.producer.timed_out:
        reasons.append("Streaming producer/consumer pair timed out")
    if pair.consumer is None:
        reasons.append("TestU01 consumer was not started or did not terminate")
        raw_result = None
    else:
        if pair.consumer.return_code != 0:
            reasons.append(f"TestU01 consumer exited with {pair.consumer.return_code}")
        try:
            raw_result = ParseConsumerResult(pair.consumer.result_bytes)
        except (
            UnicodeDecodeError,
            json.JSONDecodeError,
            TypeError,
            ValueError,
        ) as error:
            raw_result = None
            reasons.append(f"Cannot parse TestU01 consumer result: {error}")

    assessment = AssessConsumerResult(raw_result, battery)

    # Exit 3 is retained for compatibility with the qualified Phase 2 producer,
    # which used it for the normal EPIPE generated when TestU01 finishes first.
    producer_exit_is_normal = pair.producer.return_code in {0, 3}
    if not producer_exit_is_normal:
        reasons.append(f"OpenCL producer exited with {pair.producer.return_code}")
    if pair.producer.return_code == 3 and not assessment.status.startswith(
        "statistically_complete_"
    ):
        reasons.append("OpenCL producer closed before a complete TestU01 result")

    for reason in reasons:
        assessment = MarkTechnical(assessment, reason)

    summary = BaseSummary(battery, "opencl_pipe", assessment)
    summary["protocol"] = OPENCL_PIPE_PROTOCOL_NAME
    summary["input"] = {
        "stream_request_path": str(request.path),
        "case_id": request.case_id,
        "engine": request.engine,
        "seed": request.seed,
        "stream_offset": request.stream_offset,
        "layout": request.layout,
        "worker_count": request.worker_count,
        "samples_per_worker": request.samples_per_worker,
        "logical_word_capacity": request.logical_word_capacity,
        "logical_byte_capacity": request.logical_byte_capacity,
        "max_chunk_mib": request.max_chunk_mib,
        "local_size": request.local_size,
        "device_selector": request.device_selector,
    }
    summary["producer"] = {
        "path": str(producer),
        "command": list(pair.producer.command),
        "return_code": pair.producer.return_code,
        "elapsed_seconds": pair.producer.elapsed_seconds,
        "timed_out": pair.producer.timed_out,
        "stderr": DecodeText(pair.producer.stderr),
    }
    if pair.consumer is None:
        summary["consumer"] = None
        summary["consumer_result"] = None
        summary["testu01_stdout"] = ""
        summary["testu01_stderr"] = ""
    else:
        summary["consumer"] = {
            "path": str(consumer),
            "command": list(pair.consumer.command),
            "return_code": pair.consumer.return_code,
            "elapsed_seconds": pair.consumer.elapsed_seconds,
            "timed_out": pair.consumer.timed_out,
        }
        summary["consumer_result"] = raw_result
        summary["testu01_stdout"] = DecodeText(pair.consumer.stdout)
        summary["testu01_stderr"] = DecodeText(pair.consumer.stderr)

    WriteSummary(args.summary, summary)
    return summary, assessment.status


# ------------------------------------------------------------------------------


def Execute(args: Arguments) -> tuple[dict[str, object], RunStatus]:
    if args.manifest is not None:
        return ExecuteRegularFile(args)
    return ExecuteStreaming(args)


def main() -> int:
    args = ParseArguments()
    summary_path = args.summary.expanduser().resolve()

    try:
        summary, status = Execute(args)
    except (
        FileNotFoundError,
        json.JSONDecodeError,
        OSError,
        RuntimeError,
        TypeError,
        ValueError,
    ) as error:
        print(f"GGEMS TestU01 validation failed:\n{error}", file=sys.stderr)
        return 1

    output = RequireObject(summary.get("output"), "summary output")
    observed_slots = RequireInt(output.get("observed_slot_count"), "observed slots")
    print("GGEMS TestU01 single-case validation")
    print(f"Battery : {args.battery}")
    print(f"Status  : {status}")
    print(f"Slots   : {observed_slots}")
    print(f"Summary : {summary_path}")
    return 0 if status.startswith("statistically_complete_") else 1


# ------------------------------------------------------------------------------

if __name__ == "__main__":
    raise SystemExit(main())
