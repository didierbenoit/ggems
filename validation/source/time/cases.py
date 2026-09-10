from dataclasses import dataclass
from typing import Literal

DEFAULT_PRIMARIES = 256
DEFAULT_WORKERS = 64
DEFAULT_SEED = 77777
MONO_ENERGY_MICRO_EV = 511000000000


@dataclass(frozen=True, slots=True)
class TimeCase:
    name: str
    chronology: Literal["static", "configured"]
    # Configuration triples are start, stop, step. Explicit window tables are
    # the canonical analytical fixtures, not a second chronology simulator.
    requested_ns: tuple[int, int, int] | None
    configured_ps: tuple[int, int, int] | None
    windows_ps: tuple[tuple[int, int], ...]
    reset_before_run: int | None = None


CASES = (
    TimeCase("T1_static", "static", None, None, ((0, 0),)),
    TimeCase(
        "T1_configured_windows",
        "configured",
        (10, 65, 20),
        (10000, 65000, 20000),
        ((10000, 30000), (30000, 50000), (50000, 65000)),
    ),
    TimeCase(
        "T1_reset",
        "configured",
        (10, 50, 20),
        (10000, 50000, 20000),
        ((10000, 30000), (30000, 50000), (10000, 30000)),
        reset_before_run=2,
    ),
)
