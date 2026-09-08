from dataclasses import dataclass
from typing import Literal

type FrameName = Literal["identity", "cyclic", "oblique"]
type Vector = tuple[int, int, int]

ORIGIN: Vector = (0, 0, 0)
CENTER_MM: Vector = (12, -7, 25)
FOCUS_MM: Vector = (75, -40, 140)
BOUNDS_DEG = (20, 60, -45, 90)
DEFAULT_EXACT_PRIMARIES = 256
DEFAULT_STATISTICAL_PRIMARIES = 1024
DEFAULT_WORKERS = 64
DEFAULT_SEED = 77777


def orientation_request(frame: FrameName) -> tuple[Vector, Vector] | None:
    """Return the public SetOrientation direction/up inputs, not packed axes."""
    if frame == "identity":
        return None
    if frame == "cyclic":
        return (1, 0, 0), (0, 0, 1)
    return (4, -4, -7), (8, 1, 4)


@dataclass(frozen=True, slots=True)
class FrameCase:
    name: str
    geometry: Literal["point", "rectangle", "box"]
    dimensions_mm: Vector
    frame: FrameName
    angular: Literal["fixed", "isotropic", "bounded-isotropic", "focused"] = "fixed"
    center_mm: Vector = CENTER_MM
    paired: bool = False
    statistical: bool = False


CASES = (
    FrameCase("G2_translated_point", "point", ORIGIN, "oblique"),
    FrameCase("G2_box_signed_permutation", "box", (24, 16, 10), "cyclic", paired=True),
    FrameCase(
        "G2_rectangle_oblique",
        "rectangle",
        (40, 20, 0),
        "oblique",
        statistical=True,
    ),
    FrameCase("A2_fixed_oblique", "point", ORIGIN, "oblique"),
    FrameCase(
        "A2_full_sphere_frame_invariant",
        "point",
        ORIGIN,
        "oblique",
        angular="isotropic",
        center_mm=ORIGIN,
        paired=True,
    ),
    FrameCase(
        "A2_bounded_signed_permutation",
        "point",
        ORIGIN,
        "cyclic",
        angular="bounded-isotropic",
        center_mm=ORIGIN,
        paired=True,
    ),
    FrameCase(
        "A2_bounded_oblique",
        "point",
        ORIGIN,
        "oblique",
        angular="bounded-isotropic",
        statistical=True,
    ),
    FrameCase(
        "A2_focused_transformed_rectangle",
        "rectangle",
        (40, 20, 0),
        "oblique",
        angular="focused",
        statistical=True,
    ),
)
