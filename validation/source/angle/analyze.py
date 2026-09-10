import argparse
import csv
import json
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

import numpy as np
from numpy.typing import NDArray

# Direct script entry points use the modules beside this file.
from cases import CASES, AngleCase  # pyright: ignore[reportImplicitRelativeImport]

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
    "energy_micro_eV",
    "time_ps",
    "weight",
    "record_kind",
)
BOUND_FIELDS = (
    "isotropic_cos_theta_lower",
    "isotropic_cos_theta_upper",
    "isotropic_phi_min_rad",
    "isotropic_phi_max_rad",
)


@dataclass(frozen=True, slots=True)
class Metadata:
    case: AngleCase
    primary_count: int
    dimensions_pm: tuple[int, int, int]
    fixed_direction: tuple[float, float, float]
    bounds: tuple[float, float, float, float]
    focus_pm: tuple[int, int, int]
    raw: JsonObject


def _object(value: object, name: str) -> JsonObject:
    if not isinstance(value, dict):
        raise TypeError(f"{name} must be a JSON object.")
    return cast(JsonObject, value)


def _integer(
    value: object, name: str, minimum: int = 0, maximum: int = (1 << 64) - 1
) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{name} must be an integer.")
    if not minimum <= value <= maximum:
        raise ValueError(f"{name} is outside its integer field range.")
    return value


def _number(value: object, name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"{name} must be numeric.")
    result = float(value)
    if not math.isfinite(result):
        raise ValueError(f"{name} must be finite.")
    return result


