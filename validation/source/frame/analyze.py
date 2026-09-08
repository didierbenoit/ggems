import argparse
import csv
import json
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

import numpy as np

# Direct script entry points: Python adds this directory to sys.path.
from cases import (  # pyright: ignore[reportImplicitRelativeImport]
    BOUNDS_DEG,
    CASES,
    FOCUS_MM,
    ORIGIN,
    FrameCase,
    orientation_request,
)
from numpy.typing import NDArray

type JsonObject = dict[str, object]
type FloatArray = NDArray[np.float64]
type IntArray = NDArray[np.int64]
type ProvenanceArray = NDArray[np.uint64]
type IntVector = tuple[int, int, int]

UINT64_MAX = (1 << 64) - 1
UINT32_MAX = (1 << 32) - 1
INT64_MIN = -(1 << 63)
INT64_MAX = (1 << 63) - 1
CONVENTION = "axes_as_columns"
IDENTITY_AXES = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
CYCLIC_AXES = ((0, 1, 0), (0, 0, 1), (1, 0, 0))
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
class Metadata:
    case: FrameCase
    count: int
    center_pm: IntVector
    dimensions_pm: IntVector
    focus_pm: IntVector
    display_unit_pm: int
    matrix: FloatArray
    inverse: FloatArray
    frame: JsonObject
    bounds: tuple[float, float, float, float]
    raw: JsonObject


@dataclass(frozen=True, slots=True)
class Samples:
    # Indexed by verified source_local_primary_id, irrespective of CSV order.
    provenance: ProvenanceArray
    positions: IntArray
    directions: FloatArray


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
        raise ValueError(f"{name} is outside its integer range.")
    return value


def _number(value: object, name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"{name} must be numeric.")
    number = float(value)
    if not math.isfinite(number):
        raise ValueError(f"{name} must be finite.")
    return number


def _binary32(value: object, name: str) -> float:
    number = _number(value, name)
    if abs(number) > float(np.finfo(np.float32).max):
        raise ValueError(f"{name} is outside finite binary32.")
    packed = float(np.float32(number))
    # Metadata is written at host max_digits10, unlike the compact CSV.
    if packed != number:
        raise ValueError(f"{name} is not an actual packed binary32 value.")
    return packed


def _triple(value: object, name: str) -> list[object]:
    values = _list(value, name)
    if len(values) != 3:
        raise ValueError(f"{name} must contain exactly three components.")
    return values


def _int_vector(value: object, name: str) -> IntVector:
    values = [
        _integer(item, name, INT64_MIN, INT64_MAX) for item in _triple(value, name)
    ]
    return values[0], values[1], values[2]


def frame_matrix(raw: JsonObject) -> tuple[FloatArray, FloatArray, JsonObject]:
    """Read execution axes; invert the packed matrix, never its ideal rotation."""
    if raw.get("frame_matrix_convention") != CONVENTION:
        raise ValueError("Frame convention must be axes_as_columns.")
    axes = np.array(
        [
            [_binary32(item, "frame component") for item in _triple(axis, "frame axis")]
            for axis in _triple(raw.get("frame_axes"), "frame_axes")
        ],
        dtype=np.float64,
    )
    if np.any(np.abs(axes) > 1.0):
        raise ValueError("A normalized frame component cannot exceed one.")
    matrix = axes.T.copy()
    determinant = float(np.linalg.det(matrix))
    if not math.isfinite(determinant) or determinant <= 0.0:
        raise ValueError("Frame must be nonsingular and right-handed.")
    inverse = np.linalg.inv(matrix)
    if not np.all(np.isfinite(inverse)):
        raise ValueError("Frame inverse is not finite.")
    norms = np.linalg.norm(axes, axis=1)
    return (
        matrix,
        inverse,
        {
            "requested": _object(raw.get("requested_frame"), "requested_frame"),
            "actual_center_pm": list(
                _int_vector(raw.get("source_center_pm"), "center")
            ),
            "actual_frame_axes": [[float(value) for value in axis] for axis in axes],
            "frame_matrix_convention": CONVENTION,
            "local_to_global": "global_delta = M @ local; M = [axis_x axis_y axis_z]",
            "axis_norms": [float(value) for value in norms],
            "pairwise_axis_dots": {
                "xy": float(np.dot(axes[0], axes[1])),
                "xz": float(np.dot(axes[0], axes[2])),
                "yz": float(np.dot(axes[1], axes[2])),
            },
            "determinant": determinant,
            "handedness": "right-handed",
            "condition_number": float(np.linalg.cond(matrix)),
            "orthogonality_acceptance_threshold": None,
        },
    )


