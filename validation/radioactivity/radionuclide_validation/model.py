# *****************************************************************************
# * This file is part of GGEMS.                                               *
# *                                                                           *
# * SPDX-License-Identifier: GPL-3.0-or-later                                 *
# * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
# * Inserm.                                                                   *
# *                                                                           *
# * GGEMS is free software: you can redistribute it and/or modify             *
# * it under the terms of the GNU General Public License as published by      *
# * the Free Software Foundation, either version 3 of the License, or         *
# * (at your option) any later version.                                       *
# *                                                                           *
# * GGEMS is distributed in the hope that it will be useful,                  *
# * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
# * GNU General Public License for more details.                              *
# *                                                                           *
# * You should have received a copy of the GNU General Public License         *
# * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
# *****************************************************************************

# Authors:
#     Julien BERT <julien.bert@univ-brest.fr>
#     Didier BENOIT <didier.benoit@inserm.fr>

"""Reference tables and exact finite-ticket energy distributions."""

import csv
import json
import math
from bisect import bisect_right
from collections.abc import Iterator
from dataclasses import dataclass
from decimal import Decimal
from pathlib import Path

type JsonObject = dict[str, object]


def load_json(path: Path) -> JsonObject:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, data: object) -> None:
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8", newline="\n")


def read_csv(path: Path) -> Iterator[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        yield from csv.DictReader(stream)


@dataclass(frozen=True, slots=True)
class Spectrum:
    energy: tuple[float, ...]
    density: tuple[float, ...]
    cumulative_area: tuple[float, ...]
    area: float
    mean: float

    @classmethod
    def load(cls, path: Path) -> "Spectrum":
        energy: list[float] = []
        density: list[float] = []
        for row in read_csv(path):
            energy.append(float(row["energy_keV"]))
            density.append(float(row["density_per_keV"]))

        areas = [
            (b - a) * (c + d) / 2
            for a, b, c, d in zip(energy, energy[1:], density, density[1:])
        ]
        area = math.fsum(areas)
        cumulative = [0.0]
        for value in areas:
            cumulative.append(cumulative[-1] + value)

        cumulative[-1] = area
        moment = math.fsum(
            (b - a) * (a * (2 * c + d) + b * (c + 2 * d)) / 6
            for a, b, c, d in zip(energy, energy[1:], density, density[1:])
        )
        return cls(
            tuple(energy), tuple(density), tuple(cumulative), area, moment / area
        )

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

        if energy == self.energy[-1]:
            return self.density[-1] / self.area

        index = bisect_right(self.energy, energy) - 1
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
        half_life = raw["half_life"]
        groups: list[ReferenceGroup] = []
        for group in raw["groups"]:
            distribution = group["distribution"]
            kind = str(distribution["kind"])
            yield_value = group["yield_per_decay"]
            spectrum = (
                Spectrum.load(path.parent / str(distribution["spectrum_file"]))
                if kind == "RegularSpectrum"
                else None
            )
            groups.append(
                ReferenceGroup(
                    group["index"],
                    str(group["id"]),
                    str(group["particle"]),
                    Decimal(str(yield_value["value"])),
                    kind,
                    distribution,
                    spectrum,
                )
            )

        return cls(
            str(raw["name"]), Decimal(str(half_life["value"])), tuple(groups), raw
        )


@dataclass(frozen=True, slots=True)
class RuntimeGroup:
    index: int
    particle: str
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
        return previous + ((offset + 1) * count + self.width - 1) // self.width

    def continuous_cdf(self, energy: float) -> float:
        """Ideal piecewise constant grid law before finite-ticket quantization."""
        if energy <= self.lower_edge:
            return 0.0

        if energy >= self.upper_edge:
            return 1.0

        index = int((energy - self.lower_edge) // self.width)
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

    @classmethod
    def load(cls, directory: Path) -> "Runtime":
        raw = load_json(directory / "definition.json")
        space = raw["ticket_space"]
        groups: list[RuntimeGroup] = []
        for item in raw["groups"]:
            kind = {1: "Mono", 2: "DiscreteLines", 3: "RegularSpectrum"}[
                item["distribution_type"]
            ]
            energies: list[int] = []
            weights: list[Decimal] = []
            tickets: list[int] = []
            for row in read_csv(directory / str(item["table_file"])):
                energies.append(int(row["energy_micro_eV"]))
                weights.append(Decimal(row["relative_weight"]))
                tickets.append(int(row["cumulative_ticket_upper"]))
            groups.append(
                RuntimeGroup(
                    item["index"],
                    str(item["particle"]),
                    Decimal(str(item["yield_per_decay"])),
                    kind,
                    item["mono_energy_micro_eV"],
                    item["bin_width_micro_eV"],
                    tuple(energies),
                    tuple(weights),
                    tuple(tickets),
                    space,
                )
            )

        return cls(
            str(raw["name"]),
            Decimal(str(raw["half_life_seconds"])),
            raw["energy_micro_eV_per_keV"],
            raw["time_ps_per_s"],
            raw["time_max_ps"],
            tuple(groups),
        )