def _binary32(value: object, name: str) -> float:
    result = _number(value, name)
    if (
        abs(result) > float(np.finfo(np.float32).max)
        or float(np.float32(result)) != result
    ):
        raise ValueError(f"{name} must preserve the exact packed binary32 value.")
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
    case = next((item for item in CASES if item.name == raw.get("case_name")), None)
    if case is None:
        raise ValueError("Unknown canonical A1 case in metadata.")

    mode = {
        "fixed": "Fixed",
        "isotropic": "Isotropic",
        "bounded-isotropic": "Isotropic",
        "focused": "Focused",
    }[case.angular]
    for key, expected in {
        "geometry": case.geometry,
        "angular_configuration": case.angular,
        "angular_mode": mode,
        "population_mode": "CountDriven",
        "rng_engine": "Philox",
        "particle": "Gamma",
        "energy_mode": "Mono",
        "energy_micro_eV": 511_000_000_000,
        "chronology": "static",
        "time_ps": 0,
        "weight": 1,
        "source_center_pm": [0, 0, 0],
        "frame_axes": [[1, 0, 0], [0, 1, 0], [0, 0, 1]],
        "fixed_direction": [0, 0, 1],
        "source_index": 0,
        "global_primary_begin": 0,
    }.items():
        if raw.get(key) != expected:
            raise ValueError(
                f"Metadata violates A1 configuration: {key} must be {expected}."
            )

    for key in ("energy_micro_eV", "time_ps", "source_index", "global_primary_begin"):
        _ = _integer(raw.get(key), key)
    _ = _number(raw.get("weight"), "weight")
    for value in _triple(raw.get("source_center_pm"), "source_center_pm"):
        _ = _integer(value, "source_center_pm", -(1 << 63), (1 << 63) - 1)
    for axis in _triple(raw.get("frame_axes"), "frame_axes"):
        for value in _triple(axis, "frame axis"):
            _ = _binary32(value, "frame axis")

    fixed = tuple(
        _binary32(value, "fixed_direction")
        for value in _triple(raw.get("fixed_direction"), "fixed_direction")
    )
    dimensions = tuple(
        _integer(value, "dimensions_pm")
        for value in _triple(raw.get("dimensions_pm"), "dimensions_pm")
    )
    display = tuple(
        _number(value, "dimensions_mm")
        for value in _triple(raw.get("dimensions_mm"), "dimensions_mm")
    )
    if display != case.dimensions_mm:
        raise ValueError("Dimensions do not describe the selected A1 case.")
    if case.geometry == "point":
        if dimensions != (0, 0, 0):
            raise ValueError("Point dimensions must be exactly zero.")
    elif dimensions[0] <= 0 or dimensions[1] <= 0 or dimensions[2] != 0:
        raise ValueError("Rectangle requires positive X/Y dimensions and zero Z.")

    bounds = tuple(_binary32(raw.get(key), key) for key in BOUND_FIELDS)
    lower, upper, phi_min, phi_max = bounds
    if not (-1.0 <= lower < upper <= 1.0 and phi_min < phi_max):
        raise ValueError("Invalid packed angular domain.")
    requested = raw.get("requested_bounded_degrees")
    if case.bounds_deg is not None:
        requested_bounds = _object(requested, "requested_bounded_degrees")
        degrees = tuple(
            _number(requested_bounds.get(key), key)
            for key in ("theta_min", "theta_max", "phi_min", "phi_max")
        )
        if degrees != case.bounds_deg:
            raise ValueError(
                "Requested bounds do not match the canonical bounded A1 case."
            )
        if not (-math.pi < phi_min < phi_max < math.pi):
            raise ValueError(
                "A1 bounded phi must lie inside atan2's ordinary interval."
            )
    elif requested is not None:
        raise ValueError("Only the bounded case may specify requested angular bounds.")
    if case.angular == "isotropic" and bounds != (
        -1.0,
        1.0,
        0.0,
        float(np.float32(math.tau)),
    ):
        raise ValueError(
            "The full-sphere case must use the exact canonical packed domain."
        )

    focus = tuple(
        _integer(value, "focus_position_pm", -(1 << 63), (1 << 63) - 1)
        for value in _triple(raw.get("focus_position_pm"), "focus_position_pm")
    )
    focus_display = tuple(
        _number(value, "focus_position_mm")
        for value in _triple(raw.get("focus_position_mm"), "focus_position_mm")
    )
    if focus_display != (case.focus_mm or (0.0, 0.0, 0.0)):
        raise ValueError("Focus does not match the selected A1 case.")
    if case.angular == "focused":
        if focus[0] != 0 or focus[1] != 0 or focus[2] <= 0:
            raise ValueError("Canonical Focused requires a global focus on positive Z.")
    elif focus != (0, 0, 0):
        raise ValueError("Inactive focus coordinates must be zero.")

    primary_count = _integer(
        raw.get("primary_count"), "primary_count", 1, ((1 << 32) - 1) // 2
    )
    _ = _integer(raw.get("worker_count"), "worker_count", 1, (1 << 32) - 1)
    _ = _integer(raw.get("seed"), "seed")
    selector = raw.get("device_selector")
    if not isinstance(selector, str) or not selector:
        raise ValueError("Metadata must contain a nonempty device_selector.")
    device_names = raw.get("device_names")
    if not isinstance(device_names, list):
        raise TypeError("Metadata device_names must be a list.")
    names = cast(list[object], device_names)
    if not names or any(not isinstance(name, str) or not name for name in names):
        raise ValueError("Metadata must identify the selected devices.")

    observer = _object(raw.get("observer"), "observer")
    for key, expected in {
        "overflow_count": 0,
        "record_count": 2 * primary_count,
        "captured_primary_count": primary_count,
        "source_record_count": primary_count,
    }.items():
        if _integer(observer.get(key), f"observer.{key}") != expected:
            raise ValueError(f"Incomplete Observer capture: {key} must be {expected}.")
    for key in ("capacity_per_device", "host_capacity"):
        _ = _integer(observer.get(key), f"observer.{key}", 1, (1 << 32) - 1)

    return Metadata(
        case=case,
        primary_count=primary_count,
        dimensions_pm=cast(tuple[int, int, int], dimensions),
        fixed_direction=cast(tuple[float, float, float], fixed),
        bounds=cast(tuple[float, float, float, float], bounds),
        focus_pm=cast(tuple[int, int, int], focus),
        raw=raw,
    )


def _decimal(value: str, minimum: int, maximum: int) -> int:
    digits = value.removeprefix("-")
    if not digits or not digits.isascii() or not digits.isdecimal():
        raise ValueError(f"Expected an exact decimal integer, received {value!r}.")
    result = int(value)
    if not minimum <= result <= maximum:
        raise ValueError(f"Integer {value!r} is outside its serialized field range.")
    return result


