"""Validated file boundaries and exact finite-ticket distribution semantics."""

import csv
import hashlib
import json
import math
from bisect import bisect_right
from dataclasses import dataclass
from decimal import Decimal
from itertools import pairwise
from pathlib import Path
from typing import cast

type JsonObject = dict[str, object]


def object_value(value: object) -> JsonObject:
    if not isinstance(value, dict):
        raise TypeError("Expected an object with string keys.")
    keys = cast(dict[object, object], value)
    if not all(isinstance(key, str) for key in keys):
        raise TypeError("Expected an object with string keys.")
    return cast(JsonObject, value)


def array_value(value: object) -> list[object]:
    if not isinstance(value, list):
        raise TypeError("Expected an array.")
    return cast(list[object], value)


def string_value(value: object) -> str:
    if not isinstance(value, str):
        raise TypeError("Expected a string.")
    return value


def integer_value(value: object, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise ValueError(f"Expected an integer >= {minimum}.")
    return value


def decimal_value(value: object) -> Decimal:
    if isinstance(value, bool) or not isinstance(value, (str, int, float)):
        raise TypeError("Expected a decimal string or number.")
    result = Decimal(str(value))
    if not result.is_finite():
        raise ValueError("Non-finite scientific value.")
    return result


def load_json(path: Path) -> JsonObject:
    return object_value(cast(object, json.loads(path.read_text(encoding="utf-8"))))


def write_json(path: Path, data: object) -> None:
    _ = path.write_text(
        json.dumps(data, indent=2, allow_nan=False) + "\n", encoding="utf-8"
    )


def read_csv(path: Path, columns: tuple[str, ...]) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if reader.fieldnames != list(columns):
            raise ValueError(f"Unexpected CSV columns in {path}.")
        rows = list(reader)
    if any(
        set(row) != set(columns) or any(value is None for value in row.values())
        for row in rows
    ):
        raise ValueError(f"Incomplete CSV row in {path}.")
    return rows


@dataclass(frozen=True, slots=True)
class Spectrum:
    energy: tuple[float, ...]
    density: tuple[float, ...]
    cumulative_area: tuple[float, ...]
    area: float
    mean: float

    @classmethod
    def load(cls, path: Path) -> "Spectrum":
        rows = read_csv(
            path, ("energy_keV", "density_per_keV", "standard_uncertainty_per_keV")
        )
        energy = tuple(float(decimal_value(row["energy_keV"])) for row in rows)
        density = tuple(float(decimal_value(row["density_per_keV"])) for row in rows)
        if len(energy) < 2 or energy[0] < 0 or any(b <= a for a, b in pairwise(energy)):
            raise ValueError(
                "Spectrum requires at least two strictly increasing nonnegative energies."
            )
        if any(value < 0 for value in density) or any(
            decimal_value(row["standard_uncertainty_per_keV"]) < 0 for row in rows
        ):
            raise ValueError("Negative reference density or uncertainty.")
        areas = [
            (b - a) * (c + d) / 2
            for a, b, c, d in zip(energy, energy[1:], density, density[1:])
        ]
        area = math.fsum(areas)
        if area <= 0:
            raise ValueError("Reference spectrum has zero area.")
        cumulative = [0.0]
        for value in areas:
            cumulative.append(cumulative[-1] + value)
        cumulative[-1] = area
        moment = math.fsum(
            (b - a) * (a * (2 * c + d) + b * (c + 2 * d)) / 6
            for a, b, c, d in zip(energy, energy[1:], density, density[1:])
        )
        return cls(energy, density, tuple(cumulative), area, moment / area)

    def cdf(self, energy: float) -> float:
        if energy <= self.energy[0]:
            return 0.0
        if energy >= self.energy[-1]:
            return 1.0
        index = bisect_right(self.energy, energy) - 1
        width = energy - self.energy[index]
        slope = (self.density[index + 1] - self.density[index]) / (
            self.energy[index + 1] - self.energy[index]
        )
        return (
            self.cumulative_area[index]
            + width * self.density[index]
            + width * width * slope / 2
        ) / self.area

    def pdf(self, energy: float) -> float:
        if energy < self.energy[0] or energy > self.energy[-1]:
            return 0.0
        index = min(len(self.energy) - 2, bisect_right(self.energy, energy) - 1)
        fraction = (energy - self.energy[index]) / (
            self.energy[index + 1] - self.energy[index]
        )
        return (
            self.density[index] * (1 - fraction) + self.density[index + 1] * fraction
        ) / self.area


@dataclass(frozen=True, slots=True)
class ReferenceGroup:
    index: int
    identifier: str
    particle: str
    yield_per_decay: Decimal
    kind: str
    distribution: JsonObject
    spectrum: Spectrum | None


@dataclass(frozen=True, slots=True)
class Reference:
    name: str
    half_life: Decimal
    groups: tuple[ReferenceGroup, ...]
    raw: JsonObject

    @classmethod
    def load(cls, path: Path) -> "Reference":
        raw = load_json(path)
        if raw["schema_version"] != 1:
            raise ValueError("Unsupported reference schema.")
        half_life = object_value(raw["half_life"])
        if half_life["unit"] != "s" or decimal_value(half_life["value"]) <= 0:
            raise ValueError("Reference half-life must be positive seconds.")
        groups: list[ReferenceGroup] = []
        for index, value in enumerate(array_value(raw["groups"])):
            group = object_value(value)
            distribution = object_value(group["distribution"])
            kind = string_value(distribution["kind"])
            if integer_value(group["index"]) != index or kind not in (
                "Mono",
                "DiscreteLines",
                "RegularSpectrum",
            ):
                raise ValueError("Invalid ordered group index or distribution kind.")
            yield_value = decimal_value(object_value(group["yield_per_decay"])["value"])
            if yield_value <= 0:
                raise ValueError(
                    "Emission yield must be positive; it is not normalized."
                )
            spectrum = None
            if kind == "RegularSpectrum":
                if (
                    distribution["reference_representation"]
                    != "piecewise_linear_density"
                    or distribution["energy_unit"] != "keV"
                ):
                    raise ValueError("Unsupported continuous reference representation.")
                spectrum = Spectrum.load(
                    path.parent / string_value(distribution["spectrum_file"])
                )
            groups.append(
                ReferenceGroup(
                    index,
                    string_value(group["id"]),
                    string_value(group["particle"]),
                    yield_value,
                    kind,
                    distribution,
                    spectrum,
                )
            )
        if not groups or len({group.identifier for group in groups}) != len(groups):
            raise ValueError("Reference requires distinct nonempty emission groups.")
        return cls(
            string_value(raw["name"]),
            decimal_value(half_life["value"]),
            tuple(groups),
            raw,
        )

    def verify_raw_files(self, reference_path: Path) -> int:
        files = array_value(object_value(self.raw["provenance"])["raw_files"])
        for value in files:
            entry = object_value(value)
            path = reference_path.parent.parent / "raw" / string_value(entry["path"])
            data = path.read_bytes()
            if (
                len(data) != entry["bytes"]
                or hashlib.sha256(data).hexdigest() != entry["sha256"]
            ):
                raise ValueError(f"Reference evidence differs from manifest: {path}")
        return len(files)


@dataclass(frozen=True, slots=True)
class RuntimeGroup:
    index: int
    particle: str
    particle_type: int
    yield_per_decay: Decimal
    kind: str
    mono: int
    width: int
    energies: tuple[int, ...]
    weights: tuple[Decimal, ...]
    upper_tickets: tuple[int, ...]
    ticket_space: int

    @property
    def lower_edge(self) -> int:
        return (
            self.energies[0] - self.width // 2
            if self.kind == "RegularSpectrum"
            else self.mono
        )

    @property
    def upper_edge(self) -> int:
        return self.lower_edge + len(self.energies) * self.width

    def ticket_cdf(self, energy: int) -> int:
        """Return the exact number of raw words producing E <= energy."""
        if self.kind == "Mono":
            return self.ticket_space if energy >= self.mono else 0
        if self.kind == "DiscreteLines":
            index = bisect_right(self.energies, energy) - 1
            return self.upper_tickets[index] if index >= 0 else 0
        if energy < self.lower_edge:
            return 0
        if energy >= self.upper_edge:
            return self.ticket_space
        index = (energy - self.lower_edge) // self.width
        previous = self.upper_tickets[index - 1] if index else 0
        count = self.upper_tickets[index] - previous
        offset = energy - (self.lower_edge + index * self.width)
        # Invert floor(width*t/count): exact ceil((offset+1)*count/width).
        return previous + min(
            count, ((offset + 1) * count + self.width - 1) // self.width
        )

    def continuous_cdf(self, energy: float) -> float:
        """Ideal piecewise constant grid law before finite-ticket quantization."""
        if self.kind != "RegularSpectrum":
            return self.ticket_cdf(math.floor(energy)) / self.ticket_space
        if energy <= self.lower_edge:
            return 0.0
        if energy >= self.upper_edge:
            return 1.0
        index = min(
            len(self.energies) - 1, int((energy - self.lower_edge) // self.width)
        )
        previous = self.upper_tickets[index - 1] if index else 0
        fraction = (energy - self.lower_edge - index * self.width) / self.width
        return (
            previous + fraction * (self.upper_tickets[index] - previous)
        ) / self.ticket_space


@dataclass(frozen=True, slots=True)
class Runtime:
    name: str
    half_life: Decimal
    energy_scale: int
    time_scale: int
    time_max: int
    groups: tuple[RuntimeGroup, ...]
    raw: JsonObject

    @classmethod
    def load(cls, directory: Path) -> "Runtime":
        raw = load_json(directory / "definition.json")
        if raw["schema_version"] != 1:
            raise ValueError("Unsupported compiled definition schema.")
        space = integer_value(raw["ticket_space"], 1)
        groups: list[RuntimeGroup] = []
        for index, value in enumerate(array_value(raw["groups"])):
            item = object_value(value)
            if integer_value(item["index"]) != index:
                raise ValueError("Compiled group order is not contiguous.")
            kind = {1: "Mono", 2: "DiscreteLines", 3: "RegularSpectrum"}[
                integer_value(item["distribution_type"], 1)
            ]
            rows = read_csv(
                directory / string_value(item["table_file"]),
                ("energy_micro_eV", "relative_weight", "cumulative_ticket_upper"),
            )
            energies = tuple(int(row["energy_micro_eV"]) for row in rows)
            weights = tuple(decimal_value(row["relative_weight"]) for row in rows)
            tickets = tuple(int(row["cumulative_ticket_upper"]) for row in rows)
            width = integer_value(item["bin_width_micro_eV"])
            if (
                len(rows) != integer_value(item["table_count"])
                or any(x < 0 for x in energies)
                or any(x < 0 for x in weights)
            ):
                raise ValueError("Invalid compiled energy table.")
            if kind != "Mono" and (
                not rows
                or tickets[-1] != space
                or any(b < a for a, b in zip((0,) + tickets, tickets))
                or any(
                    (b > a) != (weight > 0)
                    for a, b, weight in zip((0,) + tickets, tickets, weights)
                )
            ):
                raise ValueError(
                    "Compiled ticket CDF must cover its raw-word space and give tickets exactly to positive-weight entries."
                )
            if any(b <= a for a, b in pairwise(energies)):
                raise ValueError("Compiled energies must be strictly increasing.")
            if kind == "RegularSpectrum" and (
                width <= 0
                or width % 2
                or energies[0] < width // 2
                or any(b - a != width for a, b in pairwise(energies))
            ):
                raise ValueError("Invalid regular compiled grid.")
            groups.append(
                RuntimeGroup(
                    index,
                    string_value(item["particle"]),
                    integer_value(item["particle_type"]),
                    decimal_value(item["yield_per_decay"]),
                    kind,
                    integer_value(item["mono_energy_micro_eV"]),
                    width,
                    energies,
                    weights,
                    tickets,
                    space,
                )
            )
        half_life = decimal_value(raw["half_life_seconds"])
        total_yield = sum((group.yield_per_decay for group in groups), Decimal(0))
        if (
            not groups
            or half_life <= 0
            or any(group.yield_per_decay <= 0 for group in groups)
        ):
            raise ValueError("Invalid compiled half-life or emission yield.")
        if abs(
            total_yield - decimal_value(raw["total_yield_per_decay"])
        ) > total_yield * Decimal("1e-14"):
            raise ValueError(
                "Compiled total yield differs from the sum of unnormalized group yields."
            )
        return cls(
            string_value(raw["name"]),
            half_life,
            integer_value(raw["energy_micro_eV_per_keV"], 1),
            integer_value(raw["time_ps_per_s"], 1),
            integer_value(raw["time_max_ps"], 1),
            tuple(groups),
            raw,
        )
