import argparse
import csv
import json
import math
from collections import Counter
from dataclasses import dataclass
from fractions import Fraction
from pathlib import Path
from typing import Protocol, cast

import numpy as np

# Direct script entry points: Python adds this directory to sys.path.
from cases import (  # pyright: ignore[reportImplicitRelativeImport]
    CASES,
    CENTER_MM,
    FRAME_DIRECTION,
    FRAME_UP,
    LINE_ENERGIES_KEV,
    LINE_WEIGHTS,
    PAIR_WORKERS,
    REGULAR_CENTERS_KEV,
    REGULAR_WEIGHTS,
    REGULAR_WIDTH_KEV,
    Configuration,
    IntegrationCase,
    draw_budget,
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
TICKET_SPACE = 1 << 32
CONVENTION = "axes_as_columns"
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
    "energy_micro_eV",
    "time_ps",
    "weight",
    "record_kind",
)


@dataclass(frozen=True, slots=True)
class Metadata:
    name: str
    configuration: Configuration
    count: int
    center_pm: IntVector
    dimensions_pm: IntVector
    display_unit_pm: int
    matrix: FloatArray
    inverse: FloatArray
    frame: JsonObject
    bounds: tuple[float, float, float, float]
    energies_micro_ev: tuple[int, ...]
    ticket_counts: tuple[int, ...]
    bin_width_micro_ev: int
    display_unit_micro_ev: int
    raw: JsonObject


@dataclass(frozen=True, slots=True)
class Samples:
    # Every array is indexed by validated source_local_primary_id, not CSV order.
    provenance: ProvenanceArray
    positions: IntArray
    directions: FloatArray
    energies: tuple[int, ...]
    times: tuple[int, ...]


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


def _decimal(value: str, minimum: int, maximum: int) -> int:
    digits = value.removeprefix("-")
    if not digits or not digits.isascii() or not digits.isdecimal():
        raise ValueError(f"Expected an exact decimal integer, received {value!r}.")
    return _integer(int(value), "CSV integer", minimum, maximum)


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


def finite_cdf_ticket_count(offset: int, ticket_count: int, width: int) -> int:
    """Count tickets with floor(W*t/M) <= offset, without enumerating tickets."""
    if ticket_count <= 0 or width <= 0:
        raise ValueError("The finite law requires positive M and W.")
    if offset < 0:
        return 0
    if offset >= width - 1:
        return ticket_count

    # floor(W*t/M) <= k iff t < M*(k+1)/W. There are ceil(M*(k+1)/W)
    # nonnegative integer tickets satisfying this strict inequality.
    return (ticket_count * (offset + 1) + width - 1) // width


def finite_cdf_max_deviation(
    offsets: list[int], ticket_count: int, width: int
) -> float | None:
    """Supremum of |empirical CDF - exact discrete CDF|, including both jumps."""
    if ticket_count <= 0 or width <= 0:
        raise ValueError("The finite law requires positive M and W.")
    if not offsets:
        return None

    count = len(offsets)
    before = 0
    maximum_numerator = 0
    for offset, multiplicity in sorted(Counter(offsets).items()):
        if not 0 <= offset < width:
            raise ValueError("Offset is outside its half-open bin.")
        exact_before = finite_cdf_ticket_count(offset - 1, ticket_count, width)
        exact_after = finite_cdf_ticket_count(offset, ticket_count, width)
        if exact_before == exact_after:
            raise ValueError(
                "Emitted offset has zero probability in the finite ticket law."
            )

        after = before + multiplicity
        # Group ties: an empirical jump has only a left and a right limit.
        # Between observed offsets the empirical CDF is constant and the
        # reference is monotone, so these endpoints also cover unobserved jumps.
        maximum_numerator = max(
            maximum_numerator,
            abs(before * ticket_count - exact_before * count),
            abs(after * ticket_count - exact_after * count),
        )
        before = after

    return float(Fraction(maximum_numerator, count * ticket_count))