def load_samples(path: Path, metadata: Metadata) -> tuple[IntArray, FloatArray]:
    positions = np.empty((metadata.primary_count, 3), dtype=np.int64)
    directions = np.empty((metadata.primary_count, 3), dtype=np.float64)
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
                if slot != 0 or source_local != local_id or global_id != local_id:
                    raise ValueError(
                        "Missing, duplicate, unordered, or impossible primary provenance."
                    )
                if row[12] != "Source":
                    raise ValueError("Only Source records are accepted.")

                for axis in range(3):
                    positions[local_id, axis] = _decimal(
                        row[3 + axis], -(1 << 63), (1 << 63) - 1
                    )
                    value = float(row[6 + axis])
                    if (
                        not math.isfinite(value)
                        or abs(value) > float(np.finfo(np.float32).max)
                    ):
                        raise ValueError(
                            "Direction components must be finite binary32 values."
                        )
                    # CSV max_digits10 preserves float bits when read as binary32.
                    # Promote the reconstructed value, not the decimal approximation.
                    directions[local_id, axis] = float(np.float32(value))

                energy = _decimal(row[9], 0, (1 << 64) - 1)
                time = _decimal(row[10], 0, (1 << 64) - 1)
                weight = float(row[11])
                if (
                    energy != metadata.raw["energy_micro_eV"]
                    or time != 0
                    or weight != 1.0
                ):
                    raise ValueError(
                        "Record violates the Mono energy, static time, or weight contract."
                    )
            except ValueError as error:
                raise ValueError(f"{path}, row {local_id + 2}: {error}") from error
            count += 1

    if count != metadata.primary_count:
        raise ValueError(
            f"Expected {metadata.primary_count} Source records, found {count}."
        )
    if metadata.case.geometry == "point" and np.any(positions != 0):
        raise ValueError("Canonical Point must emit at the exact origin.")
    if metadata.case.geometry == "rectangle" and np.any(positions[:, 2] != 0):
        raise ValueError("Canonical Rectangle must emit in the exact Z=0 plane.")

    return positions, directions


def uniform_statistics(values: FloatArray) -> JsonObject:
    count = values.size
    result: JsonObject = {
        "sample_count": int(count),
        "minimum": None,
        "maximum": None,
        "mean": None,
        "variance": None,
        "ideal_mean": 0.5,
        "ideal_variance": 1.0 / 12.0,
        "ecdf_max_deviation": None,
    }
    if count == 0:
        return result

    ordered = np.sort(values)
    # Constant tails belong to the ideal CDF. Observations are never clipped.
    reference = np.where(ordered < 0.0, 0.0, np.where(ordered > 1.0, 1.0, ordered))
    right = np.arange(1, count + 1, dtype=np.float64) / count
    left = np.arange(count, dtype=np.float64) / count
    result.update(
        {
            "minimum": float(ordered[0]),
            "maximum": float(ordered[-1]),
            "mean": float(np.mean(values)),
            "variance": float(np.var(values)),
            "ecdf_max_deviation": max(
                float(np.max(right - reference)), float(np.max(reference - left))
            ),
        }
    )
    return result


def norm_statistics(norms: FloatArray) -> JsonObject:
    return {
        "minimum": float(np.min(norms)),
        "maximum": float(np.max(norms)),
        "mean": float(np.mean(norms)),
        "maximum_absolute_error": float(np.max(np.abs(norms - 1.0))),
    }


