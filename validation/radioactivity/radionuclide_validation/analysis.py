"""Keep reference agreement, population law, birth times and energy tests distinct."""

import math
from bisect import bisect_right
from collections import defaultdict
from collections.abc import Callable
from decimal import Decimal
from fractions import Fraction
from itertools import pairwise
from pathlib import Path
from typing import cast

from .model import (
    JsonObject,
    Reference,
    ReferenceGroup,
    Runtime,
    RuntimeGroup,
    Spectrum,
    load_json,
    read_csv,
    write_json,
)
from .numerics import (
    conditioned_time_cdf,
    decay_integral,
    dkw_test,
    poisson_dispersion,
    poisson_interval,
    poisson_shape,
)


def grid_comparison(group: RuntimeGroup, spectrum: Spectrum, scale: int) -> JsonObject:
    edges = [
        (group.lower_edge + index * group.width) / scale
        for index in range(len(group.energies) + 1)
    ]
    masses = [
        (b - a) / group.ticket_space
        for a, b in zip((0,) + group.upper_tickets, group.upper_tickets)
    ]
    reference_masses = [spectrum.cdf(b) - spectrum.cdf(a) for a, b in pairwise(edges)]
    weight_sum = sum(group.weights, Decimal(0))
    normalized_weights = [float(weight / weight_sum) for weight in group.weights]
    outside = spectrum.cdf(edges[0]) + 1 - spectrum.cdf(edges[-1])
    points = sorted(set(edges + list(spectrum.energy)))
    distances = [abs(group.continuous_cdf(x * scale) - spectrum.cdf(x)) for x in points]
    # Between each pair of knots, the CDF difference is quadratic. Include its
    # stationary point to measure the supremum, rather than only comparing nodes.
    for a, b in pairwise(points):
        slope = (spectrum.pdf(b) - spectrum.pdf(a)) / (b - a)
        grid_density = (
            group.continuous_cdf(b * scale) - group.continuous_cdf(a * scale)
        ) / (b - a)
        if slope != 0:
            stationary = a + (grid_density - spectrum.pdf(a)) / slope
            if a < stationary < b:
                distances.append(
                    abs(
                        group.continuous_cdf(stationary * scale)
                        - spectrum.cdf(stationary)
                    )
                )
    numerator = 0
    for index, upper in enumerate(group.upper_tickets):
        count = upper - (group.upper_tickets[index - 1] if index else 0)
        if count:
            floor_sum = (
                (group.width - 1) * (count - 1) + math.gcd(group.width, count) - 1
            ) // 2
            numerator += count * (group.lower_edge + index * group.width) + floor_sum
    finite_mean = Fraction(numerator, group.ticket_space * scale)
    return {
        "status": "comparison_only",
        "compiled_table_entries": len(group.energies),
        "compiled_relative_weight_sum": str(weight_sum),
        "max_normalized_weight_reference_bin_mass_difference": max(
            abs(a - b) for a, b in zip(normalized_weights, reference_masses)
        ),
        "max_finite_ticket_vs_normalized_weight_bin_mass_difference": max(
            abs(a - b) for a, b in zip(masses, normalized_weights)
        ),
        "compiled_lower_edge_micro_eV": group.lower_edge,
        "compiled_upper_edge_micro_eV_exclusive": group.upper_edge,
        "compiled_width_micro_eV": group.width,
        "reference_support_keV": [spectrum.energy[0], spectrum.energy[-1]],
        "reference_mass_outside_compiled_support": outside,
        "max_absolute_bin_mass_difference": max(
            abs(a - b) for a, b in zip(masses, reference_masses)
        ),
        "binned_total_variation": (
            math.fsum(abs(a - b) for a, b in zip(masses, reference_masses)) + outside
        )
        / 2,
        "continuous_grid_reference_Kolmogorov_distance": max(distances),
        "compiled_finite_ticket_mean_keV": float(finite_mean),
        "compiled_finite_ticket_mean_keV_exact_fraction": str(finite_mean),
        "reference_interpolated_mean_keV": spectrum.mean,
        "mean_difference_keV": float(finite_mean) - spectrum.mean,
        "interpretation": "Deterministic central-shape discrepancy, with no uncertainty covariance model or physics correction. Reference bin masses are not renormalized to the compiled support.",
    }


