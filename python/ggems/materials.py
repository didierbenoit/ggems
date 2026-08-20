from __future__ import annotations

import json
from os import PathLike
from pathlib import Path
from typing import cast

from .ggems import materials as _native


def available() -> tuple[str, ...]:
    return cast(
        tuple[str, ...],
        _native.available(),  # pyright: ignore[reportUnknownMemberType]
    )


def registered() -> tuple[str, ...]:
    return cast(
        tuple[str, ...],
        _native.registered(),  # pyright: ignore[reportUnknownMemberType]
    )


add = _native.add
verbose = _native.verbose
describe = _native.describe
register = _native.register


def load_json(path: str | PathLike[str]) -> int:
    with Path(path).open("r", encoding="utf-8") as stream:
        raw = cast(object, json.load(stream))

    if not isinstance(raw, dict):
        raise ValueError("GGEMS Material JSON root must be an object.")

    data = cast(dict[str, object], raw)

    required = {"name", "density", "elements"}
    missing = required - data.keys()

    if missing:
        names = ", ".join(sorted(missing))
        raise ValueError(f"Missing GGEMS Material JSON field(s): {names}.")

    allowed = required | {"density_unit"}
    unknown = data.keys() - allowed

    if unknown:
        names = ", ".join(sorted(unknown))
        raise ValueError(f"Unknown GGEMS Material JSON field(s): {names}.")

    name = data["name"]
    density = data["density"]
    elements_raw = data["elements"]
    density_unit = data.get("density_unit", "g/cm3")

    if not isinstance(name, str):
        raise ValueError("GGEMS Material JSON 'name' must be a string.")

    if not isinstance(density, (int, float)) or isinstance(density, bool):
        raise ValueError("GGEMS Material JSON 'density' must be a number.")

    if not isinstance(density_unit, str):
        raise ValueError("GGEMS Material JSON 'density_unit' must be a string.")

    if not isinstance(elements_raw, dict):
        raise ValueError("GGEMS Material JSON 'elements' must be an object.")

    elements_data = cast(dict[object, object], elements_raw)
    elements: dict[str, float] = {}

    for symbol, mass_fraction in elements_data.items():
        if not isinstance(symbol, str):
            raise ValueError("GGEMS Material JSON element symbols must be strings.")

        if not isinstance(mass_fraction, (int, float)) or isinstance(
            mass_fraction, bool
        ):
            raise ValueError(
                f"GGEMS Material JSON mass fraction for '{symbol}' must be a number."
            )

        elements[symbol] = float(mass_fraction)

    return add(name, float(density), elements, density_unit)
