from dataclasses import dataclass
from typing import Literal

type EnergyMode = Literal["mono", "discrete-lines", "regular-spectrum"]


@dataclass(frozen=True, slots=True)
class EnergyCase:
    name: str
    mode: EnergyMode
    values_kev: tuple[int, ...]
    # Explicit canonical E1 fixtures, checked against the executed metadata.
    # These are not a replacement Units converter.
    energies_micro_ev: tuple[int, ...]
    weights: tuple[int, ...] = ()
    bin_width_kev: int | None = None
    bin_width_micro_ev: int = 0


CASES: tuple[EnergyCase, ...] = (
    EnergyCase("E1_mono", "mono", (511,), (511_000_000_000,)),
    EnergyCase(
        "E1_discrete_lines",
        "discrete-lines",
        (20, 40, 60, 80),
        (20_000_000_000, 40_000_000_000, 60_000_000_000, 80_000_000_000),
        (1, 0, 1, 2),
    ),
    EnergyCase(
        "E1_regular_spectrum",
        "regular-spectrum",
        (25, 35, 45, 55),
        (25_000_000_000, 35_000_000_000, 45_000_000_000, 55_000_000_000),
        (1, 1, 2, 4),
        bin_width_kev=10,
        bin_width_micro_ev=10_000_000_000,
    ),
)

# Development inspection only; the current Run still builds its Observer dump.
DEFAULT_PRIMARIES = 4096
DEFAULT_WORKERS = 4096
DEFAULT_SEED = 77777