def reference_cdf(group: ReferenceGroup, scale: int) -> Callable[[int], float]:
    if group.spectrum is not None:
        spectrum = group.spectrum
        return lambda energy: spectrum.cdf(energy / scale)
    if group.kind == "Mono":
        quantity = cast(JsonObject, group.distribution["energy"])
        value = Decimal(str(quantity["value"])) * scale
        return lambda energy: float(Decimal(energy) >= value)
    lines = cast(list[JsonObject], group.distribution["lines"])
    energies = tuple(Decimal(str(line["energy_keV"])) * scale for line in lines)
    weights = tuple(Decimal(str(line["weight"])) for line in lines)
    exact = tuple(int(energy) for energy in energies)
    total = sum(weights, Decimal(0))
    cumulative = [0.0]
    running = Decimal(0)
    for weight in weights:
        running += weight
        cumulative.append(float(running / total))
    return lambda energy: cumulative[bisect_right(exact, energy)]


def definition_audit(
    reference: Reference, runtime: Runtime, source_tree: Path
) -> JsonObject:
    matches: dict[str, bool] = {
        "name": runtime.name == reference.name,
        "half_life": abs(runtime.half_life - reference.half_life)
        <= reference.half_life * Decimal("1e-14"),
        "ordered_group_count": len(runtime.groups) == len(reference.groups),
    }
    groups: list[JsonObject] = []
    for actual, expected in zip(runtime.groups, reference.groups):
        checks = {
            "index": actual.index == expected.index,
            "particle": actual.particle == expected.particle,
            "yield": abs(actual.yield_per_decay - expected.yield_per_decay)
            <= expected.yield_per_decay * Decimal("1e-14"),
            "distribution_kind": actual.kind == expected.kind,
        }
        result: JsonObject = {
            "index": actual.index,
            "reference_group": expected.identifier,
            "checks": checks,
            "compiled_yield": str(actual.yield_per_decay),
            "reference_yield": str(expected.yield_per_decay),
            "compiled_kind": actual.kind,
            "reference_kind": expected.kind,
        }
        if actual.kind == "RegularSpectrum" and expected.spectrum is not None:
            result["spectrum_comparison"] = grid_comparison(
                actual, expected.spectrum, runtime.energy_scale
            )
        elif actual.kind == expected.kind == "Mono":
            energy = cast(JsonObject, expected.distribution["energy"])
            checks["mono_energy"] = (
                Decimal(actual.mono)
                == Decimal(str(energy["value"])) * runtime.energy_scale
                and energy["unit"] == "keV"
            )
        elif actual.kind == expected.kind == "DiscreteLines":
            cdf = reference_cdf(expected, runtime.energy_scale)
            result["line_cdf_max_difference"] = max(
                abs(actual.ticket_cdf(e) / actual.ticket_space - cdf(e))
                for e in actual.energies
            )
        groups.append(result)
        matches[f"group_{actual.index}"] = all(checks.values())
    provenance: JsonObject = {
        "status": "unavailable",
        "reason": "The compiled definition exposes no generator/provenance field.",
    }
    audit_value = reference.raw.get("implementation_audit")
    if audit_value is not None:
        audit = cast(JsonObject, audit_value)
        path = source_tree / str(audit["source_file"])
        prefix = str(audit["generator_prefix"])
        lines = path.read_text(encoding="utf-8").splitlines()
        matching = [
            index for index, line in enumerate(lines) if line.startswith(prefix)
        ]
        if len(matching) == 1:
            index = matching[0]
            version = lines[index][len(prefix) :].split()[0]
            provenance = {
                "status": "match"
                if version == audit["selected_version"]
                else "discrepancy",
                "documented_version": version,
                "selected_version": audit["selected_version"],
                "source_file": audit["source_file"],
                "source_comment": "\n".join(lines[index : index + 6]),
                "limitation": "Source-comment audit, not a compiled provenance API.",
            }
    forbidden: list[JsonObject] = []
    for item in cast(list[JsonObject], reference.raw["excluded_source_emissions"]):
        if "particle" in item and "energy_keV" in item:
            energy = Decimal(str(item["energy_keV"])) * runtime.energy_scale
            found = [
                group.index
                for group in runtime.groups
                if group.particle == item["particle"]
                and (
                    (group.kind == "Mono" and group.mono == energy)
                    or (group.kind == "DiscreteLines" and energy in group.energies)
                )
            ]
            forbidden.append(
                {
                    "id": item["id"],
                    "unexpected_groups": found,
                    "status": "fail" if found else "pass",
                }
            )
    return {
        "nuclear_scalars_and_mapping": "match"
        if all(matches.values())
        else "discrepancy",
        "scalar_relative_representation_tolerance": "1e-14",
        "checks": matches,
        "compiled_half_life_seconds": str(runtime.half_life),
        "reference_half_life_seconds": str(reference.half_life),
        "groups": groups,
        "generator_provenance": provenance,
        "excluded_source_emissions": forbidden,
        "transport": "Not tested: no physical annihilation, navigation, stopping, dose or daughter-chain transport.",
    }


