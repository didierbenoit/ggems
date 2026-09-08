from dataclasses import dataclass

DEFAULT_INTEGRATED_PRIMARIES = 8192
DEFAULT_PAIR_PRIMARIES = 2048
DEFAULT_WORKERS = 64
DEFAULT_SEED = 20260908
PAIR_WORKERS = 1

CENTER_MM = (12, -7, 25)
FRAME_DIRECTION = (4, -4, -7)
FRAME_UP = (8, 1, 4)
BOUNDS_DEG = (20, 60, -45, 90)
ALTERNATE_BOUNDS_DEG = (10, 35, 20, 160)
REGULAR_CENTERS_KEV = (25, 35, 45, 55)
REGULAR_WIDTH_KEV = 10
REGULAR_WEIGHTS = (1, 1, 2, 4)
LINE_ENERGIES_KEV = (20, 40, 60, 80)
LINE_WEIGHTS = (1, 0, 1, 2)


@dataclass(frozen=True, slots=True)
class Configuration:
    geometry: str = "rectangle"
    dimensions_mm: tuple[int, int, int] = (40, 20, 0)
    angular: str = "bounded-isotropic"
    bounds_deg: tuple[int, int, int, int] | None = BOUNDS_DEG
    energy: str = "regular-spectrum"


@dataclass(frozen=True, slots=True)
class IntegrationCase:
    name: str
    reference: Configuration
    comparison: Configuration | None = None
    exact_fields: tuple[str, ...] = ()


CASES = (
    IntegrationCase("I1_integrated_oblique", Configuration()),
    IntegrationCase(
        "I1_pair_geometry_same_draw_count",
        Configuration(),
        Configuration(geometry="box", dimensions_mm=(40, 20, 10)),
        ("direction", "energy", "time"),
    ),
    IntegrationCase(
        "I1_pair_angle_same_draw_count",
        Configuration(),
        Configuration(bounds_deg=ALTERNATE_BOUNDS_DEG),
        ("position", "energy", "time"),
    ),
    IntegrationCase(
        "I1_pair_draw_owner_swap",
        Configuration(angular="fixed", bounds_deg=None),
        Configuration(geometry="point", dimensions_mm=(0, 0, 0)),
        ("energy", "time"),
    ),
    IntegrationCase(
        "I1_pair_energy_configuration_order",
        Configuration(),
        Configuration(energy="discrete-lines"),
        ("position", "direction", "time"),
    ),
)


def draw_budget(configuration: Configuration) -> dict[str, object]:
    position = int(configuration.geometry != "point")
    angular = int(configuration.angular == "bounded-isotropic")
    return {
        "countdriven_time_draws": 0,
        "position_uniform4_calls": position,
        "angular_uniform4_calls": angular,
        "energy_uint32_calls": 1,
        "philox_blocks_before_energy": position + angular,
        "philox_blocks_per_primary": position + angular + 1,
        "order": ["time", "position", "direction", "energy"],
        "lookup_and_pose_draws": 0,
        "scalar_philox": "One fresh block; returns X; discards Y/Z/W",
    }