def probability_results(
    counts: list[int], tickets: tuple[int, ...], total: int
) -> tuple[list[JsonObject], JsonObject]:
    rows: list[JsonObject] = []
    residuals: list[Fraction] = []
    cumulative = Fraction(0)
    maximum_cumulative = Fraction(0)

    for observed, ticket_count in zip(counts, tickets, strict=True):
        expected = Fraction(ticket_count, TICKET_SPACE)
        residual = Fraction(observed, total) - expected
        residuals.append(residual)
        cumulative += residual
        maximum_cumulative = max(maximum_cumulative, abs(cumulative))
        rows.append(
            {
                "ticket_count": ticket_count,
                "expected_probability": float(expected),
                "expected_probability_fraction": f"{expected.numerator}/{expected.denominator}",
                "observed_count": observed,
                "observed_probability": observed / total,
                "probability_residual": float(residual),
                "absolute_probability_error": float(abs(residual)),
            }
        )

    return rows, {
        "maximum_absolute_probability_error": float(max(map(abs, residuals))),
        "total_variation_distance": float(sum(map(abs, residuals), Fraction(0)) / 2),
        "cumulative_categorical_max_deviation": float(maximum_cumulative),
    }


def load_metadata(
    path: Path, configuration: Configuration, name: str, *, paired: bool
) -> Metadata:
    raw = _object(cast(object, json.loads(path.read_text(encoding="utf-8"))), str(path))
    for key, expected in {
        "case_name": name,
        "geometry": configuration.geometry,
        "angular_configuration": configuration.angular,
        "angular_mode": "Fixed" if configuration.angular == "fixed" else "Isotropic",
        "energy_configuration": configuration.energy,
        "energy_mode": "Regular spectrum"
        if configuration.energy == "regular-spectrum"
        else "Discrete lines",
        "population_mode": "CountDriven",
        "particle": "Gamma",
        "rng_engine": "Philox",
        "chronology": "static",
    }.items():
        if raw.get(key) != expected:
            raise ValueError(f"I1 metadata mismatch: {key} must be {expected}.")

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
        "energy_micro_eV",
    ):
        if _integer(raw.get(key), key) != 0:
            raise ValueError(f"A fresh static non-Mono I1 run requires {key} == 0.")
    if _integer(raw.get("global_primary_last"), "global_primary_last") != count - 1:
        raise ValueError("Global primary range differs from the expected domain.")
    if _number(raw.get("weight"), "weight") != 1.0:
        raise ValueError("I1 requires unit weight.")

    names = _list(raw.get("device_names"), "device_names")
    if not names or any(not isinstance(item, str) or not item for item in names):
        raise ValueError("Actual selected device names are required.")
    selector = raw.get("device_selector")
    if not isinstance(selector, str) or not selector:
        raise ValueError("A nonempty device selector is required.")
    if paired and (workers != PAIR_WORKERS or len(names) != 1):
        raise ValueError("I1 exact pairs require exactly one worker and one device.")

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

    # Physical fixtures use scales exported by central GGEMS Units.
    unit = _integer(raw.get("position_display_unit_pm"), "position_display_unit_pm", 1)
    center = _int_vector(raw.get("source_center_pm"), "source_center_pm")
    dimensions = _int_vector(raw.get("dimensions_pm"), "dimensions_pm")
    if center != tuple(value * unit for value in CENTER_MM):
        raise ValueError("Committed center differs from the I1 fixture.")
    if dimensions != tuple(value * unit for value in configuration.dimensions_mm):
        raise ValueError("Committed dimensions differ from the I1 fixture.")
    for key, expected_vector in (
        ("requested_center_mm", CENTER_MM),
        ("dimensions_mm", configuration.dimensions_mm),
    ):
        if (
            tuple(_number(item, key) for item in _triple(raw.get(key), key))
            != expected_vector
        ):
            raise ValueError(f"I1 metadata mismatch: {key}.")
    if _int_vector(raw.get("focus_position_pm"), "focus_position_pm") != (0, 0, 0):
        raise ValueError("I1 has no Focused configuration.")

    requested = _object(raw.get("requested_frame"), "requested_frame")
    if requested.get("api") != "SetOrientation":
        raise ValueError("I1 requires the public SetOrientation request.")
    for key, expected_vector in (
        ("direction", FRAME_DIRECTION),
        ("up_reference", FRAME_UP),
    ):
        values = tuple(_number(item, key) for item in _triple(requested.get(key), key))
        if values != expected_vector:
            raise ValueError(f"Requested frame {key} differs from the I1 fixture.")
    matrix, inverse, frame = frame_matrix(raw)
    fixed = [
        _binary32(item, "fixed_direction")
        for item in _triple(raw.get("fixed_direction"), "fixed_direction")
    ]
    if not np.array_equal(fixed, matrix[:, 2]):
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
    if configuration.bounds_deg is not None:
        requested_bounds = _object(
            raw.get("requested_bounded_degrees"), "requested bounds"
        )
        for key, expected in zip(
            ("theta_min", "theta_max", "phi_min", "phi_max"),
            configuration.bounds_deg,
            strict=True,
        ):
            if _number(requested_bounds.get(key), key) != expected:
                raise ValueError("Requested bounded law differs from the I1 fixture.")
        # Both selected sectors fit atan2's nonwrapping interval. Packed values
        # supply the law; requested degrees are never used to rebuild its limits.
        if not (0.0 < lower < upper < 1.0 and -math.pi < phi_min < phi_max < math.pi):
            raise ValueError("Invalid packed bounded sector.")
    elif raw.get("requested_bounded_degrees") is not None or (
        lower,
        upper,
        phi_min,
        phi_max,
    ) != (-1.0, 1.0, 0.0, float(np.float32(math.tau))):
        raise ValueError("Fixed must retain its inactive default angular descriptor.")

    energy = _object(raw.get("energy"), "energy")
    regular = configuration.energy == "regular-spectrum"
    energy_unit = _integer(
        energy.get("display_unit_micro_eV"), "display_unit_micro_eV", 1
    )
    width = _integer(
        energy.get("regular_bin_width_micro_eV"), "regular_bin_width_micro_eV"
    )
    energies = tuple(
        _integer(value, "energy value", 1)
        for value in _list(energy.get("energy_values_micro_eV"), "energy values")
    )
    bounds = tuple(
        _integer(value, "ticket bound", 0, TICKET_SPACE)
        for value in _list(
            energy.get("cumulative_ticket_upper_bounds"), "ticket bounds"
        )
    )
    expected_values = REGULAR_CENTERS_KEV if regular else LINE_ENERGIES_KEV
    weights = REGULAR_WEIGHTS if regular else LINE_WEIGHTS
    for key, expected in {
        "distribution_type": 3 if regular else 2,
        "table_count": len(expected_values),
        "table_offset": 0,
        "mono_energy_micro_eV": 0,
        "ticket_space_size": TICKET_SPACE,
    }.items():
        if _integer(energy.get(key), key) != expected:
            raise ValueError(f"Invalid packed energy descriptor: {key}.")
    if (
        energy.get("representation") != "uint64 micro-eV"
        or energy.get("display_unit") != "keV"
    ):
        raise ValueError("I1 requires the current uint64 micro-eV representation.")
    if energies != tuple(value * energy_unit for value in expected_values):
        raise ValueError("Packed energies differ from the E1 fixture.")
    if width != (REGULAR_WIDTH_KEV * energy_unit if regular else 0):
        raise ValueError("Packed bin width differs from the E1 fixture.")
    if regular and (width <= 0 or width % 2 or energies[0] <= width // 2):
        raise ValueError("RegularSpectrum needs an even width and positive first edge.")
    actual_weights = tuple(
        _number(value, "relative weight")
        for value in _list(energy.get("relative_weights"), "relative weights")
    )
    if actual_weights != weights or len(bounds) != len(energies):
        raise ValueError("Malformed energy weights/ticket table.")
    tickets = tuple(
        upper - lower for lower, upper in zip((0,) + bounds[:-1], bounds, strict=True)
    )
    expected_probabilities = (
        (Fraction(1, 8), Fraction(1, 8), Fraction(1, 4), Fraction(1, 2))
        if regular
        else (Fraction(1, 4), Fraction(0), Fraction(1, 4), Fraction(1, 2))
    )
    if (
        bounds[-1] != TICKET_SPACE
        or any(ticket < 0 for ticket in tickets)
        or tuple(Fraction(ticket, TICKET_SPACE) for ticket in tickets)
        != expected_probabilities
    ):
        raise ValueError("Packed ticket masses differ from the exact E1 probabilities.")

    return Metadata(
        name,
        configuration,
        count,
        center,
        dimensions,
        unit,
        matrix,
        inverse,
        frame,
        (lower, upper, phi_min, phi_max),
        energies,
        tickets,
        width,
        energy_unit,
        raw,
    )


def load_samples(path: Path, metadata: Metadata) -> Samples:
    positions = np.empty((metadata.count, 3), dtype=np.int64)
    directions = np.empty((metadata.count, 3), dtype=np.float64)
    provenance = np.empty((metadata.count, 3), dtype=np.uint64)
    energies = [0] * metadata.count
    times = [0] * metadata.count
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
            energies[local] = _decimal(row[9], 1, UINT64_MAX)
            times[local] = _decimal(row[10], 0, UINT64_MAX)
            if times[local] != 0:
                raise ValueError("I1 requires exact static 0 ps.")
            if _number(float(row[11]), "weight") != 1.0:
                raise ValueError("I1 requires exact unit weight.")
    if len(seen) != metadata.count:
        raise ValueError(
            "Missing Source records or incomplete local primary ID domain."
        )
    if np.any(np.linalg.norm(directions, axis=1) == 0.0):
        raise ValueError("Zero Source direction.")
    if metadata.configuration.geometry == "point" and np.any(
        positions != metadata.center_pm
    ):
        raise ValueError("Point position differs from the committed center.")
    if metadata.configuration.angular == "fixed" and np.any(
        directions != metadata.matrix[:, 2]
    ):
        raise ValueError("Fixed direction differs from packed axis_z.")
    return Samples(provenance, positions, directions, tuple(energies), tuple(times))


def position_measurements(
    local: FloatArray, metadata: Metadata
) -> tuple[JsonObject, dict[str, FloatArray]]:
    variables = {
        "u_x": local[:, 0] / metadata.dimensions_pm[0] + 0.5,
        "u_y": local[:, 1] / metadata.dimensions_pm[1] + 0.5,
    }
    outside = np.zeros(metadata.count, dtype=np.bool_)
    support: JsonObject = {}
    for axis, (name, values) in enumerate(variables.items()):
        outside |= (values < 0.0) | (values > 1.0)
        item = uniform_support(values)
        item["maximum_absolute_excess_pm"] = max(
            0.0,
            float(np.max(np.abs(local[:, axis]))) - metadata.dimensions_pm[axis] / 2.0,
        )
        support[name] = item
    residual = np.abs(local[:, 2])
    return {
        "transformed_variables": {
            name: uniform_statistics(values) for name, values in variables.items()
        },
        "correlation": correlations(variables),
        "lateral_support": support,
        "outside_lateral_support_count": int(np.count_nonzero(outside)),
        "local_z_residual_pm": {
            "signed_statistics": scalar_statistics(local[:, 2]),
            "nonzero_count": int(np.count_nonzero(residual)),
            "mean_absolute": float(np.mean(residual)),
            "median_absolute": float(np.median(residual)),
            "p95_absolute": float(np.percentile(residual, 95)),
            "maximum_absolute": float(np.max(residual)),
        },
    }, variables


def angular_measurements(
    directions: FloatArray, metadata: Metadata
) -> tuple[JsonObject, dict[str, FloatArray], NDArray[np.bool_]]:
    local_raw = (metadata.inverse @ directions.T).T
    norms = np.linalg.norm(local_raw, axis=1)
    if np.any(norms == 0.0):
        raise ValueError("Undefined inverse-frame direction.")
    local = local_raw / norms[:, None]
    defined_phi = (local[:, 0] != 0.0) | (local[:, 1] != 0.0)
    lower, upper, phi_min, phi_max = metadata.bounds
    u_cos = (local[:, 2] - lower) / (upper - lower)
    u_phi = (np.arctan2(local[defined_phi, 1], local[defined_phi, 0]) - phi_min) / (
        phi_max - phi_min
    )
    variables = {"u_cos": u_cos, "u_phi": u_phi}
    return (
        {
            "packed_bounds": {
                "cos_theta_lower": lower,
                "cos_theta_upper": upper,
                "phi_min_rad": phi_min,
                "phi_max_rad": phi_max,
            },
            "global_direction_norm": norm_diagnostics(directions),
            "recovered_local_raw_norm": norm_diagnostics(local_raw),
            "transformed_variables": {
                name: uniform_statistics(values) for name, values in variables.items()
            },
            "correlation": correlations({"u_cos": u_cos[defined_phi], "u_phi": u_phi}),
            "support": {
                name: uniform_support(values) for name, values in variables.items()
            },
            "undefined_phi_exclusions": int(np.count_nonzero(~defined_phi)),
        },
        variables,
        defined_phi,
    )


def energy_measurements(
    samples: Samples, metadata: Metadata
) -> tuple[JsonObject, IntArray]:
    # These finite-law definitions are the established E1 analytical counting
    # CDF. No tickets are generated, reconstructed, or exported by I1.
    regular = metadata.configuration.energy == "regular-spectrum"
    width = metadata.bin_width_micro_ev
    lower_edges = [center - width // 2 for center in metadata.energies_micro_ev]
    offsets: list[list[int]] = [[] for _ in metadata.energies_micro_ev]
    bin_indices = np.empty(metadata.count, dtype=np.int64)
    for local_id, energy in enumerate(samples.energies):
        if regular:
            memberships = [
                index
                for index, lower in enumerate(lower_edges)
                if lower <= energy < lower + width
            ]
        else:
            memberships = [
                index
                for index, center in enumerate(metadata.energies_micro_ev)
                if energy == center
            ]
        if len(memberships) != 1:
            raise ValueError(
                f"Energy {energy} micro-eV belongs to {len(memberships)} intended bins/lines."
            )
        index = memberships[0]
        if metadata.ticket_counts[index] == 0:
            raise ValueError("A zero-ticket energy entry was emitted.")
        bin_indices[local_id] = index
        offsets[index].append(energy - lower_edges[index])

    rows, metrics = probability_results(
        [len(values) for values in offsets], metadata.ticket_counts, metadata.count
    )
    for index, values in enumerate(offsets):
        rows[index]["center_energy_micro_eV"] = metadata.energies_micro_ev[index]
        if regular:
            rows[index].update(
                {
                    "lower_energy_micro_eV": lower_edges[index],
                    "upper_energy_micro_eV": lower_edges[index] + width,
                    "distinct_emitted_energy_count": len(set(values)),
                    "minimum_offset_micro_eV": min(values) if values else None,
                    "maximum_offset_micro_eV": max(values) if values else None,
                    "finite_conditional_cdf_max_deviation": finite_cdf_max_deviation(
                        values, metadata.ticket_counts[index], width
                    ),
                }
            )
    return {
        "representation": "uint64 micro-eV",
        "packed_configuration": metadata.raw["energy"],
        "bin_results": rows,
        "probability_metrics": metrics,
        "support": {
            "outside_all_bins_or_lines_count": 0,
            "ambiguous_membership_count": 0,
            "unreachable_energy_count": 0,
        },
        "distinct_emitted_energies": len(set(samples.energies)),
        "repeated_sample_count": metadata.count - len(set(samples.energies)),
    }, bin_indices


def structural_status(metadata: Metadata) -> JsonObject:
    return {
        "status": "complete",
        "observer": metadata.raw["observer"],
        "source_records_only": True,
        "complete_unique_provenance": True,
        "source_index": 0,
        "static_time_ps": 0,
        "weight": 1,
        "particle": "Gamma; exporter verifies every raw Source record (no CSV particle column)",
    }


def compare_pair(
    case: IntegrationCase,
    reference: Samples,
    comparison: Samples,
    reference_metadata: Metadata,
    comparison_metadata: Metadata,
) -> JsonObject:
    if not np.array_equal(reference.provenance, comparison.provenance):
        raise ValueError("Paired provenance domains differ.")
    for key in (
        "seed",
        "worker_count",
        "device_names",
        "device_selector",
        "primary_count",
        "source_index",
        "global_primary_begin",
        "global_primary_last",
        "run_id",
        "rng_engine",
        "source_center_pm",
        "frame_axes",
    ):
        if reference_metadata.raw[key] != comparison_metadata.raw[key]:
            raise ValueError(f"Paired execution configuration differs: {key}.")
    if reference_metadata.raw["worker_count"] != PAIR_WORKERS:
        raise ValueError("Exact I1 pairing requires one worker.")
    if (
        reference_metadata.configuration.angular
        == comparison_metadata.configuration.angular
        and (
            reference_metadata.configuration.bounds_deg
            == comparison_metadata.configuration.bounds_deg
        )
        and reference_metadata.bounds != comparison_metadata.bounds
    ):
        raise ValueError("Identical angular requests produced different packed bounds.")
    if (
        reference_metadata.configuration.energy
        == comparison_metadata.configuration.energy
        and (reference_metadata.raw["energy"] != comparison_metadata.raw["energy"])
    ):
        raise ValueError("Identical energy requests produced different packed tables.")

    results: JsonObject = {}
    for field in case.exact_fields:
        if field == "direction":
            difference = np.abs(reference.directions - comparison.directions)
            mismatches = int(np.count_nonzero(np.any(difference != 0.0, axis=1)))
            maximum = float(np.max(difference))
            results["direction_mismatch_count"] = mismatches
            results["maximum_component_direction_difference"] = maximum
        elif field == "position":
            differences = [
                [abs(int(left[axis]) - int(right[axis])) for axis in range(3)]
                for left, right in zip(
                    reference.positions, comparison.positions, strict=True
                )
            ]
            mismatches = sum(any(row) for row in differences)
            maximum = max(value for row in differences for value in row)
            results["position_mismatch_count"] = mismatches
            results["maximum_component_position_difference_pm"] = maximum
        else:
            left_values = reference.energies if field == "energy" else reference.times
            right_values = (
                comparison.energies if field == "energy" else comparison.times
            )
            deltas = [
                abs(left - right)
                for left, right in zip(left_values, right_values, strict=True)
            ]
            mismatches = sum(value != 0 for value in deltas)
            maximum = max(deltas)
            results[f"{field}_mismatch_count"] = mismatches
            unit = "micro-eV" if field == "energy" else "ps"
            results[f"maximum_exact_{field}_difference_{unit}"] = maximum
        if mismatches:
            raise ValueError(
                f"{case.name}: exact {field} contract failed: {mismatches} mismatches; maximum {maximum}."
            )
    return results


def analyze_case(
    case: IntegrationCase,
    case_dir: Path,
    output_dir: Path,
    *,
    make_figures: bool = True,
) -> JsonObject:
    output_dir.mkdir(parents=True, exist_ok=True)
    summary_path = output_dir / "summary.json"
    if summary_path.exists():
        raise FileExistsError(
            f"Refusing to overwrite an existing summary: {summary_path}"
        )
    paired = case.comparison is not None
    reference_name = case.name + ("_reference" if paired else "")
    reference_dir = case_dir / "reference" if paired else case_dir
    reference_metadata = load_metadata(
        reference_dir / "metadata.json", case.reference, reference_name, paired=paired
    )
    reference = load_samples(reference_dir / "samples.csv", reference_metadata)
    energy, bin_indices = energy_measurements(reference, reference_metadata)

    if case.comparison is not None:
        comparison_dir = case_dir / "comparison"
        comparison_metadata = load_metadata(
            comparison_dir / "metadata.json",
            case.comparison,
            case.name + "_comparison",
            paired=True,
        )
        comparison = load_samples(comparison_dir / "samples.csv", comparison_metadata)
        comparison_energy, _ = energy_measurements(comparison, comparison_metadata)
        results = compare_pair(
            case, reference, comparison, reference_metadata, comparison_metadata
        )
        summary: JsonObject = {
            "pair_name": case.name,
            "reference_configuration": reference_metadata.raw,
            "comparison_configuration": comparison_metadata.raw,
            "exact_worker_count": PAIR_WORKERS,
            "paired_sample_count": reference_metadata.count,
            "provenance_validation": {
                "identical_domains": True,
                "complete_unique": True,
                "pairing_key": "source_local_primary_id; global IDs also identical",
                "raw_csv_order_ignored": True,
            },
            "structural_validation": {
                "reference": structural_status(reference_metadata),
                "comparison": structural_status(comparison_metadata),
                "energy_support": [energy["support"], comparison_energy["support"]],
            },
            "expected_draw_budget": {
                "reference": draw_budget(case.reference),
                "comparison": draw_budget(case.comparison),
            },
            "exact_comparison_results": results,
            "acceptance_thresholds": None,
            "figures": [],
            "figure_status": "not_required_exact_pair",
        }
        print(
            f"{case.name}: N={reference_metadata.count}, workers=1; {results}",
            flush=True,
        )
    else:
        local = recover_local(reference.positions, reference_metadata)
        position, position_variables = position_measurements(local, reference_metadata)
        angular, angle_variables, defined_phi = angular_measurements(
            reference.directions, reference_metadata
        )
        common = {
            "u_x": position_variables["u_x"][defined_phi],
            "u_y": position_variables["u_y"][defined_phi],
            "u_cos": angle_variables["u_cos"][defined_phi],
            "u_phi": angle_variables["u_phi"],
        }
        conditional: list[JsonObject] = []
        for index, center in enumerate(reference_metadata.energies_micro_ev):
            in_bin = bin_indices == index
            phi_in_bin = bin_indices[defined_phi] == index
            conditional.append(
                {
                    "bin_index": index,
                    "center_energy_micro_eV": center,
                    "sample_count": int(np.count_nonzero(in_bin)),
                    "phi_sample_count": int(np.count_nonzero(phi_in_bin)),
                    "means": {
                        "u_x": float(np.mean(position_variables["u_x"][in_bin]))
                        if np.any(in_bin)
                        else None,
                        "u_y": float(np.mean(position_variables["u_y"][in_bin]))
                        if np.any(in_bin)
                        else None,
                        "u_cos": float(np.mean(angle_variables["u_cos"][in_bin]))
                        if np.any(in_bin)
                        else None,
                        "u_phi": float(np.mean(angle_variables["u_phi"][phi_in_bin]))
                        if np.any(phi_in_bin)
                        else None,
                    },
                }
            )
        correlation = correlations(common)
        summary = {
            "case_name": case.name,
            "sample_count": reference_metadata.count,
            "structural_validation": structural_status(reference_metadata),
            "metadata": reference_metadata.raw,
            "frame": reference_metadata.frame,
            "position_validation": position,
            "angular_validation": angular,
            "energy_validation": energy,
            "cross_component_diagnostics": {
                "correlation": correlation,
                "conditional_means_per_energy_bin": conditional,
                "interpretation": "Descriptive; small correlations do not prove independence.",
            },
            "numerical_representation": {
                "positions": "Committed int64 pm; exact integer center subtraction before binary64 inverse(M)",
                "directions_and_frame": "Reconstructed actual binary32, then binary64 analysis",
                "energy": "Current uint64 micro-eV; exact packed integer ticket-induced law",
                "future_energy_migration": "Approved ueV migration is separate and requires new E1/I1 validation",
                "local_z": "Binary32 displacement/frame arithmetic and integer-pm commitment residuals; no clipping",
            },
            "acceptance_thresholds": None,
            "figures": [],
            "figure_status": "not_requested" if not make_figures else "pending",
        }
        if make_figures:
            import plot  # pyright: ignore[reportImplicitRelativeImport]

            width = reference_metadata.bin_width_micro_ev
            scale = reference_metadata.display_unit_micro_ev
            lower_edges = [
                value - width // 2 for value in reference_metadata.energies_micro_ev
            ]
            edges = (
                np.array(lower_edges + [lower_edges[-1] + width], dtype=np.float64)
                / scale
            )
            expected_density = (
                np.array(reference_metadata.ticket_counts, dtype=np.float64)
                / TICKET_SPACE
                / (width / scale)
            )
            summary["figures"] = plot.plot_integrated(
                case.name,
                local / reference_metadata.display_unit_pm,
                common,
                np.array(reference.energies, dtype=np.float64) / scale,
                edges,
                expected_density,
                np.array(correlation["pearson_matrix"], dtype=np.float64),
                output_dir,
            )
            summary["figure_status"] = "generated_png_and_pdf"
        print(
            f"{case.name}: N={reference_metadata.count}; lateral outside={position['outside_lateral_support_count']}; "
            + f"energy {energy['probability_metrics']}; figures={summary['figure_status']}",
            flush=True,
        )

    _ = summary_path.write_text(
        json.dumps(summary, indent=2, allow_nan=False) + "\n", encoding="utf-8"
    )
    return summary


class Arguments(Protocol):
    case: str
    case_dir: Path
    output_dir: Path
    no_plots: bool


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Analyze actual I1 Source CSV/metadata, including exact pairs."
    )
    _ = parser.add_argument(
        "--case", choices=[case.name for case in CASES], required=True
    )
    _ = parser.add_argument("--case-dir", type=Path, required=True)
    _ = parser.add_argument("--output-dir", type=Path, required=True)
    _ = parser.add_argument("--no-plots", action="store_true")
    args = cast(Arguments, cast(object, parser.parse_args()))
    case = next(case for case in CASES if case.name == args.case)
    _ = analyze_case(
        case,
        args.case_dir.resolve(),
        args.output_dir.resolve(),
        make_figures=not args.no_plots,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
