from __future__ import annotations

import json
from collections.abc import Callable
from os import PathLike
from pathlib import Path
from typing import NotRequired, Protocol, TypedDict, cast

from .ggems import materials as _native_module


class _IsotopeDefinition(TypedDict):
    mass_number: int
    fraction: float
    isomer_state: NotRequired[int]


class _ElementDefinition(TypedDict):
    mass_fraction: float
    isotopes: NotRequired[list[_IsotopeDefinition]]


type _ElementValue = float | _ElementDefinition
type _Elements = dict[str, _ElementValue]


class _MaterialDefinition(TypedDict):
    name: str
    density: float
    elements: _Elements
    density_unit: NotRequired[str]


class _MaterialLibrary(TypedDict):
    materials: list[_MaterialDefinition]


class _NativeMaterials(Protocol):
    add: Callable[[str, float, _Elements, str], None]
    verbose: Callable[[str], None]
    available: Callable[[], None]
    registered: Callable[[], None]


def _as_object(value: object) -> object:
    return value


_native = cast(_NativeMaterials, _as_object(_native_module))

add = _native.add
verbose = _native.verbose
available = _native.available
registered = _native.registered


def load_json(path: str | PathLike[str]) -> None:
    """Load custom GGEMS materials from a JSON file.

    A JSON material library contains a ``materials`` array. Each entry defines
    one custom material.

    Elements specified directly by a number use their default isotopic
    composition. An element can instead provide ``mass_fraction`` and an
    ``isotopes`` list. Isotope fractions are atom fractions. Isotopes are
    identified numerically by ``mass_number`` and optional ``isomer_state``;
    isotope names are never parsed.

    Loading materials makes them available but does not register them.
    Registration occurs when a material is actually used by GGEMS.

    Args:
        path: Path to the material-library JSON file.

    Example:
        A single file can contain one or more materials::

            {
              "materials": [
                {
                  "name": "CustomWater",
                  "density": 1.0,
                  "density_unit": "g/cm3",
                  "elements": {
                    "H": 0.111898,
                    "O": 0.888102
                  }
                },
                {
                  "name": "Deuterium",
                  "density": 0.000180,
                  "density_unit": "g/cm3",
                  "elements": {
                    "H": {
                      "mass_fraction": 1.0,
                      "isotopes": [
                        {
                          "mass_number": 2,
                          "fraction": 1.0
                        }
                      ]
                    }
                  }
                }
              ]
            }

        Load the complete library with::

            ggems.materials.load_json("materials.json")
    """
    with Path(path).open("r", encoding="utf-8") as stream:
        data = cast(_MaterialLibrary, json.load(stream))

    for material in data["materials"]:
        _native.add(
            material["name"],
            material["density"],
            material["elements"],
            material.get("density_unit", "g/cm3"),
        )
