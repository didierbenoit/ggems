import argparse
import csv
import json
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

# Direct script entry points; Python adds this directory to sys.path.
from cases import (  # pyright: ignore[reportImplicitRelativeImport]
    CASES,
    MONO_ENERGY_MEV,
    TimeCase,
)

type JsonObject = dict[str, object]

UINT64_MAX = (1 << 64) - 1
UINT32_MAX = (1 << 32) - 1
CSV_COLUMNS = (
    "source_index",
    "source_local_primary_id",
    "global_primary_id",
    "x_pm",
    "y_pm",
    "z_pm",
    "direction_x",
    "direction_y",
    "direction_z",
    "energy_meV",
    "time_ps",
    "weight",
    "record_kind",
)


@dataclass(frozen=True, slots=True)
class RunMetadata:
    sequence_index: int
    run_id: int
    samples_path: Path
    global_begin: int
    global_last: int
    start_ps: int
    stop_ps: int
    raw: JsonObject


@dataclass(frozen=True, slots=True)
class Metadata:
    case: TimeCase
    primary_count: int
    display_unit_ps: int
    runs: tuple[RunMetadata, ...]
    raw: JsonObject


def _object(value: object, name: str) -> JsonObject:
    if not isinstance(value, dict):
        raise TypeError(f"{name} must be a JSON object.")
    return cast(JsonObject, value)


def _list(value: object, name: str) -> list[object]:
    if not isinstance(value, list):
        raise TypeError(f"{name} must be a JSON array.")
    return cast(list[object], value)


def _integer(
    value: object, name: str, minimum: int = 0, maximum: int = UINT64_MAX
) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{name} must be an integer.")
    if not minimum <= value <= maximum:
        raise ValueError(f"{name} is outside its integer field range.")
    return value


def _number(value: object, name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"{name} must be numeric.")
    number = float(value)
    if not math.isfinite(number):
        raise ValueError(f"{name} must be finite.")
    return number


def _integers(value: object, name: str) -> tuple[int, ...]:
    return tuple(_integer(item, name) for item in _list(value, name))


def _numbers(value: object, name: str) -> tuple[float, ...]:
    return tuple(_number(item, name) for item in _list(value, name))


def _validate_source(raw: JsonObject, case: TimeCase, primary_count: int) -> None:
    for key, expected in {
        "case_name": case.name,
        "geometry": "point",
        "angular_configuration": "fixed",
        "angular_mode": "Fixed",
        "population_mode": "CountDriven",
        "rng_engine": "Philox",
        "particle": "Gamma",
        "energy_configuration": "mono",
        "energy_mode": "Mono",
        "chronology": case.chronology,
    }.items():
        if raw.get(key) != expected:
            raise ValueError(f"T1 metadata mismatch: {key} must be {expected}.")

    if _integer(raw.get("primary_count"), "primary_count") != primary_count:
        raise ValueError("Per-Run primary count differs from the sequence count.")
    if _integer(raw.get("source_index"), "source_index") != 0:
        raise ValueError("T1 requires source slot zero.")
    for key in ("source_center_pm", "dimensions_pm"):
        if _integers(raw.get(key), key) != (0, 0, 0):
            raise ValueError(f"T1 requires {key} == [0, 0, 0].")
    if _numbers(raw.get("dimensions_mm"), "dimensions_mm") != (0, 0, 0):
        raise ValueError("Point dimensions must be zero.")
    if _numbers(raw.get("fixed_direction"), "fixed_direction") != (0, 0, 1):
        raise ValueError("T1 requires exact stored Fixed +Z.")
    axes = tuple(
        _numbers(axis, "frame axis") for axis in _list(raw.get("frame_axes"), "axes")
    )
    if axes != ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
        raise ValueError("T1 requires the identity frame.")
    if _number(raw.get("weight"), "weight") != 1:
        raise ValueError("T1 requires weight 1.")
    if _integer(raw.get("energy_meV"), "energy_meV") != MONO_ENERGY_MEV:
        raise ValueError("T1 requires exactly 511000000 meV.")

    energy = _object(raw.get("energy"), "energy")
    if energy.get("representation") != "uint64 meV":
        raise ValueError("Mono energy authority must be canonical uint64 meV.")
    for key, expected in {
        "distribution_type": 1,
        "table_offset": 0,
        "table_count": 0,
        "regular_bin_width_meV": 0,
        "mono_energy_meV": MONO_ENERGY_MEV,
    }.items():
        if _integer(energy.get(key), key) != expected:
            raise ValueError(f"T1 packed Mono metadata mismatch: {key}.")
    for key in (
        "energy_values_meV",
        "relative_weights",
        "cumulative_ticket_upper_bounds",
    ):
        if _list(energy.get(key), key):
            raise ValueError("T1 Mono must have no energy table.")

    _ = _integer(raw.get("worker_count"), "worker_count", 1, UINT32_MAX)
    _ = _integer(raw.get("seed"), "seed")
    selector = raw.get("device_selector")
    if not isinstance(selector, str) or not selector:
        raise ValueError("Metadata must contain a nonempty device selector.")
    names = _list(raw.get("device_names"), "device_names")
    if not names or any(not isinstance(name, str) or not name for name in names):
        raise ValueError("Metadata must identify the selected devices.")

    observer = _object(raw.get("observer"), "observer")
    for key, expected in {
        "overflow_count": 0,
        "record_count": 2 * primary_count,
        "captured_primary_count": primary_count,
        "source_record_count": primary_count,
    }.items():
        if _integer(observer.get(key), key) != expected:
            raise ValueError(f"Incomplete Observer capture: {key} must be {expected}.")
    for key in ("capacity_per_device", "host_capacity"):
        _ = _integer(observer.get(key), key, 2 * primary_count, UINT32_MAX)