def analyze_campaign(
    directory: Path, reference: Reference, source_tree: Path
) -> JsonObject:
    settings = load_json(directory / "settings.json")
    run_dir = directory / "run"
    runtime = Runtime.load(run_dir)
    metadata = load_json(run_dir / "run.json")
    windows = cast(int, metadata["windows"])
    step = cast(int, metadata["step_ps"])
    replicas = cast(int, metadata["population_replicates"])
    group_count = len(runtime.groups)
    alpha = float(Decimal(str(settings["family_alpha"])))
    hypotheses = windows * (2 * group_count + 3) + 5 * group_count + 1
    individual_alpha = alpha / hypotheses
    counts: dict[tuple[int, int, int], int] = {}
    max_relative_integral_error = Decimal(0)
    means: dict[tuple[int, int], Decimal] = {}
    parent_means: dict[int, Decimal] = {}
    for window in range(windows):
        parent_means[window] = decay_integral(
            Decimal(str(metadata["activity_bq"])),
            runtime.half_life,
            cast(int, metadata["reference_time_ps"]),
            window * step,
            (window + 1) * step,
            runtime.time_scale,
        )
        for group in runtime.groups:
            means[window, group.index] = parent_means[window] * group.yield_per_decay
    for row in read_csv(run_dir / "populations.csv"):
        replicate, window, group = (
            int(row[key]) for key in ("replicate", "window", "group")
        )
        key = (replicate, window, group)
        counts[key] = int(row["observed_count"])
        for actual, expected in (
            (Decimal(str(row["expected_parent_decays"])), parent_means[window]),
            (Decimal(str(row["expected_emissions"])), means[window, group]),
        ):
            max_relative_integral_error = max(
                max_relative_integral_error,
                abs(actual - expected) / expected if expected else abs(actual),
            )

    # ----------------------------------------------------------------------------
    # Read generated births once; keep only the columns used by the analysis.

    times: dict[int, list[int]] = defaultdict(list)
    energies: dict[int, list[int]] = defaultdict(list)
    observed_group: dict[tuple[int, int], int] = defaultdict(int)
    for row in read_csv(run_dir / "samples.csv"):
        window, group = int(row["window"]), int(row["group"])
        times[window].append(int(row["time_ps"]))
        energies[group].append(int(row["energy_micro_eV"]))
        observed_group[window, group] += 1
    windows_metadata = cast(list[JsonObject], metadata["window_results"])
    sample_count = sum(len(values) for values in energies.values())

    # ----------------------------------------------------------------------------
    # Compare populations and conditioned birth-time distributions per window.

    window_results: list[JsonObject] = []
    for window in range(windows):
        total_mean = sum(
            (means[window, group.index] for group in runtime.groups), Decimal(0)
        )
        total_count = len(times[window])
        item = windows_metadata[window]
        groups: list[JsonObject] = []
        for group in runtime.groups:
            groups.append(
                {
                    "index": group.index,
                    "mean_decimal": str(means[window, group.index]),
                    "population": poisson_interval(
                        observed_group[window, group.index],
                        float(means[window, group.index]),
                        individual_alpha,
                    ),
                    "replicated_population_shape": poisson_shape(
                        [
                            counts[rep, window, group.index]
                            for rep in range(1, replicas + 1)
                        ],
                        float(means[window, group.index]),
                        individual_alpha,
                    ),
                }
            )
        scaled = float(
            Decimal(2).ln()
            * Decimal(step)
            / (Decimal(runtime.time_scale) * runtime.half_life)
        )

        def birth_cdf(time: int, w: int = window, x: float = scaled) -> float:
            return conditioned_time_cdf((time - w * step) / step, x)

        birth = dkw_test(
            times[window],
            birth_cdf,
            birth_cdf,
            individual_alpha,
            1e-5 + 1 / (1 << 24) + conditioned_time_cdf(1 / step, scaled),
        )
        window_results.append(
            {
                "index": window,
                "start_ps": window * step,
                "stop_ps": (window + 1) * step,
                "start_half_lives": float(
                    Decimal(window * step)
                    / (Decimal(runtime.time_scale) * runtime.half_life)
                ),
                "independent_parent_decays": str(parent_means[window]),
                "total_mean_decimal": str(total_mean),
                "total_population": poisson_interval(
                    total_count, float(total_mean), individual_alpha
                ),
                "groups": groups,
                "birth_times": birth,
                "all_observed_births_in_half_open_window": all(
                    window * step <= time < (window + 1) * step
                    for time in times[window]
                ),
                "scaled_decay_independent": scaled,
                "scaled_decay_packed": item["scaled_decay"],
            }
        )
    # ----------------------------------------------------------------------------
    # Compare sampled energies with the compiled law and the selected reference.

    energy_results: list[JsonObject] = []
    for actual in runtime.groups:
        observed = energies[actual.index]
        unreachable = sum(
            actual.ticket_cdf(energy) == actual.ticket_cdf(energy - 1)
            for energy in observed
        )

        def cdf(energy: int, g: RuntimeGroup = actual) -> float:
            return g.ticket_cdf(energy) / g.ticket_space

        def left(energy: int, g: RuntimeGroup = actual) -> float:
            return g.ticket_cdf(energy - 1) / g.ticket_space

        result: JsonObject = {
            "index": actual.index,
            "unreachable_observations": unreachable,
            "finite_ticket_support_status": "pass" if unreachable == 0 else "fail",
            "finite_ticket_runtime_shape": dkw_test(
                observed, cdf, left, individual_alpha
            ),
        }
        if actual.index < len(reference.groups):
            expected = reference.groups[actual.index]
            ref_cdf = reference_cdf(expected, runtime.energy_scale)

            def ref_left(
                energy: int,
                fn: Callable[[int], float] = ref_cdf,
                continuous: bool = expected.spectrum is not None,
            ) -> float:
                return fn(energy if continuous else energy - 1)

            result["selected_reference_shape"] = dkw_test(
                observed, ref_cdf, ref_left, individual_alpha
            )
            if expected.spectrum is not None:
                support = expected.spectrum.energy
                result["outside_reference_support"] = sum(
                    not support[0] <= energy / runtime.energy_scale < support[-1]
                    for energy in observed
                )
                result["reference_support_status"] = (
                    "pass" if result["outside_reference_support"] == 0 else "fail"
                )
        energy_results.append(result)
    dispersion = [
        {
            "index": group.index,
            "result": poisson_dispersion(
                [
                    (
                        counts[rep, window, group.index],
                        float(means[window, group.index]),
                    )
                    for rep in range(1, replicas + 1)
                    for window in range(windows)
                ],
                individual_alpha,
            ),
        }
        for group in runtime.groups
    ]
    total_expected = sum(means.values(), Decimal(0))
    result = {
        "name": runtime.name,
        "reference_id": reference.raw["reference_id"],
        "definition_reference": definition_audit(reference, runtime, source_tree),
        "integrated_decay_counts": {
            "status": "pass"
            if max_relative_integral_error <= Decimal("5e-14")
            else "fail",
            "maximum_relative_error": str(max_relative_integral_error),
            "relative_tolerance": "5e-14",
            "independent_precision_decimal_digits": 80,
            "compared_plans": len(counts),
            "meaning": "GGEMS expected parent/emission counts versus independent exponential integral, using the compiled half-life and rounded input activity.",
        },
        "statistical_policy": {
            "family_alpha": alpha,
            "bonferroni_hypotheses_upper_bound": hypotheses,
            "individual_alpha": individual_alpha,
            "minimum_samples_policy": "Empty samples and DKW comparisons with complete acceptance limit >= 1 are insufficient_samples; rare physical groups are never amplified to force a statistical pass.",
            "birth_cdf_numerical_budget": "Existing 1e-5 Source CDF contract + 2^-24 uniform step + one ps conditional CDF mass.",
            "parameter_fitting": "None; all null means and shapes are fixed before samples are read.",
            "population_replicates": replicas,
            "replicate_seed_rule": "seed + replicate, 1..population_replicates; separate host planner streams only",
            "cpu_gpu_population_dependence": "Same seed gives the same host plan on CPU and GPU. Do not pool those duplicate host populations as independent experiments.",
        },
        "windows": window_results,
        "poisson_replicate_dispersion": dispersion,
        "whole_horizon_population": poisson_interval(
            sample_count, float(total_expected), individual_alpha
        ),
        "energy_distributions": energy_results,
        "sample_count": sample_count,
        "bounds": {
            "observed_lower_bound_inclusion_and_stop_exclusion": "pass"
            if all(
                item["all_observed_births_in_half_open_window"]
                for item in window_results
            )
            else "fail",
            "deterministic_raw_zero_and_maximum_word_probes": "Separate GGEMSRadioactiveTimeSampling host/OpenCL tests; statistical samples alone do not establish endpoint reachability.",
        },
        "transport_scope": "Source births through the current production diagnostic transport path. No physical radioactive daughter transport, positron annihilation, stopping, dose or navigation validation.",
    }
    write_json(directory / "analysis.json", result)
    return result
