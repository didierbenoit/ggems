import argparse
import csv
import json
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

import numpy as np
from numpy.typing import NDArray

type FloatArray = NDArray[np.float64]
type IntArray = NDArray[np.int64]
type JsonObject = dict[str, object]

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
    case_name: str
    geometry: str
    dimensions_pm: tuple[int, int, int]
    dimensions_mm: tuple[float, float, float]
    primary_count: int
    source_index: int
    global_primary_begin: int
    energy_meV: int
    time_ps: int
    weight: float
    raw: JsonObject


def _object(value: object, name: str) -> JsonObject:
    if not isinstance(value, dict):
        raise TypeError(f"{name} must be a JSON object.")
    return cast(JsonObject, value)


def _integer(value: object, name: str, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{name} must be an integer.")
    if not minimum <= value <= (1 << 64) - 1:
        raise ValueError(f"{name} is outside its unsigned integer domain.")
    return value


def _number(value: object, name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"{name} must be numeric.")
    result = float(value)
    if not math.isfinite(result):
        raise ValueError(f"{name} must be finite.")
    return result


def _triple(value: object, name: str) -> list[object]:
    if not isinstance(value, list):
        raise TypeError(f"{name} must contain three coordinates.")
    result = cast(list[object], value)
    if len(result) != 3:
        raise ValueError(f"{name} must contain three coordinates.")
    return result


def load_metadata(path: Path) -> Metadata:
    raw = _object(cast(object, json.loads(path.read_text(encoding="utf-8"))), str(path))
    geometry = raw.get("geometry")
    if geometry not in (
        "point",
        "rectangle",
        "ellipse",
        "circle",
        "box",
        "sphere",
        "cylinder",
    ):
        raise ValueError("Unknown G1 geometry in metadata.")
    case_name = raw.get("case_name")
    if not isinstance(case_name, str) or not case_name:
        raise ValueError("Metadata must contain a nonempty case_name.")
    selector = raw.get("device_selector")
    if not isinstance(selector, str) or not selector:
        raise ValueError("Metadata must contain a nonempty device_selector.")
    device_names = raw.get("device_names")
    if not isinstance(device_names, list):
        raise TypeError("Metadata device_names must be a list.")
    names = cast(list[object], device_names)
    if not names or any(not isinstance(name, str) or not name for name in names):
        raise ValueError("Metadata must contain nonempty selected device names.")
    for key, expected in {
        "population_mode": "CountDriven",
        "rng_engine": "Philox",
        "particle": "Gamma",
        "angular_mode": "Fixed",
        "energy_mode": "Mono",
        "chronology": "static",
        "source_center_pm": [0, 0, 0],
        "frame_axes": [[1, 0, 0], [0, 1, 0], [0, 0, 1]],
        "fixed_direction": [0, 0, 1],
        "time_ps": 0,
        "source_index": 0,
        "global_primary_begin": 0,
        "weight": 1,
    }.items():
        if raw.get(key) != expected:
            raise ValueError(
                f"Metadata violates G1 configuration: {key} must be {expected}."
            )
    dimensions = tuple(
        _integer(value, "dimensions_pm")
        for value in _triple(raw.get("dimensions_pm"), "dimensions_pm")
    )
    display = tuple(
        _number(value, "dimensions_mm")
        for value in _triple(raw.get("dimensions_mm"), "dimensions_mm")
    )
    required_axes = (
        0
        if geometry == "point"
        else (2 if geometry in ("rectangle", "ellipse", "circle") else 3)
    )
    for axis in range(3):
        if (dimensions[axis] > 0) != (axis < required_axes) or (display[axis] > 0) != (
            axis < required_axes
        ):
            raise ValueError("Metadata dimensions do not match the geometry.")
        if axis >= required_axes and (dimensions[axis] != 0 or display[axis] != 0):
            raise ValueError("Unused geometry dimensions must be zero.")
    if geometry in ("circle", "sphere", "cylinder") and dimensions[0] != dimensions[1]:
        raise ValueError("Circular geometry requires equal X/Y diameters.")
    if geometry == "sphere" and dimensions[0] != dimensions[2]:
        raise ValueError("Sphere requires equal X/Y/Z diameters.")
    primary_count = _integer(raw.get("primary_count"), "primary_count", 1)
    worker_count = _integer(raw.get("worker_count"), "worker_count", 1)
    if primary_count > ((1 << 32) - 1) // 2 or worker_count > (1 << 32) - 1:
        raise ValueError("Counts exceed the current Observer or worker interface.")
    _ = _integer(raw.get("seed"), "seed")
    for value in _triple(raw.get("source_center_pm"), "source_center_pm"):
        _ = _integer(value, "source_center_pm")
    for value in _triple(raw.get("fixed_direction"), "fixed_direction"):
        _ = _number(value, "fixed_direction")
    for frame_axis in _triple(raw.get("frame_axes"), "frame_axes"):
        for value in _triple(frame_axis, "frame_axes vector"):
            _ = _number(value, "frame_axes vector")
    observer = _object(raw.get("observer"), "observer")
    for key, expected in {
        "overflow_count": 0,
        "record_count": 2 * primary_count,
        "captured_primary_count": primary_count,
        "source_record_count": primary_count,
    }.items():
        if _integer(observer.get(key), f"observer.{key}") != expected:
            raise ValueError(f"Incomplete Observer capture: {key} must be {expected}.")
    return Metadata(
        case_name=case_name,
        geometry=cast(str, geometry),
        dimensions_pm=cast(tuple[int, int, int], dimensions),
        dimensions_mm=cast(tuple[float, float, float], display),
        primary_count=primary_count,
        source_index=_integer(raw.get("source_index"), "source_index"),
        global_primary_begin=_integer(
            raw.get("global_primary_begin"), "global_primary_begin"
        ),
        energy_meV=_integer(raw.get("energy_meV"), "energy_meV", 1),
        time_ps=_integer(raw.get("time_ps"), "time_ps"),
        weight=_number(raw.get("weight"), "weight"),
        raw=raw,
    )


def _decimal(value: str, minimum: int, maximum: int) -> int:
    digits = value.removeprefix("-")
    if not digits or not digits.isascii() or not digits.isdecimal():
        raise ValueError(f"Expected a decimal integer, received {value!r}.")
    result = int(value)
    if not minimum <= result <= maximum:
        raise ValueError(f"Integer {value!r} is outside its serialized field range.")
    return result


def load_positions(path: Path, metadata: Metadata) -> IntArray:
    positions = np.empty((metadata.primary_count, 3), dtype=np.int64)
    count = 0
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.reader(stream, strict=True)
        if tuple(next(reader, ())) != CSV_COLUMNS:
            raise ValueError(f"Unexpected CSV header in {path}.")
        for local_id, row in enumerate(reader):
            if len(row) != len(CSV_COLUMNS) or local_id >= metadata.primary_count:
                raise ValueError(
                    f"Malformed or excess CSV row {local_id + 2} in {path}."
                )
            try:
                slot = _decimal(row[0], 0, (1 << 32) - 1)
                source_local = _decimal(row[1], 0, (1 << 64) - 1)
                global_id = _decimal(row[2], 0, (1 << 64) - 1)
                if (
                    slot != metadata.source_index
                    or source_local != local_id
                    or global_id != metadata.global_primary_begin + local_id
                ):
                    raise ValueError(
                        "Missing, duplicate, unordered, or impossible primary provenance."
                    )
                if row[12] != "Source":
                    raise ValueError("Only Source records are accepted.")
                for axis in range(3):
                    positions[local_id, axis] = _decimal(
                        row[3 + axis], -(1 << 63), (1 << 63) - 1
                    )
                direction = tuple(float(value) for value in row[6:9])
                energy = _decimal(row[9], 0, (1 << 64) - 1)
                time = _decimal(row[10], 0, (1 << 64) - 1)
                weight = float(row[11])
                if (
                    direction != (0.0, 0.0, 1.0)
                    or energy != metadata.energy_meV
                    or time != metadata.time_ps
                    or weight != metadata.weight
                ):
                    raise ValueError(
                        "Record does not match the fixed G1 initialization contract."
                    )
            except ValueError as error:
                raise ValueError(f"{path}, row {local_id + 2}: {error}") from error
            count += 1
    if count != metadata.primary_count:
        raise ValueError(
            f"Expected {metadata.primary_count} Source records, found {count}."
        )
    return positions


def uniform_statistics(values: FloatArray) -> JsonObject:
    finite = values[np.isfinite(values)]
    count = finite.size
    if count == 0:
        return {
            "sample_count": 0,
            "minimum": None,
            "maximum": None,
            "mean": None,
            "variance": None,
            "ecdf_max_deviation": None,
        }
    ordered = np.sort(finite)
    # These are the reference CDF's constant tails; samples are never clipped.
    reference = np.where(ordered < 0.0, 0.0, np.where(ordered > 1.0, 1.0, ordered))
    right = np.arange(1, count + 1, dtype=np.float64) / count
    left = np.arange(count, dtype=np.float64) / count
    deviation = max(float(np.max(right - reference)), float(np.max(reference - left)))
    return {
        "sample_count": int(count),
        "minimum": float(np.min(ordered)),
        "maximum": float(np.max(ordered)),
        "mean": float(np.mean(finite)),
        "variance": float(np.var(finite)),
        "ideal_mean": 0.5,
        "ideal_variance": 1.0 / 12.0,
        "ecdf_max_deviation": deviation,
        "below_zero_count": int(np.count_nonzero(finite < 0.0)),
        "above_one_count": int(np.count_nonzero(finite > 1.0)),
    }


def correlations(variables: dict[str, FloatArray]) -> JsonObject:
    names = list(variables)
    matrix: list[list[float | None]] = []
    counts: list[list[int]] = []
    for first in names:
        row: list[float | None] = []
        row_counts: list[int] = []
        for second in names:
            valid = np.isfinite(variables[first]) & np.isfinite(variables[second])
            left, right = variables[first][valid], variables[second][valid]
            row_counts.append(int(left.size))
            if left.size < 2 or np.var(left) == 0.0 or np.var(right) == 0.0:
                row.append(None)
            else:
                row.append(float(cast(np.float64, np.corrcoef(left, right)[0, 1])))
        matrix.append(row)
        counts.append(row_counts)
    return {"variables": names, "pearson_matrix": matrix, "pair_sample_counts": counts}


def _axis_support(values: IntArray, full_size: int) -> JsonObject:
    maximum = max(abs(int(np.min(values))), abs(int(np.max(values))))
    outside = (values < -(full_size // 2)) | (values > full_size // 2)
    excess = max(0, 2 * maximum - full_size) / 2.0
    return {
        "outside_count": int(np.count_nonzero(outside)),
        "maximum_absolute_excess_pm": excess,
        "maximum_normalized_excess": excess / (full_size / 2.0) if full_size else None,
    }


def _duplicates(values: IntArray) -> JsonObject:
    _, counts = np.unique(values, axis=0, return_counts=True)
    return {
        "unique_count": int(counts.size),
        "repeated_sample_count": len(values) - int(counts.size),
        "repeated_coordinate_count": int(np.count_nonzero(counts > 1)),
        "maximum_multiplicity": int(np.max(counts)),
    }


def measure_geometry(
    positions: IntArray, metadata: Metadata
) -> tuple[JsonObject, dict[str, FloatArray]]:
    xyz = positions.astype(np.float64)
    x, y, z = xyz[:, 0], xyz[:, 1], xyz[:, 2]
    width, height, depth = metadata.dimensions_pm
    geometry = metadata.geometry
    variables: dict[str, FloatArray] = {}
    support: JsonObject = {}
    exclusions: JsonObject = {}
    summary: JsonObject = {
        "case_name": metadata.case_name,
        "geometry": geometry,
        "sample_count": metadata.primary_count,
        "structural_validation": {
            "status": "complete",
            "observer_overflow_count": 0,
            "provenance_complete_unique_and_ordered": True,
            "source_records_only": True,
        },
        "metadata": metadata.raw,
        "numerical_representation": {
            "input": "Exact decimal integer-pm positions from production Source records",
            "analysis": "Binary64 analytical transforms; exact integer ideal-support membership",
            "sampling": "Binary32 geometry arithmetic followed by integer-pm commitment",
            "variance_convention": "Population moment (ddof=0)",
            "acceptance_thresholds": None,
        },
        "lattice_repetitions": {
            "xyz": _duplicates(positions),
            **{
                axis: _duplicates(positions[:, index])
                for index, axis in enumerate(("x", "y", "z"))
            },
        },
    }
    if geometry == "point":
        nonzero = int(np.count_nonzero(positions))
        maximum = max(abs(int(np.min(positions))), abs(int(np.max(positions))))
        summary["exact_point"] = {
            "nonzero_coordinate_count": nonzero,
            "maximum_absolute_coordinate_pm": maximum,
        }
        if nonzero:
            raise ValueError(
                f"Point contract failure: {nonzero} nonzero coordinates, maximum {maximum} pm."
            )
        support = {"outside_count": 0, "maximum_absolute_excess_pm": 0}
    elif geometry in ("rectangle", "box"):
        variables = {"u_x": x / width + 0.5, "u_y": y / height + 0.5}
        axes = 2 if geometry == "rectangle" else 3
        if geometry == "box":
            variables["u_z"] = z / depth + 0.5
        outside = np.zeros(metadata.primary_count, dtype=np.bool_)
        for axis in range(axes):
            size = metadata.dimensions_pm[axis]
            support[("x", "y", "z")[axis]] = _axis_support(positions[:, axis], size)
            outside |= (positions[:, axis] < -(size // 2)) | (
                positions[:, axis] > size // 2
            )
        support["outside_count"] = int(np.count_nonzero(outside))
    else:
        radius_xy = cast(FloatArray, np.hypot(x, y))
        zero_xy = cast(NDArray[np.bool_], radius_xy == 0.0)
        if geometry in ("ellipse", "circle"):
            normalized_radius = np.hypot(2.0 * x / width, 2.0 * y / height)
            variables["q"] = normalized_radius**2
            # Python integers avoid int64 overflow and floating support classification.
            denominator = width**2 * height**2
            maximum_surplus = 0
            outside = np.empty(metadata.primary_count, dtype=np.bool_)
            for index, (px, py) in enumerate(
                zip(
                    cast(list[int], positions[:, 0].tolist()),
                    cast(list[int], positions[:, 1].tolist()),
                    strict=True,
                )
            ):
                surplus = 4 * (px**2 * height**2 + py**2 * width**2) - denominator
                outside[index] = surplus > 0
                maximum_surplus = max(maximum_surplus, surplus)
            phi = np.mod(np.arctan2(y / height, x / width), 2.0 * np.pi) / (2.0 * np.pi)
        else:
            radial = (
                cast(FloatArray, np.hypot(radius_xy, z))
                if geometry == "sphere"
                else radius_xy
            )
            normalized_radius = 2.0 * radial / width
            variables["q_r"] = normalized_radius ** (3 if geometry == "sphere" else 2)
            used_axes = 3 if geometry == "sphere" else 2
            denominator = width**2
            maximum_surplus = 0
            outside = np.empty(metadata.primary_count, dtype=np.bool_)
            for index, row in enumerate(cast(list[list[int]], positions.tolist())):
                surplus = 4 * sum(value**2 for value in row[:used_axes]) - denominator
                outside[index] = surplus > 0
                maximum_surplus = max(maximum_surplus, surplus)
            phi = np.mod(np.arctan2(y, x), 2.0 * np.pi) / (2.0 * np.pi)
            if geometry == "sphere":
                nonzero = cast(NDArray[np.bool_], radial != 0.0)
                cosine = np.full(metadata.primary_count, np.nan, dtype=np.float64)
                cosine[nonzero] = (z[nonzero] / radial[nonzero] + 1.0) / 2.0
                variables["u_cos"] = cosine
                exclusions["zero_radius_count"] = int(np.count_nonzero(~nonzero))
                exclusions["nonzero_pole_phi_exclusion_count"] = int(
                    np.count_nonzero(nonzero & zero_xy)
                )
            else:
                variables["u_z"] = z / depth + 0.5
                support["axial"] = _axis_support(positions[:, 2], depth)
        phi[zero_xy] = np.nan
        variables["u_phi"] = phi
        exclusions["phi_exclusion_count"] = int(np.count_nonzero(zero_xy))
        if geometry != "sphere":
            exclusions["zero_radius_count"] = int(np.count_nonzero(zero_xy))
        # Subtract in integers first; sqrt(q) - 1 loses tiny support excursions.
        squared_radius_excess = maximum_surplus / denominator
        maximum_excess = squared_radius_excess / (
            math.sqrt(1.0 + squared_radius_excess) + 1.0
        )
        support["radial"] = {
            "outside_count": int(np.count_nonzero(outside)),
            "maximum_normalized_excess": maximum_excess,
            "maximum_squared_normalized_radius_excess": squared_radius_excess,
        }
        if geometry not in ("ellipse", "circle"):
            radial_support = cast(JsonObject, support["radial"])
            radial_support["maximum_absolute_excess_pm"] = maximum_excess * (
                width / 2.0
            )
        if geometry == "cylinder":
            outside |= (positions[:, 2] < -(depth // 2)) | (
                positions[:, 2] > depth // 2
            )
        support["outside_count"] = int(np.count_nonzero(outside))
    if geometry in ("rectangle", "ellipse", "circle"):
        nonzero_z = int(np.count_nonzero(positions[:, 2]))
        support["plane"] = _axis_support(positions[:, 2], 0)
        if nonzero_z:
            raise ValueError(
                f"Canonical planar contract failure: {nonzero_z} nonzero Z coordinates."
            )
    summary["transformed_variables"] = {
        name: uniform_statistics(values) for name, values in variables.items()
    }
    summary["correlations"] = correlations(variables)
    summary["support"] = support
    summary["exclusions"] = exclusions
    return summary, variables


def analyze_case(
    samples_path: Path,
    metadata_path: Path,
    output_dir: Path,
    *,
    make_figures: bool = True,
) -> JsonObject:
    metadata = load_metadata(metadata_path)
    positions = load_positions(samples_path, metadata)
    summary, variables = measure_geometry(positions, metadata)
    output_dir.mkdir(parents=True, exist_ok=True)
    summary["figures"] = []
    summary["figure_status"] = "not_requested"
    if make_figures and metadata.geometry != "point":
        # These validation scripts are launched directly from their directory.
        from plot import plot_geometry  # pyright: ignore[reportImplicitRelativeImport]

        summary["figures"] = [
            str(path)
            for path in plot_geometry(
                positions,
                metadata.geometry,
                metadata.case_name,
                metadata.dimensions_pm,
                metadata.dimensions_mm,
                variables,
                output_dir,
            )
        ]
        summary["figure_status"] = "generated"
    elif metadata.geometry == "point":
        summary["figure_status"] = "not_required"
    _ = (output_dir / "summary.json").write_text(
        json.dumps(summary, indent=2, allow_nan=False) + "\n", encoding="utf-8"
    )
    print(
        f"{metadata.case_name}: {metadata.primary_count} Source records; structure complete; overflow=0"
    )
    for name, stats in cast(
        dict[str, JsonObject], summary["transformed_variables"]
    ).items():
        print(
            f"  {name}: n={stats['sample_count']}, mean={stats['mean']}, variance={stats['variance']}, D={stats['ecdf_max_deviation']}"
        )
    print(f"  support: {summary['support']}")
    if metadata.geometry == "point":
        print(f"  exact Point: {summary['exact_point']}")
    print(f"  summary: {output_dir / 'summary.json'}")
    return summary


class Arguments(Protocol):
    samples: Path
    metadata: Path
    output_dir: Path
    no_plots: bool


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Measure analytical G1 laws from exported production Source records."
    )
    _ = parser.add_argument("--samples", type=Path, required=True)
    _ = parser.add_argument("--metadata", type=Path, required=True)
    _ = parser.add_argument("--output-dir", type=Path, required=True)
    _ = parser.add_argument(
        "--no-plots",
        action="store_true",
        help="Write measurements only; skip the Matplotlib extraction boundary.",
    )
    args = cast(Arguments, cast(object, parser.parse_args()))
    _ = analyze_case(
        args.samples, args.metadata, args.output_dir, make_figures=not args.no_plots
    )


if __name__ == "__main__":
    main()