def measure_angle(
    positions: IntArray, directions: FloatArray, metadata: Metadata
) -> tuple[JsonObject, dict[str, FloatArray]]:
    norms = cast(FloatArray, np.linalg.norm(directions, axis=1))
    if np.any(norms == 0.0):
        raise ValueError("A zero direction does not define an emitted ray.")

    summary: JsonObject = {
        "case_name": metadata.case.name,
        "sample_count": metadata.primary_count,
        "structural_validation": {
            "status": "complete",
            "observer_overflow_count": 0,
            "provenance_complete_unique_and_ordered": True,
            "source_records_only": True,
            "finite_direction_components": True,
        },
        "metadata": metadata.raw,
        "numerical_representation": {
            "input": "Exact integer-pm positions; reconstructed binary32 directions",
            "analysis": "Binary64 after reconstruction; observations never clipped",
            "norms": "Measured before any analytical normalization",
            "bounds": "Actual packed binary32 metadata; descriptive requested degrees",
            "focused_reference": "Exact integer subtraction, then binary64 geometry",
            "variance_convention": "Population moment (ddof=0)",
        },
        "norm": norm_statistics(norms),
        "transformed_variables": {},
        "cartesian_moments": {},
        "correlations": {},
        "support": {},
        "exclusions": {},
        "focused": None,
        "exact_fixed": None,
        "acceptance_thresholds": None,
    }
    variables: dict[str, FloatArray] = {}
    dx, dy, dz = directions[:, 0], directions[:, 1], directions[:, 2]

    if metadata.case.angular == "fixed":
        difference = np.abs(
            directions - np.asarray(metadata.fixed_direction, dtype=np.float64)
        )
        mismatch_count = int(np.count_nonzero(np.any(difference != 0.0, axis=1)))
        maximum = float(np.max(difference))
        summary["exact_fixed"] = {
            "sample_count": metadata.primary_count,
            "mismatching_direction_count": mismatch_count,
            "maximum_component_absolute_difference": maximum,
        }
        if mismatch_count:
            raise ValueError(
                f"Fixed contract failure: {mismatch_count} mismatches; maximum component difference {maximum}."
            )

    elif metadata.case.angular in ("isotropic", "bounded-isotropic"):
        lower, upper, phi_min, phi_max = metadata.bounds
        valid_phi = (dx != 0.0) | (dy != 0.0)
        phi = np.arctan2(dy[valid_phi], dx[valid_phi])
        if metadata.case.angular == "isotropic":
            u_cos = (dz + 1.0) / 2.0
            phi = np.where(phi < 0.0, phi + math.tau, phi)
            u_phi = phi / math.tau
            # The full-sphere analytical azimuth law uses real 2*pi.
            phi_min, phi_max = 0.0, math.tau
        else:
            u_cos = (dz - lower) / (upper - lower)
            u_phi = (phi - phi_min) / (phi_max - phi_min)

        variables = {
            "u_cos": u_cos,
            "u_phi": u_phi,
            "u_cos_for_phi": u_cos[valid_phi],
        }
        summary["transformed_variables"] = {
            "u_cos": uniform_statistics(u_cos),
            "u_phi": uniform_statistics(u_phi),
        }
        paired_cos = u_cos[valid_phi]
        correlation = None
        if u_phi.size >= 2 and np.var(paired_cos) > 0.0 and np.var(u_phi) > 0.0:
            correlation = float(cast(np.float64, np.corrcoef(paired_cos, u_phi)[0, 1]))
        summary["correlations"] = {
            "variables": ["u_cos", "u_phi"],
            "sample_count": int(u_phi.size),
            "pearson": correlation,
        }
        excursion = max(0.0, -float(np.min(u_cos)), float(np.max(u_cos)) - 1.0)
        if u_phi.size:
            excursion = max(
                excursion, -float(np.min(u_phi)), float(np.max(u_phi)) - 1.0
            )
        summary["support"] = {
            "below_cos_lower_count": int(np.count_nonzero(dz < lower)),
            "above_cos_upper_count": int(np.count_nonzero(dz > upper)),
            "below_phi_min_count": int(np.count_nonzero(phi < phi_min)),
            "above_phi_max_count": int(np.count_nonzero(phi > phi_max)),
            "maximum_normalized_support_excursion": excursion,
        }
        summary["exclusions"] = {
            "phi_exclusion_count": int(np.count_nonzero(~valid_phi)),
            "north_pole_count": int(np.count_nonzero(~valid_phi & (dz > 0.0))),
            "south_pole_count": int(np.count_nonzero(~valid_phi & (dz < 0.0))),
        }

        if metadata.case.angular == "isotropic":
            summary["cartesian_moments"] = {
                "first": {
                    axis: float(np.mean(directions[:, index]))
                    for index, axis in enumerate(("x", "y", "z"))
                },
                "second": {
                    axis: float(np.mean(directions[:, index] ** 2))
                    for index, axis in enumerate(("x", "y", "z"))
                },
                "cross": {
                    "xy": float(np.mean(dx * dy)),
                    "xz": float(np.mean(dx * dz)),
                    "yz": float(np.mean(dy * dz)),
                },
                "ideal_first": 0.0,
                "ideal_second": 1.0 / 3.0,
                "ideal_cross": 0.0,
            }

    else:
        displacement = np.empty_like(directions)
        for axis in range(3):
            # Subtract Python integers before conversion, including at large origins.
            displacement[:, axis] = np.fromiter(
                (metadata.focus_pm[axis] - int(value) for value in positions[:, axis]),
                dtype=np.float64,
                count=metadata.primary_count,
            )
        distance = cast(FloatArray, np.linalg.norm(displacement, axis=1))
        if np.any(distance == 0.0):
            raise ValueError("An emission at the focus has no line-of-sight direction.")
        reference = displacement / distance[:, None]
        observed_unit = directions / norms[:, None]
        dot = np.sum(observed_unit * reference, axis=1)
        sine = np.linalg.norm(np.cross(observed_unit, reference), axis=1)
        angular_error = np.arctan2(sine, dot)
        miss_distance = (
            np.linalg.norm(np.cross(displacement, directions), axis=1) / norms
        )
        variables = {
            "angular_error_rad": angular_error,
            "miss_distance_pm": miss_distance,
        }
        summary["focused"] = {
            "minimum_unit_dot_reference": float(np.min(dot)),
            "mean_angular_error_rad": float(np.mean(angular_error)),
            "maximum_angular_error_rad": float(np.max(angular_error)),
            "mean_miss_distance_pm": float(np.mean(miss_distance)),
            "maximum_miss_distance_pm": float(np.max(miss_distance)),
            "median_miss_distance_pm": float(np.median(miss_distance)),
            "p95_miss_distance_pm": float(np.percentile(miss_distance, 95)),
            "nonforward_direction_count": int(np.count_nonzero(dot <= 0.0)),
            "unique_source_position_count": int(np.unique(positions, axis=0).shape[0]),
        }
        # Keep position support visible without adding a second Geometry campaign.
        width, height, _ = metadata.dimensions_pm
        summary["support"] = {
            "source_xy_outside_rectangle_count": sum(
                2 * abs(int(px)) > width or 2 * abs(int(py)) > height
                for px, py in zip(positions[:, 0], positions[:, 1], strict=True)
            ),
            "nonzero_source_z_count": 0,
        }

    return summary, variables


