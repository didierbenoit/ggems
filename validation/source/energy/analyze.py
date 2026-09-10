import argparse
import csv
import json
import math
from collections import Counter
from dataclasses import dataclass
from fractions import Fraction
from itertools import pairwise
from pathlib import Path
from typing import Protocol, cast

# These entry points run directly; Python adds this directory to sys.path.
from cases import CASES, EnergyCase  # pyright: ignore[reportImplicitRelativeImport]

type JsonObject = dict[str, object]

TICKET_SPACE = 1 << 32
UINT64_MAX = (1 << 64) - 1
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
    case: EnergyCase
    primary_count: int
    energies_micro_ev: tuple[int, ...]
    ticket_counts: tuple[int, ...]
    bin_width_micro_ev: int
    display_unit_micro_ev: int
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
    result = float(value)
    if not math.isfinite(result):
        raise ValueError(f"{name} must be finite.")
    return result


def _integers(value: object, name: str) -> tuple[int, ...]:
    return tuple(_integer(item, name) for item in _list(value, name))


def _numbers(value: object, name: str) -> tuple[float, ...]:
    return tuple(_number(item, name) for item in _list(value, name))


def load_metadata(path: Path) -> Metadata:
    raw = _object(cast(object, json.loads(path.read_text(encoding="utf-8"))), str(path))
    case = next((item for item in CASES if item.name == raw.get("case_name")), None)
    if case is None:
        raise ValueError("Unknown canonical E1 case in metadata.")

    type_id, mode_name = {
        "mono": (1, "Mono"),
        "discrete-lines": (2, "Discrete lines"),
        "regular-spectrum": (3, "Regular spectrum"),
    }[case.mode]
    for key, expected in {
        "geometry": "point",
        "angular_configuration": "fixed",
        "angular_mode": "Fixed",
        "population_mode": "CountDriven",
        "rng_engine": "Philox",
        "particle": "Gamma",
        "chronology": "static",
        "energy_configuration": case.mode,
        "energy_mode": mode_name,
    }.items():
        if raw.get(key) != expected:
            raise ValueError(f"E1 metadata mismatch: {key} must be {expected}.")

    for key in ("time_ps", "source_index", "global_primary_begin"):
        if _integer(raw.get(key), key) != 0:
            raise ValueError(f"E1 metadata requires {key} == 0.")
    for key in ("source_center_pm", "dimensions_pm"):
        if _integers(raw.get(key), key) != (0, 0, 0):
            raise ValueError(f"E1 metadata requires {key} == [0, 0, 0].")
    if _numbers(raw.get("dimensions_mm"), "dimensions_mm") != (0, 0, 0):
        raise ValueError("Point dimensions must be zero.")
    if _numbers(raw.get("fixed_direction"), "fixed_direction") != (0, 0, 1):
        raise ValueError("E1 requires exact stored Fixed +Z.")
    axes = tuple(
        _numbers(axis, "frame axis") for axis in _list(raw.get("frame_axes"), "axes")
    )
    if axes != ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
        raise ValueError("E1 requires the identity frame.")
    if _number(raw.get("weight"), "weight") != 1:
        raise ValueError("E1 requires weight 1.")

    primary_count = _integer(
        raw.get("primary_count"), "primary_count", 1, (TICKET_SPACE - 1) // 2
    )
    _ = _integer(raw.get("worker_count"), "worker_count", 1, TICKET_SPACE - 1)
    _ = _integer(raw.get("seed"), "seed")
    selector = raw.get("device_selector")
    if not isinstance(selector, str) or not selector:
        raise ValueError("Metadata must contain a nonempty device_selector.")
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
        if _integer(observer.get(key), f"observer.{key}") != expected:
            raise ValueError(f"Incomplete Observer capture: {key} must be {expected}.")
    for key in ("capacity_per_device", "host_capacity"):
        _ = _integer(observer.get(key), key, 2 * primary_count, TICKET_SPACE - 1)

    energy = _object(raw.get("energy"), "energy")
    if energy.get("representation") != "uint64 micro-eV":
        raise ValueError("Energy authority must be canonical uint64 micro-eV.")
    if _integer(energy.get("distribution_type"), "distribution_type") != type_id:
        raise ValueError("Packed distribution type disagrees with the E1 case.")
    if _integer(energy.get("table_offset"), "table_offset") != 0:
        raise ValueError("This single-source exporter requires table offset zero.")
    if _integer(energy.get("ticket_space_size"), "ticket_space_size") != TICKET_SPACE:
        raise ValueError("Expected the complete raw uint32 ticket space.")

    values = _integers(energy.get("energy_values_micro_eV"), "energy_values_micro_eV")
    bounds = _integers(energy.get("cumulative_ticket_upper_bounds"), "ticket bounds")
    weights = _numbers(energy.get("relative_weights"), "relative_weights")
    table_count = _integer(
        energy.get("table_count"), "table_count", 0, TICKET_SPACE - 1
    )
    width = _integer(
        energy.get("regular_bin_width_micro_eV"), "regular_bin_width_micro_eV"
    )
    mono = _integer(energy.get("mono_energy_micro_eV"), "mono_energy_micro_eV")
    if _integer(raw.get("energy_micro_eV"), "energy_micro_eV") != mono:
        raise ValueError("Source-record energy disagrees with energy metadata.")
    if (
        table_count != len(values)
        or len(bounds) != table_count
        or len(weights) != table_count
    ):
        raise ValueError("Packed energy table counts disagree.")
    if width != case.bin_width_micro_ev:
        raise ValueError("Packed width disagrees with the canonical E1 width.")

    tickets: tuple[int, ...] = ()
    if case.mode == "mono":
        if table_count or mono != case.energies_micro_ev[0]:
            raise ValueError(
                "E1 Mono must be table-free and exactly 511000000000 micro-eV."
            )
        values = (mono,)
    else:
        if mono != 0 or values != case.energies_micro_ev or weights != case.weights:
            raise ValueError("Packed energies/weights disagree with the E1 table.")
        if any(left >= right for left, right in pairwise(values)):
            raise ValueError("Canonical energies must be strictly increasing.")
        if not bounds or bounds[-1] != TICKET_SPACE:
            raise ValueError("Final cumulative ticket bound must be 2^32.")
        tickets = tuple(
            upper - lower
            for lower, upper in zip((0, *bounds[:-1]), bounds, strict=True)
        )
        if any(count < 0 for count in tickets):
            raise ValueError("Cumulative ticket bounds must be nondecreasing.")
        # Check these binary-friendly fixtures against ACTUAL packed intervals.
        # This verifies the requested fractions; it does not allocate tickets.
        for count, weight in zip(tickets, case.weights, strict=True):
            if Fraction(count, TICKET_SPACE) != Fraction(weight, sum(case.weights)):
                raise ValueError(
                    "Packed ticket probability differs from the exact E1 fraction."
                )

    if case.mode == "regular-spectrum":
        if width == 0 or width % 2 or values[0] <= width // 2:
            raise ValueError(
                "Regular bins require even positive width and positive lower edges."
            )
        if any(right - left != width for left, right in pairwise(values)):
            raise ValueError("Regular centers must be separated by one full bin width.")
        if values[-1] + width // 2 > UINT64_MAX:
            raise ValueError("Regular upper edge exceeds uint64 storage.")

    requested = _object(raw.get("requested_energy"), "requested_energy")
    if requested.get("unit") != "keV" or energy.get("display_unit") != "keV":
        raise ValueError("E1 requested/display energies must use keV.")
    display_unit = _integer(
        energy.get("display_unit_micro_eV"), "display_unit_micro_eV", 1
    )
    if any(
        value != display_unit * display
        for value, display in zip(values, case.values_kev, strict=True)
    ):
        raise ValueError(
            "Central display conversion disagrees with canonical E1 energies."
        )
    expected_values = () if case.mode == "mono" else case.values_kev
    if _numbers(requested.get("values"), "requested values") != expected_values:
        raise ValueError("Requested energies disagree with the E1 case.")
    if _numbers(requested.get("relative_weights"), "requested weights") != case.weights:
        raise ValueError("Requested weights disagree with the E1 case.")
    if case.mode == "mono":
        if _number(requested.get("mono"), "requested mono") != case.values_kev[0]:
            raise ValueError("Requested Mono energy disagrees with E1.")
    elif requested.get("mono") is not None:
        raise ValueError("Mono request is inactive in a tabulated mode.")
    if case.bin_width_kev is not None:
        if _number(requested.get("bin_width"), "requested width") != case.bin_width_kev:
            raise ValueError("Requested regular bin width disagrees with E1.")
    elif requested.get("bin_width") is not None:
        raise ValueError("Bin width is inactive outside RegularSpectrum.")

    return Metadata(case, primary_count, values, tickets, width, display_unit, raw)