def load_metadata(path: Path, case: FrameCase, *, reference: bool = False) -> Metadata:
    raw = _object(cast(object, json.loads(path.read_text(encoding="utf-8"))), str(path))
    expected_name = case.name + ("_reference" if reference else "")
    angular_name = {"fixed": "Fixed", "focused": "Focused"}.get(
        case.angular, "Isotropic"
    )
    for key, expected in {
        "case_name": expected_name,
        "geometry": case.geometry,
        "angular_configuration": case.angular,
        "angular_mode": angular_name,
        "energy_configuration": "mono",
        "energy_mode": "Mono",
        "population_mode": "CountDriven",
        "particle": "Gamma",
        "rng_engine": "Philox",
        "chronology": "static",
    }.items():
        if raw.get(key) != expected:
            raise ValueError(f"Frame case metadata mismatch: {key} must be {expected}.")

    count = _integer(raw.get("primary_count"), "primary_count", 1, UINT32_MAX // 2)
    workers = _integer(raw.get("worker_count"), "worker_count", 1, UINT32_MAX)
    _ = _integer(raw.get("seed"), "seed")
    for key in (
        "source_index",
        "run_id",
        "sequence_index",
        "global_primary_begin",
        "time_ps",
        "effective_start_ps",
        "effective_stop_ps",
        "snapshot_time_start_ps",
        "snapshot_time_stop_ps",
    ):
        if _integer(raw.get(key), key) != 0:
            raise ValueError(
                f"A fresh static single-Run frame case requires {key} == 0."
            )
    if _integer(raw.get("global_primary_last"), "global_primary_last") != count - 1:
        raise ValueError("Global primary range differs from the expected domain.")
    if _integer(raw.get("energy_meV"), "energy_meV") != 511000000:
        raise ValueError("Frame cases require Mono exactly 511000000 meV.")
    if _number(raw.get("weight"), "weight") != 1.0:
        raise ValueError("Frame cases require unit weight.")

    energy = _object(raw.get("energy"), "energy")
    for key, expected in {
        "distribution_type": 1,
        "table_offset": 0,
        "table_count": 0,
        "regular_bin_width_meV": 0,
        "mono_energy_meV": 511000000,
    }.items():
        if _integer(energy.get(key), key) != expected:
            raise ValueError(f"Packed Mono metadata mismatch: {key}.")
    if energy.get("representation") != "uint64 meV":
        raise ValueError("Energy representation must be uint64 meV.")
    for key in (
        "energy_values_meV",
        "relative_weights",
        "cumulative_ticket_upper_bounds",
    ):
        if _list(energy.get(key), key):
            raise ValueError("Mono must have no energy table.")

    names = _list(raw.get("device_names"), "device_names")
    if not names or any(not isinstance(name, str) or not name for name in names):
        raise ValueError("Actual selected device names are required.")
    selector = raw.get("device_selector")
    if not isinstance(selector, str) or not selector:
        raise ValueError("A nonempty device selector is required.")
    if case.paired and (workers != 1 or len(names) != 1):
        raise ValueError("Paired replay requires exactly one worker and one device.")

    observer = _object(raw.get("observer"), "observer")
    for key, expected in {
        "overflow_count": 0,
        "record_count": 2 * count,
        "captured_primary_count": count,
        "source_record_count": count,
    }.items():
        if _integer(observer.get(key), key) != expected:
            raise ValueError(f"Incomplete Observer capture: {key}.")
    for key in ("capacity_per_device", "host_capacity"):
        _ = _integer(observer.get(key), key, 2 * count, UINT32_MAX)

    unit = _integer(raw.get("position_display_unit_pm"), "position_display_unit_pm", 1)
    center = _int_vector(raw.get("source_center_pm"), "source_center_pm")
    dimensions = _int_vector(raw.get("dimensions_pm"), "dimensions_pm")
    focus = _int_vector(raw.get("focus_position_pm"), "focus_position_pm")
    center_mm = ORIGIN if reference else case.center_mm
    if center != tuple(value * unit for value in center_mm):
        raise ValueError("Committed center differs from the frame case fixture.")
    if dimensions != tuple(value * unit for value in case.dimensions_mm):
        raise ValueError("Committed dimensions differ from the frame case fixture.")
    if (
        tuple(
            _number(item, "dimensions_mm")
            for item in _triple(raw.get("dimensions_mm"), "dimensions_mm")
        )
        != case.dimensions_mm
    ):
        raise ValueError("Displayed dimensions disagree with the case.")
    if (
        tuple(
            _number(item, "requested center")
            for item in _triple(raw.get("requested_center_mm"), "requested center")
        )
        != center_mm
    ):
        raise ValueError("Requested center disagrees with the case.")
    expected_focus = FOCUS_MM if case.angular == "focused" else ORIGIN
    if focus != tuple(value * unit for value in expected_focus):
        raise ValueError("Committed global focus differs from the case fixture.")

    frame_name = "identity" if reference else case.frame
    request = orientation_request(frame_name)
    requested = _object(raw.get("requested_frame"), "requested_frame")
    if request is None:
        if requested != {"api": "default_identity"}:
            raise ValueError("Reference requires the default identity API.")
    else:
        if requested.get("api") != "SetOrientation":
            raise ValueError("Frame requires the public SetOrientation API.")
        for key, expected_vector in zip(
            ("direction", "up_reference"), request, strict=True
        ):
            values = tuple(
                _number(item, key) for item in _triple(requested.get(key), key)
            )
            if values != expected_vector:
                raise ValueError(f"Requested {key} differs from the frame case.")
    matrix, inverse, frame = frame_matrix(raw)
    if frame_name in ("identity", "cyclic"):
        expected_axes = IDENTITY_AXES if frame_name == "identity" else CYCLIC_AXES
        if not np.array_equal(matrix.T, np.array(expected_axes, dtype=np.float64)):
            raise ValueError("Exact identity/cyclic axes differ from the contract.")
    stored_fixed = np.array(
        [
            _binary32(item, "fixed_direction")
            for item in _triple(raw.get("fixed_direction"), "fixed_direction")
        ],
        dtype=np.float64,
    )
    if not np.array_equal(stored_fixed, matrix[:, 2]):
        raise ValueError("Stored Fixed direction must be actual axis_z.")

    lower, upper, phi_min, phi_max = (
        _binary32(raw.get(key), key)
        for key in (
            "isotropic_cos_theta_lower",
            "isotropic_cos_theta_upper",
            "isotropic_phi_min_rad",
            "isotropic_phi_max_rad",
        )
    )
    if not (-1.0 <= lower < upper <= 1.0 and phi_min < phi_max):
        raise ValueError("Invalid packed angular bounds.")
    if case.angular == "bounded-isotropic":
        requested_bounds = _object(
            raw.get("requested_bounded_degrees"), "bounds request"
        )
        for key, expected in zip(
            ("theta_min", "theta_max", "phi_min", "phi_max"), BOUNDS_DEG, strict=True
        ):
            if _number(requested_bounds.get(key), key) != expected:
                raise ValueError("Requested bounded angular law differs from the case.")
        if not (
            0.0 < lower < upper < 1.0 and -math.pi < phi_min < 0.0 < phi_max < math.pi
        ):
            raise ValueError(
                "Packed bounds must retain this case's nonwrapping sector."
            )
    else:
        if raw.get("requested_bounded_degrees") is not None or (
            lower,
            upper,
            phi_min,
            phi_max,
        ) != (-1.0, 1.0, 0.0, float(np.float32(math.tau))):
            raise ValueError("Expected the canonical no-bounds full-sphere descriptor.")
    return Metadata(
        case,
        count,
        center,
        dimensions,
        focus,
        unit,
        matrix,
        inverse,
        frame,
        (lower, upper, phi_min, phi_max),
        raw,
    )


def _decimal(value: str, minimum: int, maximum: int) -> int:
    digits = value.removeprefix("-")
    if not digits or not digits.isascii() or not digits.isdecimal():
        raise ValueError(f"Expected an exact decimal integer, received {value!r}.")
    return _integer(int(value), "CSV integer", minimum, maximum)


def load_samples(path: Path, metadata: Metadata) -> Samples:
    positions = np.empty((metadata.count, 3), dtype=np.int64)
    directions = np.empty((metadata.count, 3), dtype=np.float64)
    provenance = np.empty((metadata.count, 3), dtype=np.uint64)
    seen: set[int] = set()
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.reader(stream, strict=True)
        if tuple(next(reader, ())) != CSV_COLUMNS:
            raise ValueError("Malformed Source CSV header.")
        for row_number, row in enumerate(reader, start=2):
            if len(row) != len(CSV_COLUMNS) or row[12] != "Source":
                raise ValueError(
                    f"Malformed or non-Source record at CSV row {row_number}."
                )
            slot = _decimal(row[0], 0, UINT32_MAX)
            local = _decimal(row[1], 0, UINT64_MAX)
            global_id = _decimal(row[2], 0, UINT64_MAX)
            if slot != 0 or local >= metadata.count or global_id != local:
                raise ValueError(
                    "Impossible Source provenance or incompatible global ID."
                )
            if local in seen:
                raise ValueError("Duplicate Source provenance.")
            seen.add(local)
            provenance[local] = (slot, local, global_id)
            positions[local] = [
                _decimal(value, INT64_MIN, INT64_MAX) for value in row[3:6]
            ]
            for axis, value in enumerate(row[6:9]):
                number = _number(float(value), "CSV direction")
                if abs(number) > float(np.finfo(np.float32).max):
                    raise ValueError("CSV direction exceeds binary32.")
                directions[local, axis] = float(np.float32(number))
            if (
                _decimal(row[9], 0, UINT64_MAX) != 511000000
                or _decimal(row[10], 0, UINT64_MAX) != 0
            ):
                raise ValueError(
                    "Frame cases require exact Mono 511 keV and static 0 ps."
                )
            if _number(float(row[11]), "weight") != 1.0:
                raise ValueError("Frame cases require exact unit weight.")
    if len(seen) != metadata.count:
        raise ValueError(
            "Missing Source records or incomplete local primary ID domain."
        )
    if np.any(np.linalg.norm(directions, axis=1) == 0.0):
        raise ValueError("Zero Source direction.")
    return Samples(provenance, positions, directions)


def integer_delta(positions: IntArray, center: IntVector) -> FloatArray:
    # Python integers avoid int64 subtraction overflow. Check the exact-integer
    # binary64 domain before converting; this campaign is deliberately modest.
    rows = [[int(row[axis]) - center[axis] for axis in range(3)] for row in positions]
    if any(abs(value) > 1 << 53 for row in rows for value in row):
        raise ValueError(
            "Position displacement exceeds exact binary64 integer analysis domain."
        )
    return np.array(rows, dtype=np.float64)


def recover_local(positions: IntArray, metadata: Metadata) -> FloatArray:
    delta = integer_delta(positions, metadata.center_pm)
    return (metadata.inverse @ delta.T).T


def scalar_statistics(values: FloatArray) -> JsonObject:
    if not values.size:
        return {
            "sample_count": 0,
            "minimum": None,
            "maximum": None,
            "mean": None,
            "variance": None,
        }
    return {
        "sample_count": int(values.size),
        "minimum": float(np.min(values)),
        "maximum": float(np.max(values)),
        "mean": float(np.mean(values)),
        "variance": float(np.var(values, ddof=0)),
    }


def uniform_statistics(values: FloatArray) -> JsonObject:
    result = scalar_statistics(values)
    result.update(
        {"ideal_mean": 0.5, "ideal_variance": 1.0 / 12.0, "ecdf_max_deviation": None}
    )
    if not values.size:
        return result
    ordered = np.sort(values)
    # Evaluate the analytical CDF at unmodified observations, including tails.
    cdf = np.where(ordered < 0.0, 0.0, np.where(ordered > 1.0, 1.0, ordered))
    after = np.arange(1, values.size + 1, dtype=np.float64) / values.size
    before = after - 1.0 / values.size
    result["ecdf_max_deviation"] = float(
        max(np.max(np.abs(after - cdf)), np.max(np.abs(before - cdf)))
    )
    return result


def correlations(variables: dict[str, FloatArray]) -> JsonObject:
    names = list(variables)
    matrix: list[list[float | None]] = []
    for left in variables.values():
        row: list[float | None] = []
        for right in variables.values():
            if left.size != right.size:
                raise ValueError(
                    "Correlation variables must refer to the same primaries."
                )
            value = None
            if left.size >= 2 and np.var(left) > 0.0 and np.var(right) > 0.0:
                value = float(np.corrcoef(left, right)[0, 1])
            row.append(value)
        matrix.append(row)
    return {
        "variables": names,
        "sample_count": int(next(iter(variables.values())).size),
        "pearson_matrix": matrix,
    }


def uniform_support(values: FloatArray) -> JsonObject:
    return {
        "below_zero_count": int(np.count_nonzero(values < 0.0)),
        "above_one_count": int(np.count_nonzero(values > 1.0)),
        "maximum_normalized_excess": max(
            0.0, -float(np.min(values)), float(np.max(values)) - 1.0
        )
        if values.size
        else None,
    }


def norm_diagnostics(directions: FloatArray) -> JsonObject:
    norms = np.linalg.norm(directions, axis=1)
    result = scalar_statistics(norms)
    result["maximum_absolute_norm_minus_one"] = float(np.max(np.abs(norms - 1.0)))
    return result


def geometry_measurements(
    local: FloatArray, metadata: Metadata
) -> tuple[JsonObject, dict[str, FloatArray]]:
    width_count = 3 if metadata.case.geometry == "box" else 2
    variables = {
        "u_" + "xyz"[axis]: local[:, axis] / metadata.dimensions_pm[axis] + 0.5
        for axis in range(width_count)
    }
    outside = np.zeros(local.shape[0], dtype=np.bool_)
    supports: JsonObject = {}
    for axis, (name, values) in enumerate(variables.items()):
        outside |= (values < 0.0) | (values > 1.0)
        support = uniform_support(values)
        support["maximum_absolute_excess_pm"] = max(
            0.0,
            float(np.max(np.abs(local[:, axis]))) - metadata.dimensions_pm[axis] / 2.0,
        )
        supports[name] = support
    result: JsonObject = {
        "transformed_variables": {
            name: uniform_statistics(values) for name, values in variables.items()
        },
        "correlations": correlations(variables),
        "support": supports,
    }
    if width_count == 2:
        z = local[:, 2]
        absolute = np.abs(z)
        result["local_z_residual_pm"] = {
            **scalar_statistics(z),
            "nonzero_count": int(np.count_nonzero(z)),
            "mean_absolute": float(np.mean(absolute)),
            "maximum_absolute": float(np.max(absolute)),
        }
        # Report ideal zero-thickness support separately from lateral support.
        result["outside_lateral_support_count"] = int(np.count_nonzero(outside))
        outside |= z != 0.0
    result["outside_ideal_support_count"] = int(np.count_nonzero(outside))
    return result, variables


def angular_measurements(
    directions: FloatArray, metadata: Metadata, *, full_sphere: bool = False
) -> tuple[JsonObject, dict[str, FloatArray]]:
    if full_sphere:
        # Same global A1 reference: canonical full-sphere sampling bypasses M.
        local = directions
        scales = np.linalg.norm(local, axis=1)
    else:
        raw = (metadata.inverse @ directions.T).T
        scales = np.linalg.norm(raw, axis=1)
        if np.any(scales == 0.0) or not np.all(np.isfinite(scales)):
            raise ValueError("Invalid recovered local direction scale.")
        local = raw / scales[:, None]

    valid_phi = (local[:, 0] != 0.0) | (local[:, 1] != 0.0)
    phi = np.arctan2(local[valid_phi, 1], local[valid_phi, 0])
    lower, upper, phi_min, phi_max = metadata.bounds
    if full_sphere:
        u_cos = (local[:, 2] + 1.0) / 2.0
        u_phi = np.mod(phi, math.tau) / math.tau
    else:
        # This case's actual packed sector lies strictly within [-pi, pi].
        u_cos = (local[:, 2] - lower) / (upper - lower)
        u_phi = (phi - phi_min) / (phi_max - phi_min)
    variables = {"u_cos": u_cos, "u_phi": u_phi, "u_cos_for_phi": u_cos[valid_phi]}
    result: JsonObject = {
        "direction_norm": norm_diagnostics(directions),
        "recovered_local_scale": scalar_statistics(scales),
        "transformed_variables": {
            "u_cos": uniform_statistics(u_cos),
            "u_phi": uniform_statistics(u_phi),
        },
        "correlations": correlations({"u_cos": u_cos[valid_phi], "u_phi": u_phi}),
        "support": {"u_cos": uniform_support(u_cos), "u_phi": uniform_support(u_phi)},
        "undefined_phi_exclusions": int(np.count_nonzero(~valid_phi)),
        "actual_packed_bounds": list(metadata.bounds),
    }
    if full_sphere:
        result["cartesian_mean"] = [
            float(np.mean(directions[:, axis])) for axis in range(3)
        ]
        result["cartesian_second_moments"] = [
            float(np.mean(directions[:, axis] ** 2)) for axis in range(3)
        ]
        result["cartesian_cross_moments"] = {
            name: float(np.mean(directions[:, left] * directions[:, right]))
            for name, left, right in (("xy", 0, 1), ("xz", 0, 2), ("yz", 1, 2))
        }
        result["ideal_cartesian_moments"] = {
            "mean": 0.0,
            "second": 1.0 / 3.0,
            "cross": 0.0,
        }
    return result, variables


def compare_positions(
    actual: IntArray, reference: IntArray, center: IntVector
) -> JsonObject:
    if actual.shape != reference.shape:
        raise ValueError("Exact position pair has incompatible shapes.")
    mismatch = 0
    maximum = 0
    for observed, local in zip(actual, reference, strict=True):
        expected = (
            center[0] + int(local[2]),
            center[1] + int(local[0]),
            center[2] + int(local[1]),
        )
        differences = [abs(int(observed[axis]) - expected[axis]) for axis in range(3)]
        mismatch += any(differences)
        maximum = max(maximum, *differences)
    metrics: JsonObject = {
        "mismatching_position_count": mismatch,
        "maximum_component_difference_pm": maximum,
    }
    if mismatch:
        raise ValueError(f"Exact cyclic position contract violated: {metrics}")
    return metrics


def compare_directions(actual: FloatArray, expected: FloatArray) -> JsonObject:
    if actual.shape != expected.shape:
        raise ValueError("Exact direction comparison has incompatible shapes.")
    delta = actual - expected
    mismatch = int(np.count_nonzero(np.any(delta != 0.0, axis=1)))
    metrics: JsonObject = {
        "mismatching_direction_count": mismatch,
        "maximum_component_difference": float(np.max(np.abs(delta))),
        "maximum_vector_difference": float(np.max(np.linalg.norm(delta, axis=1))),
    }
    if mismatch:
        raise ValueError(
            f"Exact direction contract violated (no tolerance applied): {metrics}"
        )
    return metrics


def validate_pair(
    left: Samples, right: Samples, reference: Metadata, transformed: Metadata
) -> JsonObject:
    if not np.array_equal(left.provenance, right.provenance):
        raise ValueError("Paired Source provenance domains differ.")
    for key in (
        "primary_count",
        "worker_count",
        "seed",
        "rng_engine",
        "device_selector",
        "device_names",
        "source_index",
        "global_primary_begin",
        "global_primary_last",
        "run_id",
        "geometry",
        "dimensions_pm",
        "angular_configuration",
        "isotropic_cos_theta_lower",
        "isotropic_cos_theta_upper",
        "isotropic_phi_min_rad",
        "isotropic_phi_max_rad",
        "energy",
        "chronology",
        "time_ps",
        "weight",
        "particle",
    ):
        if reference.raw.get(key) != transformed.raw.get(key):
            raise ValueError(f"Paired execution configurations differ: {key}.")
    if (
        reference.raw.get("worker_count") != 1
        or len(_list(reference.raw.get("device_names"), "devices")) != 1
    ):
        raise ValueError(
            "Per-primary paired replay requires one worker and one device."
        )
    return {
        "reference_case": reference.raw["case_name"],
        "transformed_case": transformed.raw["case_name"],
        "paired_sample_count": transformed.count,
        "provenance_compatibility": "identical complete source-local/global ID domains",
        "pairing_key": ["source_index", "source_local_primary_id"],
        "worker_count": 1,
        "rng_replay_scope": "Fresh processes; one device, stream 0, one worker, identical draw budget.",
    }


def focused_measurements(
    positions: IntArray, directions: FloatArray, focus: IntVector
) -> tuple[JsonObject, dict[str, FloatArray]]:
    # exact position - focus, then sign reversal: no float subtraction of centers.
    displacement = -integer_delta(positions, focus)
    distances = np.linalg.norm(displacement, axis=1)
    norms = np.linalg.norm(directions, axis=1)
    if np.any(distances == 0.0) or np.any(norms == 0.0):
        raise ValueError("Focused reference or emitted direction is undefined.")
    reference = displacement / distances[:, None]
    observed = directions / norms[:, None]
    dot = np.sum(reference * observed, axis=1)
    nonforward = int(np.count_nonzero(dot <= 0.0))
    if nonforward:
        raise ValueError(f"Nonforward Focused directions: {nonforward}.")
    cross = np.linalg.norm(np.cross(reference, observed), axis=1)
    angular_error = np.arctan2(cross, dot)
    miss = np.linalg.norm(np.cross(displacement, observed), axis=1)
    return {
        "actual_global_focus_pm": list(focus),
        "reference": "Exact integer focus minus actual committed global Source position, then binary64 normalization.",
        "direction_norm": norm_diagnostics(directions),
        "minimum_unit_dot_reference": float(np.min(dot)),
        "mean_angular_error_rad": float(np.mean(angular_error)),
        "maximum_angular_error_rad": float(np.max(angular_error)),
        "mean_miss_distance_pm": float(np.mean(miss)),
        "median_miss_distance_pm": float(np.median(miss)),
        "p95_miss_distance_pm": float(np.percentile(miss, 95)),
        "maximum_miss_distance_pm": float(np.max(miss)),
        "nonforward_direction_count": nonforward,
    }, {"angular_error_rad": angular_error, "miss_distance_pm": miss}


def measure_case(
    metadata: Metadata,
    samples: Samples,
    reference_metadata: Metadata | None = None,
    reference: Samples | None = None,
) -> tuple[JsonObject, FloatArray, dict[str, FloatArray]]:
    case = metadata.case
    summary: JsonObject = {
        "case_name": case.name,
        "sample_count": metadata.count,
        "structural_validation": {
            "status": "valid",
            "observer_overflow_count": 0,
            "source_records_only": True,
            "complete_unique_provenance": True,
            "mono_energy_meV": 511000000,
            "static_time_ps": 0,
            "weight": 1,
            "particle": "Gamma; checked on every raw record by the exporter",
        },
        "frame": metadata.frame,
        "metadata": metadata.raw,
        "acceptance_thresholds": None,
        "numerical_representation": {
            "positions": "Exact Observer int64 pm; Python integer subtraction before binary64 conversion.",
            "directions": "CSV decimal components reconstructed as binary32, then promoted to binary64.",
            "frame": "Actual immutable snapshot binary32 axes, inverted in binary64; not transposed for inverse analysis.",
            "geometry_residuals": "Binary32 local sampling/transform plus half-away-from-zero pm commitment; no clipping or repair.",
        },
        "direction_norm": norm_diagnostics(samples.directions),
        "repeated_position_count": metadata.count
        - int(np.unique(samples.positions, axis=0).shape[0]),
    }
    local = recover_local(samples.positions, metadata)
    variables: dict[str, FloatArray] = {}
    if case.geometry == "point":
        delta = integer_delta(samples.positions, metadata.center_pm)
        mismatches = int(np.count_nonzero(np.any(delta != 0.0, axis=1)))
        metrics = {
            "exact_center_pm": list(metadata.center_pm),
            "mismatching_position_count": mismatches,
            "maximum_component_difference_pm": int(np.max(np.abs(delta))),
        }
        if mismatches:
            raise ValueError(
                f"Point differs from its exact committed center: {metrics}"
            )
        summary["exact_point"] = metrics
    else:
        summary["geometry"], variables = geometry_measurements(local, metadata)

    if case.angular == "fixed":
        expected = np.broadcast_to(metadata.matrix[:, 2], samples.directions.shape)
        summary["exact_fixed"] = {
            **compare_directions(samples.directions, expected),
            "packed_expected_direction": [
                float(value) for value in metadata.matrix[:, 2]
            ],
        }

    if case.paired:
        if reference is None or reference_metadata is None:
            raise ValueError("This case requires reference samples and metadata.")
        summary["pair"] = validate_pair(
            reference, samples, reference_metadata, metadata
        )
        summary["reference_frame"] = reference_metadata.frame
        summary["reference_metadata"] = reference_metadata.raw
        if case.geometry == "box":
            summary["exact_pair"] = compare_positions(
                samples.positions, reference.positions, metadata.center_pm
            )
            _ = compare_directions(
                reference.directions,
                np.broadcast_to(
                    reference_metadata.matrix[:, 2], reference.directions.shape
                ),
            )
        else:
            if np.any(
                integer_delta(reference.positions, reference_metadata.center_pm) != 0.0
            ):
                raise ValueError(
                    "Reference Point positions differ from the committed center."
                )
            expected = reference.directions
            if case.angular == "bounded-isotropic":
                expected = expected[:, [2, 0, 1]]
            summary["exact_pair"] = compare_directions(samples.directions, expected)
    elif reference is not None or reference_metadata is not None:
        raise ValueError("Unexpected reference input for an unpaired case.")

    if case.angular in ("isotropic", "bounded-isotropic"):
        angular_samples, angular_metadata = samples, metadata
        if (
            case.angular == "isotropic"
            and reference is not None
            and reference_metadata is not None
        ):
            angular_samples, angular_metadata = reference, reference_metadata
        summary["angle"], variables = angular_measurements(
            angular_samples.directions,
            angular_metadata,
            full_sphere=case.angular == "isotropic",
        )
    elif case.angular == "focused":
        summary["focused"], focus_variables = focused_measurements(
            samples.positions, samples.directions, metadata.focus_pm
        )
        variables.update(focus_variables)
    return summary, local, variables


def analyze_case(
    samples_path: Path,
    metadata_path: Path,
    output: Path,
    *,
    reference_samples: Path | None = None,
    reference_metadata: Path | None = None,
    make_figures: bool = True,
) -> JsonObject:
    if (output / "summary.json").exists():
        raise FileExistsError(
            "Use a new analysis directory; summary.json already exists."
        )
    raw = _object(
        cast(object, json.loads(metadata_path.read_text(encoding="utf-8"))), "metadata"
    )
    case = next((case for case in CASES if case.name == raw.get("case_name")), None)
    if case is None:
        raise ValueError("Unknown G2/A2 frame case.")
    if (reference_samples is None) != (reference_metadata is None):
        raise ValueError("Supply both reference samples and reference metadata.")
    metadata = load_metadata(metadata_path, case)
    samples = load_samples(samples_path, metadata)
    ref_meta = (
        load_metadata(reference_metadata, case, reference=True)
        if reference_metadata
        else None
    )
    ref = (
        load_samples(reference_samples, ref_meta)
        if reference_samples and ref_meta
        else None
    )
    summary, local, variables = measure_case(metadata, samples, ref_meta, ref)

    output.mkdir(parents=True, exist_ok=True)
    summary["figures"] = []
    summary["figure_status"] = "not_required"
    if case.statistical:
        summary["figure_status"] = "disabled_by_request"
        if make_figures:
            from plot import plot_frame  # pyright: ignore[reportImplicitRelativeImport]

            summary["figures"] = plot_frame(
                case,
                samples.positions,
                local,
                variables,
                metadata.display_unit_pm,
                output,
            )
            summary["figure_status"] = "generated"
    _ = (output / "summary.json").write_text(
        json.dumps(summary, indent=2, allow_nan=False) + "\n", encoding="utf-8"
    )
    print(f"{case.name}: N={metadata.count}; complete Source capture; overflow=0")
    for key in ("exact_point", "exact_fixed", "exact_pair", "focused"):
        if key in summary:
            print(f"  {key}: {summary[key]}")
    for key in ("geometry", "angle"):
        if key in summary:
            metrics = _object(summary[key], key)
            print(f"  {key}: {metrics['transformed_variables']}")
    print(
        f"  figures: {summary['figure_status']}; {output / 'summary.json'}", flush=True
    )
    return summary


class Arguments(Protocol):
    samples: Path
    metadata: Path
    reference_samples: Path | None
    reference_metadata: Path | None
    output_dir: Path
    no_plots: bool


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Analyze production Source G2/A2 frame captures."
    )
    _ = parser.add_argument("--samples", type=Path, required=True)
    _ = parser.add_argument("--metadata", type=Path, required=True)
    _ = parser.add_argument("--reference-samples", type=Path)
    _ = parser.add_argument("--reference-metadata", type=Path)
    _ = parser.add_argument("--output-dir", type=Path, required=True)
    _ = parser.add_argument("--no-plots", action="store_true")
    args = cast(Arguments, cast(object, parser.parse_args()))
    _ = analyze_case(
        args.samples,
        args.metadata,
        args.output_dir,
        reference_samples=args.reference_samples,
        reference_metadata=args.reference_metadata,
        make_figures=not args.no_plots,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