def load_metadata(path: Path) -> Metadata:
    raw = _object(cast(object, json.loads(path.read_text(encoding="utf-8"))), str(path))
    case = next((item for item in CASES if item.name == raw.get("case_name")), None)
    if case is None:
        raise ValueError("Unknown canonical T1 case in metadata.")
    if raw.get("chronology_mode") != case.chronology:
        raise ValueError("Chronology mode differs from the canonical case.")
    if raw.get("time_representation") != "uint64 ps":
        raise ValueError("Time authority must be canonical uint64 ps.")

    count = _integer(
        raw.get("primary_count_per_run"), "primary count", 1, UINT32_MAX // 2
    )
    length = _integer(raw.get("sequence_length"), "sequence_length", 1, UINT32_MAX)
    if length != len(case.windows_ps):
        raise ValueError("Sequence length differs from the canonical T1 case.")
    reset = raw.get("reset_before_sequence_index")
    if reset is not None:
        reset = _integer(reset, "reset_before_sequence_index", 1, length - 1)
    if reset != case.reset_before_run:
        raise ValueError("Reset operation position differs from the canonical case.")

    if case.configured_ps is None:
        if (
            raw.get("configured_time_ps") is not None
            or raw.get("requested_time_ns") is not None
        ):
            raise ValueError("Static chronology must have no configured time interval.")
    else:
        if (
            _integers(raw.get("configured_time_ps"), "configured_time_ps")
            != case.configured_ps
        ):
            raise ValueError(
                "Canonical time configuration differs from the T1 fixture."
            )
        if (
            _numbers(raw.get("requested_time_ns"), "requested_time_ns")
            != case.requested_ns
        ):
            raise ValueError("Requested chronology differs from the T1 fixture.")

    # Display conversion is exported through central Units; never used to
    # convert observed times before the exact integer comparisons below.
    display_unit = _integer(raw.get("display_unit_ps"), "display_unit_ps", 1)
    if display_unit != 1000:
        raise ValueError("The exported ns display unit must be exactly 1000 ps.")

    entries = _list(raw.get("runs"), "runs")
    if len(entries) != length:
        raise ValueError("Missing or extra logical Run metadata.")

    runs: list[RunMetadata] = []
    paths: set[Path] = set()
    for index, entry in enumerate(entries):
        current = _object(entry, f"runs[{index}]")
        _validate_source(current, case, count)
        if _integer(current.get("sequence_index"), "sequence_index") != index:
            raise ValueError("Per-Run sequence metadata is unordered or duplicated.")

        run_id = _integer(current.get("run_id"), "run_id")
        begin = _integer(current.get("global_primary_begin"), "global_primary_begin")
        last = _integer(current.get("global_primary_last"), "global_primary_last")
        if last - begin + 1 != count:
            raise ValueError("Per-Run global primary range has the wrong size.")
        if runs:
            if begin != runs[-1].global_last + 1:
                raise ValueError(
                    "Global primary ranges overlap or do not continue contiguously."
                )
            if run_id != runs[-1].run_id + 1:
                raise ValueError(
                    "Run ids must continue across every successful nonempty Run."
                )
            for key in ("worker_count", "seed", "device_selector", "device_names"):
                if current.get(key) != runs[0].raw.get(key):
                    raise ValueError(
                        f"Sequence execution configuration changed: {key}."
                    )

        expected_start, expected_stop = case.windows_ps[index]
        start = _integer(current.get("snapshot_time_start_ps"), "snapshot start")
        stop = _integer(current.get("snapshot_time_stop_ps"), "snapshot stop")
        if start != expected_start or stop != expected_stop:
            raise ValueError(
                f"Run {index}: committed Source snapshot has the wrong window."
            )
        if (
            _integer(current.get("effective_start_ps"), "effective start") != start
            or _integer(current.get("effective_stop_ps"), "effective stop") != stop
            or _integer(current.get("time_ps"), "time_ps") != start
        ):
            raise ValueError(
                "Run window and committed Source-record time metadata disagree."
            )

        filename = current.get("samples_file")
        if not isinstance(filename, str) or not filename:
            raise ValueError("Each logical Run must identify its CSV file.")
        relative = Path(filename)
        sample_path = (path.parent / relative).resolve()
        if relative.is_absolute() or not sample_path.is_relative_to(
            path.parent.resolve()
        ):
            raise ValueError(
                "Per-Run samples must be relative files within the case directory."
            )
        if sample_path in paths or not sample_path.is_file():
            raise ValueError("Missing or reused logical Run CSV data.")
        paths.add(sample_path)
        runs.append(
            RunMetadata(index, run_id, sample_path, begin, last, start, stop, current)
        )

    return Metadata(case, count, display_unit, tuple(runs), raw)