def _decimal(value: str, minimum: int, maximum: int) -> int:
    digits = value.removeprefix("-")
    if not digits or not digits.isascii() or not digits.isdecimal():
        raise ValueError(f"Expected an exact decimal integer, received {value!r}.")
    result = int(value)
    if not minimum <= result <= maximum:
        raise ValueError(f"Integer {value!r} is outside its serialized field range.")
    return result


def load_samples(path: Path, metadata: Metadata) -> list[int]:
    energies: list[int] = []
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
                slot = _decimal(row[0], 0, TICKET_SPACE - 1)
                source_local = _decimal(row[1], 0, UINT64_MAX)
                global_id = _decimal(row[2], 0, UINT64_MAX)
                if slot != 0 or source_local != local_id or global_id != local_id:
                    raise ValueError(
                        "Missing, duplicate, unordered, or impossible primary provenance."
                    )
                if row[12] != "Source":
                    raise ValueError("Only Source records are accepted.")

                position = tuple(
                    _decimal(value, -(1 << 63), (1 << 63) - 1) for value in row[3:6]
                )
                direction = tuple(float(value) for value in row[6:9])
                if position != (0, 0, 0) or direction != (0, 0, 1):
                    raise ValueError(
                        "E1 requires exact origin and finite Fixed +Z direction."
                    )
                if _decimal(row[10], 0, UINT64_MAX) != 0 or float(row[11]) != 1:
                    raise ValueError("E1 requires static time 0 ps and weight 1.")

                # No NumPy/float conversion of the sampled energy authority.
                energies.append(_decimal(row[9], 0, UINT64_MAX))
            except ValueError as error:
                raise ValueError(f"{path}, row {local_id + 2}: {error}") from error

    if len(energies) != metadata.primary_count:
        raise ValueError(
            f"Expected {metadata.primary_count} Source records, found {len(energies)}."
        )
    return energies


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