def analyze_case(
    samples_path: Path,
    metadata_path: Path,
    output_dir: Path,
    *,
    make_figures: bool = True,
) -> JsonObject:
    metadata = load_metadata(metadata_path)
    positions, directions = load_samples(samples_path, metadata)
    summary, variables = measure_angle(positions, directions, metadata)
    output_dir.mkdir(parents=True, exist_ok=True)
    summary["figures"] = []
    summary["figure_status"] = "not_requested"

    if metadata.case.angular == "fixed":
        summary["figure_status"] = "not_required"
    elif make_figures:
        from plot import plot_angle  # pyright: ignore[reportImplicitRelativeImport]

        summary["figures"] = [
            str(path)
            for path in plot_angle(
                metadata.case,
                positions,
                metadata.dimensions_pm,
                variables,
                cast(JsonObject, summary["exclusions"]),
                output_dir,
            )
        ]
        summary["figure_status"] = "generated"

    _ = (output_dir / "summary.json").write_text(
        json.dumps(summary, indent=2, allow_nan=False) + "\n", encoding="utf-8"
    )
    print(
        f"{metadata.case.name}: {metadata.primary_count} Source records; structure complete; overflow=0"
    )
    print(f"  norm: {summary['norm']}")
    for name, stats in cast(
        dict[str, JsonObject], summary["transformed_variables"]
    ).items():
        print(
            f"  {name}: n={stats['sample_count']}, mean={stats['mean']}, variance={stats['variance']}, D={stats['ecdf_max_deviation']}"
        )
    if summary["exact_fixed"] is not None:
        print(f"  exact Fixed: {summary['exact_fixed']}")
    if summary["focused"] is not None:
        print(f"  Focused: {summary['focused']}")
    print(f"  support: {summary['support']}; exclusions: {summary['exclusions']}")
    print(f"  summary: {output_dir / 'summary.json'}")
    return summary


class Arguments(Protocol):
    samples: Path
    metadata: Path
    output_dir: Path
    no_plots: bool


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Measure canonical A1 laws from production Source records."
    )
    _ = parser.add_argument("--samples", type=Path, required=True)
    _ = parser.add_argument("--metadata", type=Path, required=True)
    _ = parser.add_argument("--output-dir", type=Path, required=True)
    _ = parser.add_argument("--no-plots", action="store_true")
    args = cast(Arguments, cast(object, parser.parse_args()))
    _ = analyze_case(
        args.samples, args.metadata, args.output_dir, make_figures=not args.no_plots
    )


if __name__ == "__main__":
    main()