def _csv_integer(text: str, name: str, minimum: int = 0) -> int:
    digits = text.removeprefix("-")
    if not digits or not digits.isascii() or not digits.isdecimal():
        raise ValueError(f"{name} must be an exact decimal integer in the CSV.")
    return _integer(int(text), name, minimum)


def analyze_run(run: RunMetadata, metadata: Metadata) -> JsonObject:
    times: list[int] = []
    local_ids: set[int] = set()
    global_ids: set[int] = set()

    with run.samples_path.open(newline="", encoding="utf-8") as stream:
        reader = csv.reader(stream, strict=True)
        if tuple(next(reader, ())) != CSV_COLUMNS:
            raise ValueError("CSV columns differ from the stable Source export schema.")

        for row_number, row in enumerate(reader, start=2):
            if len(row) != len(CSV_COLUMNS):
                raise ValueError(f"Malformed CSV row {row_number}: wrong field count.")
            if row[12] != "Source":
                raise ValueError("Only Source records belong in T1 sample CSVs.")

            slot = _csv_integer(row[0], "source_index")
            local = _csv_integer(row[1], "source_local_primary_id")
            global_id = _csv_integer(row[2], "global_primary_id")
            if slot != 0 or local >= metadata.primary_count:
                raise ValueError("Source provenance is outside the configured range.")
            if local in local_ids or global_id in global_ids:
                raise ValueError("Duplicate Source provenance within one Run.")
            if global_id != run.global_begin + local:
                raise ValueError("Source-local and global primary provenance disagree.")
            local_ids.add(local)
            global_ids.add(global_id)

            position = tuple(
                _csv_integer(field, "position", -(1 << 63)) for field in row[3:6]
            )
            if position != (0, 0, 0):
                raise ValueError("T1 Point position must equal the origin exactly.")
            direction = tuple(float(field) for field in row[6:9])
            if not all(math.isfinite(value) for value in direction) or direction != (
                0,
                0,
                1,
            ):
                raise ValueError("T1 direction must equal Fixed +Z exactly.")
            energy = _csv_integer(row[9], "energy_meV")
            birth = _csv_integer(row[10], "time_ps")
            if energy != MONO_ENERGY_MEV or float(row[11]) != 1:
                raise ValueError("T1 requires exact Mono 511000000 meV and weight 1.")
            times.append(birth)

    if len(times) != metadata.primary_count:
        raise ValueError("Missing or extra Source records in one logical Run.")
    # Uniqueness and bounds above imply complete local coverage. Append order
    # is irrelevant: no sorting, filling, or repair of malformed input is needed.
    if min(global_ids) != run.global_begin or max(global_ids) != run.global_last:
        raise ValueError("CSV provenance range disagrees with its actual Run metadata.")

    expected_start, expected_stop = metadata.case.windows_ps[run.sequence_index]
    mismatch_count = sum(value != expected_start for value in times)
    maximum_difference = max(abs(value - expected_start) for value in times)
    if mismatch_count:
        raise ValueError(
            f"Run {run.sequence_index}: {mismatch_count}/{len(times)} births differ "
            + f"from {expected_start} ps; distinct times={len(set(times))}; "
            + f"maximum absolute difference={maximum_difference} ps."
        )

    return {
        "sequence_index": run.sequence_index,
        "run_id": run.run_id,
        "sample_count": len(times),
        "structural_validation": "valid",
        "snapshot_validation": {
            "expected_start_ps": expected_start,
            "expected_stop_ps": expected_stop,
            "actual_snapshot_start_ps": run.start_ps,
            "actual_snapshot_stop_ps": run.stop_ps,
            "snapshot_start_difference_ps": run.start_ps - expected_start,
            "snapshot_stop_difference_ps": run.stop_ps - expected_stop,
        },
        "birth_time_validation": {
            "sample_count": len(times),
            "expected_birth_time_ps": expected_start,
            "minimum_birth_time_ps": min(times),
            "maximum_birth_time_ps": max(times),
            "distinct_birth_time_count": len(set(times)),
            "mismatching_birth_time_count": mismatch_count,
            "maximum_absolute_birth_time_difference_ps": maximum_difference,
        },
        "provenance_range": {
            "first_global_primary_id": min(global_ids),
            "last_global_primary_id": max(global_ids),
            "first_source_local_primary_id": min(local_ids),
            "last_source_local_primary_id": max(local_ids),
        },
    }


