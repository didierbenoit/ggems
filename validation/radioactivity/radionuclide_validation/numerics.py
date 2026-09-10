"""Independent decay integrals, adaptive sample sizing and distribution tests."""

import math
from bisect import bisect_right
from collections import Counter
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from decimal import ROUND_HALF_UP, Decimal, localcontext
from typing import Protocol, cast

# SciPy does not ship these stubs; the public scalar calls are typed below.
from scipy.stats import chi2, poisson  # pyright: ignore[reportMissingTypeStubs]

from .model import JsonObject, Runtime


class PoissonDistribution(Protocol):
    def cdf(self, k: int, mu: float) -> float: ...
    def sf(self, k: int, mu: float) -> float: ...
    def ppf(self, q: float, mu: float) -> float: ...


class ChiSquareDistribution(Protocol):
    def cdf(self, x: float, df: int) -> float: ...
    def sf(self, x: float, df: int) -> float: ...


POISSON = cast(PoissonDistribution, cast(object, poisson))
CHI_SQUARE = cast(ChiSquareDistribution, cast(object, chi2))
DEFAULT_HORIZON = Decimal(4)


def decay_integral(
    activity: Decimal,
    half_life: Decimal,
    reference_ps: int,
    start_ps: int,
    stop_ps: int,
    time_scale: int,
) -> Decimal:
    """Integrate A_ref exp(-ln(2)*(t-t_ref)/T) at 80 decimal digits."""
    if (
        activity < 0
        or half_life <= 0
        or reference_ps > start_ps
        or stop_ps < start_ps
        or time_scale <= 0
    ):
        raise ValueError("Invalid exponential integration interval or parameters.")
    if activity == 0 or start_ps == stop_ps:
        return Decimal(0)
    with localcontext() as context:
        context.prec = 80
        rate = Decimal(2).ln() / half_life
        width = Decimal(stop_ps - start_ps) / time_scale
        elapsed = Decimal(start_ps - reference_ps) / time_scale
        x = rate * width
        if x < Decimal("0.1"):
            # Integral divided by interval length: sum (-x)^k/(k+1)!.
            term = mass = Decimal(1)
            order = 1
            while True:
                term *= -x / (order + 1)
                updated = mass + term
                if updated == mass:
                    break
                mass = updated
                order += 1
        else:
            mass = (1 - (-x).exp()) / x
        return +(activity * (-rate * elapsed).exp() * width * mass)


@dataclass(frozen=True, slots=True)
class Design:
    step_ps: int
    windows: int
    activity_bq: Decimal
    capacity: int
    requested_horizon_half_lives: Decimal
    actual_horizon_half_lives: Decimal
    target_last_window: int
    horizon_policy: str


def design_campaign(
    runtime: Runtime,
    horizon: Decimal = DEFAULT_HORIZON,
    windows: int = 32,
    target_last_window: int = 1000,
) -> Design:
    if horizon <= 0 or windows <= 0 or target_last_window <= 0:
        raise ValueError("Horizon, windows and useful sample target must be positive.")
    total_yield = sum((group.yield_per_decay for group in runtime.groups), Decimal(0))
    if runtime.half_life <= 0 or total_yield <= 0:
        raise ValueError(
            "A campaign needs a positive half-life and total emission yield."
        )
    with localcontext() as context:
        context.prec = 80
        requested_ticks = runtime.half_life * horizon * runtime.time_scale
        # Long-lived nuclides cannot span four half-lives in uint64 picoseconds.
        bounded_ticks = min(requested_ticks, Decimal(runtime.time_max) * Decimal("0.9"))
        step = int((bounded_ticks / windows).to_integral_value(rounding=ROUND_HALF_UP))
        if step < 1 or step * windows > runtime.time_max:
            raise ValueError(
                "Requested windows are not representable in canonical Time."
            )
        late = decay_integral(
            Decimal(1),
            runtime.half_life,
            0,
            step * (windows - 1),
            step * windows,
            runtime.time_scale,
        )
        activity = Decimal(target_last_window) / (late * total_yield)
        first_mean = (
            activity
            * decay_integral(
                Decimal(1), runtime.half_life, 0, 0, step, runtime.time_scale
            )
            * total_yield
        )
        # Allocation margin only; statistical thresholds are specified separately.
        capacity = math.ceil(float(first_mean) + 12 * math.sqrt(float(first_mean)) + 64)
        if capacity > ((1 << 32) - 1) // 2:
            raise ValueError("Capture would exceed the existing Observer ABI capacity.")
        return Design(
            step,
            windows,
            +activity,
            capacity,
            horizon,
            Decimal(step * windows) / (runtime.half_life * runtime.time_scale),
            target_last_window,
            "nominal_half_life_scaled"
            if bounded_ticks == requested_ticks
            else "capped_at_90_percent_of_uint64_Time_range",
        )


