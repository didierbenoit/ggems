from __future__ import annotations

import json
from os import PathLike
from pathlib import Path

from . import ggems as _native

available = _native.materials.available
registered = _native.materials.registered
add = _native.materials.add
verbose = _native.materials.verbose
describe = _native.materials.describe
register = _native.materials.register


def load_json(path: str | PathLike[str]) -> int:
    with Path(path).open("r", encoding="utf-8") as stream:
        data = json.load(stream)

    if not isinstance(data, dict):
        raise ValueError("GGEMS Material JSON root must be an object.")

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

    return add(
        data["name"],
        data["density"],
        data["elements"],
        data.get("density_unit", "g/cm3"),
    )