def analyze_case(
    metadata_path: Path, output: Path, *, make_figures: bool = True
) -> JsonObject:
    # Validation precedes all output writes; stale summaries are never reused.
    if (output / "summary.json").exists():
        raise FileExistsError(
            "Use a new analysis directory; summary.json already exists."
        )
    metadata = load_metadata(metadata_path)
    results = [analyze_run(run, metadata) for run in metadata.runs]

    reset_validation: JsonObject | None = None
    reset = metadata.case.reset_before_run
    if reset is not None:
        before = metadata.runs[reset - 1]
        after = metadata.runs[reset]
        reset_validation = {
            "reset_before_sequence_index": reset,
            "birth_time_before_reset_ps": before.start_ps,
            "birth_time_after_reset_ps": after.start_ps,
            "chronology_returned_to_configured_start": after.start_ps
            == metadata.runs[0].start_ps,
            "global_primary_ids_continue": after.global_begin == before.global_last + 1,
            "run_ids_continue": after.run_id == before.run_id + 1,
            "interpretation": "ResetTime rewinds chronology; it is not simulation replay.",
        }

    output.mkdir(parents=True, exist_ok=True)
    figures: list[str] = []
    figure_status = "not_required_for_static"
    if metadata.case.chronology == "configured":
        figure_status = "disabled_by_request"
        if make_figures:
            from plot import (  # pyright: ignore[reportImplicitRelativeImport]
                plot_chronology,
            )

            figures = plot_chronology(
                metadata.case,
                tuple((run.start_ps, run.stop_ps) for run in metadata.runs),
                metadata.display_unit_ps,
                output,
            )
            figure_status = "generated"

    summary: JsonObject = {
        "case_name": metadata.case.name,
        "sample_count": metadata.primary_count * len(metadata.runs),
        "primary_count_per_run": metadata.primary_count,
        "structural_validation": "valid",
        "metadata": metadata.raw,
        "numerical_representation": {
            "time": "Exact decimal uint64 ps parsed as Python integers before any comparison.",
            "snapshot": "Owned committed Source snapshot and its Run window, harvested after each successful Run.",
            "provenance": "Per-Run Source records; relative Run-id progression and contiguous global ranges; append order ignored.",
            "particle_and_run_id": "Checked on every raw Observer Source record by the exporter; retained in per-Run metadata.",
            "count_driven_law": "Every birth equals time_start_ps exactly; time_stop_ps bounds chronology, not a random distribution.",
            "source_rng_draws": {"time": 0, "position": 0, "angular": 0, "energy": 0},
            "rng_scope": "Philox configured; no random-state reset or replay assertion.",
        },
        "runs": results,
        "reset_validation": reset_validation,
        "acceptance_thresholds": None,
        "figures": figures,
        "figure_status": figure_status,
    }
    _ = (output / "summary.json").write_text(
        json.dumps(summary, indent=2, allow_nan=False) + "\n", encoding="utf-8"
    )

    print(
        f"{metadata.case.name}: exact chronology and Source birth-time contract verified"
    )
    for run in metadata.runs:
        closing = "]" if metadata.case.chronology == "static" else ")"
        print(
            f"  run {run.sequence_index} (id {run.run_id}): [{run.start_ps},{run.stop_ps}{closing} ps; "
            + f"{metadata.primary_count}/{metadata.primary_count} births exactly {run.start_ps} ps; "
            + f"global IDs [{run.global_begin},{run.global_last}]; overflow=0"
        )
    print(f"  figures: {figure_status}; {output / 'summary.json'}", flush=True)
    return summary


class Arguments(Protocol):
    metadata: Path
    output_dir: Path
    no_plots: bool


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Check exact T1 CountDriven chronology and birth times."
    )
    _ = parser.add_argument("--metadata", type=Path, required=True)
    _ = parser.add_argument("--output-dir", type=Path, required=True)
    _ = parser.add_argument("--no-plots", action="store_true")
    args = cast(Arguments, cast(object, parser.parse_args()))
    _ = analyze_case(args.metadata, args.output_dir, make_figures=not args.no_plots)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
