from dataclasses import dataclass
from typing import Literal

type Geometry = Literal[
    "point", "rectangle", "ellipse", "circle", "box", "sphere", "cylinder"
]


@dataclass(frozen=True, slots=True)
class GeometryCase:
    geometry: Geometry
    dimensions_mm: tuple[float, float, float]

    @property
    def name(self) -> str:
        return f"G1_{self.geometry}"


# Complete widths/diameters/heights; these are development configurations.
CASES: tuple[GeometryCase, ...] = (
    GeometryCase("point", (0.0, 0.0, 0.0)),
    GeometryCase("rectangle", (40.0, 20.0, 0.0)),
    GeometryCase("ellipse", (40.0, 20.0, 0.0)),
    GeometryCase("circle", (30.0, 30.0, 0.0)),
    GeometryCase("box", (40.0, 20.0, 10.0)),
    GeometryCase("sphere", (30.0, 30.0, 30.0)),
    GeometryCase("cylinder", (30.0, 30.0, 40.0)),
)

# The current Run constructs an Observer dump even when no sink prints it.
# Keep first-use capture modest; article sample sizes remain undecided.
DEFAULT_PRIMARIES = 4096
DEFAULT_WORKERS = 4096
DEFAULT_SEED = 77777