def conditioned_time_cdf(relative: float, scaled_decay: float) -> float:
    if not 0 <= relative <= 1 or scaled_decay < 0:
        raise ValueError("Invalid conditioned-time coordinate.")
    if scaled_decay == 0:
        return relative
    return -math.expm1(-scaled_decay * relative) / -math.expm1(-scaled_decay)


def ecdf_distance(
    values: Sequence[int], cdf: Callable[[int], float], left_cdf: Callable[[int], float]
) -> float:
    """Handle ties and discrete reference jumps on both sides of every sample."""
    if not values:
        raise ValueError("An ECDF requires samples.")
    frequencies = Counter(values)
    previous = 0
    distance = 0.0
    for value, frequency in sorted(frequencies.items()):
        distance = max(
            distance,
            abs(previous / len(values) - left_cdf(value)),
            abs((previous + frequency) / len(values) - cdf(value)),
        )
        previous += frequency
    return distance


def dkw_test(
    values: Sequence[int],
    cdf: Callable[[int], float],
    left_cdf: Callable[[int], float],
    alpha: float,
    numeric_budget: float = 0.0,
) -> JsonObject:
    if not 0 < alpha < 1 or numeric_budget < 0:
        raise ValueError("Invalid statistical policy.")
    if not values:
        return {"status": "insufficient_samples", "n": 0, "alpha": alpha}
    distance = ecdf_distance(values, cdf, left_cdf)
    statistical_limit = math.sqrt(math.log(2 / alpha) / (2 * len(values)))
    return {
        "status": "pass" if distance <= statistical_limit + numeric_budget else "fail",
        "n": len(values),
        "distance": distance,
        "statistical_limit": statistical_limit,
        "numeric_budget": numeric_budget,
        "limit": statistical_limit + numeric_budget,
        "alpha": alpha,
        "test": "two-sided ECDF with DKW-Massart bound",
    }


def poisson_interval(observed: int, mean: float, alpha: float) -> JsonObject:
    if observed < 0 or mean < 0 or not math.isfinite(mean):
        raise ValueError("Invalid Poisson observation or mean.")
    lower = int(POISSON.ppf(alpha / 2, mean))
    upper = int(POISSON.ppf(1 - alpha / 2, mean))
    p_value = min(
        1.0, 2 * min(POISSON.cdf(observed, mean), POISSON.sf(observed - 1, mean))
    )
    return {
        "observed": observed,
        "expected": mean,
        "interval": [lower, upper],
        "alpha": alpha,
        "two_sided_tail_p": p_value,
        "status": "pass" if lower <= observed <= upper else "fail",
        "test": "exact equal-tail Poisson prediction interval",
    }


def poisson_shape(counts: Sequence[int], mean: float, alpha: float) -> JsonObject:
    return dkw_test(
        counts,
        lambda k: POISSON.cdf(k, mean),
        lambda k: POISSON.cdf(k - 1, mean),
        alpha,
    )


def poisson_dispersion(
    counts_and_means: Sequence[tuple[int, float]], alpha: float
) -> JsonObject:
    if not counts_and_means or min(mean for _, mean in counts_and_means) < 100:
        return {
            "status": "not_applicable",
            "reason": "The large-mean chi-square approximation requires every mean >= 100; retain exact interval/ECDF tests.",
        }
    statistic = math.fsum(
        (count - mean) ** 2 / mean for count, mean in counts_and_means
    )
    degrees = len(counts_and_means)
    p_value = min(
        1.0,
        2 * min(CHI_SQUARE.cdf(statistic, degrees), CHI_SQUARE.sf(statistic, degrees)),
    )
    return {
        "status": "pass" if p_value >= alpha else "fail",
        "statistic": statistic,
        "degrees_of_freedom": degrees,
        "statistic_per_degree": statistic / degrees,
        "two_sided_p": p_value,
        "alpha": alpha,
        "minimum_mean": min(mean for _, mean in counts_and_means),
        "test": "two-sided Pearson dispersion, independent Poisson means specified rather than fitted",
    }


def reference_line_cdf(
    energies: Sequence[int], weights: Sequence[Decimal], energy: int
) -> float:
    total = sum(weights, Decimal(0))
    return float(sum(weights[: bisect_right(energies, energy)], Decimal(0)) / total)