def measure_energy(energies: list[int], metadata: Metadata) -> JsonObject:
    count = len(energies)
    if count != metadata.primary_count:
        raise ValueError("Sample count differs from metadata.")
    multiplicities = Counter(energies)
    summary: JsonObject = {
        "case_name": metadata.case.name,
        "sample_count": count,
        "structural_validation": {
            "status": "complete",
            "observer_overflow_count": 0,
            "provenance_complete_unique_and_ordered": True,
            "source_records_only": True,
            "point_origin_fixed_plus_z_static_zero_time_unit_weight": True,
            "particle": "Gamma; exporter checks every raw record (no CSV particle column)",
        },
        "metadata": metadata.raw,
        "numerical_representation": {
            "input": "Exact decimal uint64 micro-eV, parsed as Python integers",
            "authority": "Immutable run snapshot energies, width, and cumulative uint32-ticket bounds",
            "analysis": "Integer support/CDF arithmetic; exact rational probability differences; binary64 display only",
            "regular_law": "Finite integer image of tickets, not a continuous Uniform test",
            "repetitions": "Expected on an integer lattice; not classified as defects",
        },
        "distinct_emitted_energy_count": len(multiplicities),
        "repeated_sample_count": count - len(multiplicities),
        "repeated_energy_count": sum(value > 1 for value in multiplicities.values()),
        "maximum_multiplicity": max(multiplicities.values()),
        "minimum_energy_micro_eV": min(energies),
        "maximum_energy_micro_eV": max(energies),
        "support": {},
        "acceptance_thresholds": None,
    }

    if metadata.case.mode == "mono":
        expected = metadata.energies_micro_ev[0]
        mismatches = sum(energy != expected for energy in energies)
        maximum = max(abs(energy - expected) for energy in energies)
        if mismatches:
            raise ValueError(
                f"Mono contract failure: {mismatches} mismatches; maximum difference {maximum} micro-eV."
            )
        summary["exact_mono"] = {
            "sample_count": count,
            "expected_energy_micro_eV": expected,
            "minimum_energy_micro_eV": min(energies),
            "maximum_energy_micro_eV": max(energies),
            "mismatching_energy_count": mismatches,
            "maximum_absolute_energy_difference_micro_eV": maximum,
        }
        summary["support"] = {
            "mismatching_energy_count": 0,
            "maximum_absolute_excursion_micro_eV": 0,
        }

    elif metadata.case.mode == "discrete-lines":
        off_line = set(energies).difference(metadata.energies_micro_ev)
        if off_line:
            raise ValueError(
                f"Off-line DiscreteLines energy: {min(off_line)} micro-eV."
            )
        counts = [multiplicities[value] for value in metadata.energies_micro_ev]
        if any(
            observed and ticket == 0
            for observed, ticket in zip(counts, metadata.ticket_counts, strict=True)
        ):
            raise ValueError("A zero-ticket DiscreteLines entry was emitted.")

        rows, metrics = probability_results(counts, metadata.ticket_counts, count)
        for row, energy in zip(rows, metadata.energies_micro_ev, strict=True):
            row.update(
                {
                    "energy_micro_eV": energy,
                    "energy_keV": energy / metadata.display_unit_micro_ev,
                }
            )
        summary["line_results"] = rows
        summary["probability_metrics"] = metrics
        summary["support"] = {
            "off_line_count": 0,
            "zero_ticket_emission_count": 0,
            "maximum_absolute_excursion_micro_eV": 0,
        }

    else:
        width = metadata.bin_width_micro_ev
        lower_edges = [center - width // 2 for center in metadata.energies_micro_ev]
        upper_edges = [lower + width for lower in lower_edges]
        offsets: list[list[int]] = [[] for _ in lower_edges]

        for energy in energies:
            memberships = [
                index
                for index, (lower, upper) in enumerate(
                    zip(lower_edges, upper_edges, strict=True)
                )
                if lower <= energy < upper
            ]
            if len(memberships) != 1:
                raise ValueError(
                    f"Energy {energy} micro-eV belongs to {len(memberships)} regular half-open bins."
                )
            index = memberships[0]
            offsets[index].append(energy - lower_edges[index])

        rows, metrics = probability_results(
            [len(values) for values in offsets], metadata.ticket_counts, count
        )
        finite: list[JsonObject] = []
        for index, (values, ticket_count) in enumerate(
            zip(offsets, metadata.ticket_counts, strict=True)
        ):
            rows[index].update(
                {
                    "center_energy_micro_eV": metadata.energies_micro_ev[index],
                    "lower_energy_micro_eV": lower_edges[index],
                    "upper_energy_micro_eV": upper_edges[index],
                }
            )
            finite.append(
                {
                    "bin_index": index,
                    "sample_count": len(values),
                    "distinct_emitted_energy_count": len(set(values)),
                    "minimum_offset_micro_eV": min(values) if values else None,
                    "maximum_offset_micro_eV": max(values) if values else None,
                    "exact_finite_cdf_max_deviation": finite_cdf_max_deviation(
                        values, ticket_count, width
                    ),
                }
            )
        summary["bin_results"] = rows
        summary["probability_metrics"] = metrics
        summary["finite_within_bin_results"] = finite
        summary["support"] = {
            "outside_all_bins_count": 0,
            "ambiguous_bin_count": 0,
            "unattainable_offset_count": 0,
            "maximum_absolute_excursion_micro_eV": 0,
        }

    return summary


def analyze_case(
    samples_path: Path,
    metadata_path: Path,
    output_dir: Path,
    *,
    make_figures: bool = True,
) -> JsonObject:
    metadata = load_metadata(metadata_path)
    energies = load_samples(samples_path, metadata)
    summary = measure_energy(energies, metadata)

    output_dir.mkdir(parents=True, exist_ok=True)
    summary["figures"] = []
    summary["figure_status"] = "not_requested"
    if metadata.case.mode == "mono":
        summary["figure_status"] = "not_required"
    elif make_figures:
        from plot import plot_energy  # pyright: ignore[reportImplicitRelativeImport]

        rows = cast(
            list[JsonObject],
            summary[
                "line_results"
                if metadata.case.mode == "discrete-lines"
                else "bin_results"
            ],
        )
        finite = cast(list[JsonObject], summary.get("finite_within_bin_results", []))
        summary["figures"] = [
            str(path)
            for path in plot_energy(
                metadata.case,
                energies,
                metadata.energies_micro_ev,
                metadata.bin_width_micro_ev,
                metadata.display_unit_micro_ev,
                rows,
                finite,
                output_dir,
            )
        ]
        summary["figure_status"] = "generated"

    _ = (output_dir / "summary.json").write_text(
        json.dumps(summary, indent=2, allow_nan=False) + "\n", encoding="utf-8"
    )
    print(
        f"{metadata.case.name}: {len(energies)} Source records; structure complete; overflow=0"
    )
    if metadata.case.mode == "mono":
        print(f"  exact Mono: {summary['exact_mono']}")
    else:
        print(f"  probabilities: {summary['probability_metrics']}")
    if "finite_within_bin_results" in summary:
        print(f"  finite within-bin: {summary['finite_within_bin_results']}")
    print(
        f"  figures: {summary['figure_status']}; summary: {output_dir / 'summary.json'}"
    )
    return summary


class Arguments(Protocol):
    samples: Path
    metadata: Path
    output_dir: Path
    no_plots: bool


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Measure exact canonical E1 energy laws from production Source records."
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
