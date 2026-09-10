"""Independent scientific, finite-ticket and external-file boundary regressions."""

import json
import math
import sys
import tempfile
import unittest
from decimal import Decimal
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[3]))

from validation.radioactivity.radionuclide_validation.analysis import (
    definition_audit,
    reference_cdf,
)
from validation.radioactivity.radionuclide_validation.model import (
    Reference,
    ReferenceGroup,
    Runtime,
    RuntimeGroup,
    Spectrum,
)
from validation.radioactivity.radionuclide_validation.numerics import (
    conditioned_time_cdf,
    decay_integral,
    design_campaign,
    dkw_test,
    ecdf_distance,
    poisson_dispersion,
    poisson_interval,
)


def mono_group(yield_value: str = "1") -> RuntimeGroup:
    return RuntimeGroup(
        0, "Gamma", 2, Decimal(yield_value), "Mono", 511000000000, 0, (), (), (), 2**32
    )


def synthetic_runtime(half_life: str, yields: tuple[str, ...] = ("1",)) -> Runtime:
    return Runtime(
        "Synthetic",
        Decimal(half_life),
        10**9,
        10**12,
        2**64 - 1,
        tuple(mono_group(value) for value in yields),
        {},
    )


class DecayTests(unittest.TestCase):
    def test_integral_exact_half_lives_and_additivity(self) -> None:
        full = decay_integral(Decimal(1), Decimal(100), 0, 0, 400, 1)
        expected = Decimal(100) * Decimal(15) / Decimal(16) / Decimal(2).ln()
        self.assertLess(abs(full - expected), Decimal("1e-24"))
        pieces = sum(
            (
                decay_integral(Decimal(1), Decimal(100), 0, start, start + 25, 1)
                for start in range(0, 400, 25)
            ),
            Decimal(0),
        )
        self.assertLess(abs(full - pieces), Decimal("1e-24"))

    def test_small_parameter_and_zero_width(self) -> None:
        value = decay_integral(Decimal(3), Decimal("1e60"), 0, 7, 8, 10**12)
        self.assertGreater(value, 0)
        self.assertLess(abs(value - Decimal("3e-12")), Decimal("1e-80"))
        self.assertEqual(decay_integral(Decimal(3), Decimal(10), 0, 7, 7, 1), 0)

    def test_reference_offset_is_not_lost(self) -> None:
        value = decay_integral(
            Decimal(3), Decimal(10), 10**16, 10**16 + 10, 10**16 + 20, 1
        )
        expected = decay_integral(Decimal("1.5"), Decimal(10), 0, 0, 10, 1)
        self.assertLess(abs(value - expected), Decimal("1e-24"))

    def test_activity_adapts_to_total_yield_without_normalizing_it(self) -> None:
        one = design_campaign(synthetic_runtime("100", ("1",)))
        three = design_campaign(synthetic_runtime("100", ("1", "2")))
        self.assertLess(abs(one.activity_bq - three.activity_bq * 3), Decimal("1e-22"))
        late = decay_integral(
            three.activity_bq,
            Decimal(100),
            0,
            31 * three.step_ps,
            32 * three.step_ps,
            10**12,
        )
        self.assertLess(abs(3 * late - 1000), Decimal("1e-22"))
        self.assertEqual(three.actual_horizon_half_lives, 4)

    def test_long_lived_nuclide_reports_generic_time_range_cap(self) -> None:
        design = design_campaign(synthetic_runtime("1e20"))
        self.assertLessEqual(design.step_ps * design.windows, 2**64 - 1)
        self.assertEqual(
            design.horizon_policy, "capped_at_90_percent_of_uint64_Time_range"
        )

    def test_chronology_rejects_sub_quantum_windows(self) -> None:
        with self.assertRaises(ValueError):
            _ = design_campaign(synthetic_runtime("1e-20"))

    def test_conditioned_cdf_normal_and_small_regimes(self) -> None:
        for scaled in (0.0, 1e-12, math.log(2) / 8, 20.0):
            self.assertEqual(conditioned_time_cdf(0, scaled), 0)
            self.assertEqual(conditioned_time_cdf(1, scaled), 1)
            points = [conditioned_time_cdf(k / 10, scaled) for k in range(11)]
            self.assertEqual(points, sorted(points))
        self.assertAlmostEqual(conditioned_time_cdf(0.5, 1e-12), 0.5, places=12)
        self.assertGreater(conditioned_time_cdf(0.5, 1), 0.5)


class TicketTests(unittest.TestCase):
    def test_zero_weight_regular_bins_have_no_reachable_energy(self) -> None:
        group = RuntimeGroup(
            0,
            "Electron",
            3,
            Decimal(1),
            "RegularSpectrum",
            0,
            10,
            (5, 15, 25),
            (Decimal(1), Decimal(0), Decimal(1)),
            (4, 4, 8),
            8,
        )
        self.assertEqual(group.ticket_cdf(9), 4)
        self.assertEqual(group.ticket_cdf(10), 4)
        self.assertEqual(group.ticket_cdf(19), 4)
        self.assertEqual(group.ticket_cdf(20), 5)
        self.assertEqual(group.continuous_cdf(15), 0.5)

    def test_loader_accepts_zero_weight_bins_but_rejects_missing_positive_tickets(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            metadata = {
                "schema_version": 1,
                "name": "Synthetic",
                "half_life_seconds": "100",
                "total_yield_per_decay": "2",
                "energy_micro_eV_per_keV": 10**9,
                "time_ps_per_s": 10**12,
                "time_max_ps": 2**64 - 1,
                "ticket_space": 8,
                "groups": [
                    {
                        "index": 0,
                        "particle": "Electron",
                        "particle_type": 3,
                        "yield_per_decay": "2",
                        "distribution_type": 3,
                        "mono_energy_micro_eV": 0,
                        "bin_width_micro_eV": 10,
                        "table_count": 3,
                        "table_file": "group_0.csv",
                    }
                ],
            }
            _ = (root / "definition.json").write_text(
                json.dumps(metadata), encoding="utf-8"
            )
            table = root / "group_0.csv"
            header = "energy_micro_eV,relative_weight,cumulative_ticket_upper\n"
            _ = table.write_text(header + "5,1,4\n15,0,4\n25,1,8\n", encoding="utf-8")
            runtime = Runtime.load(root)
            self.assertEqual(runtime.groups[0].yield_per_decay, Decimal(2))
            self.assertEqual(runtime.groups[0].ticket_cdf(19), 4)
            _ = table.write_text(header + "5,1,4\n15,1,4\n25,1,8\n", encoding="utf-8")
            with self.assertRaises(ValueError):
                _ = Runtime.load(root)

    def test_regular_cdf_exhaustively_inverts_finite_ticket_mapping(self) -> None:
        for width in (2, 8, 1000, 10**12):
            group = RuntimeGroup(
                0,
                "Electron",
                3,
                Decimal(1),
                "RegularSpectrum",
                0,
                width,
                (width // 2, 3 * width // 2),
                (Decimal(3), Decimal(5)),
                (3, 8),
                8,
            )
            outcomes = [width * t // 3 for t in range(3)] + [
                width + width * t // 5 for t in range(5)
            ]
            for value in sorted(
                set([-1, 0, 2 * width] + [e + d for e in outcomes for d in (-1, 0, 1)])
            ):
                self.assertEqual(
                    group.ticket_cdf(value), sum(e <= value for e in outcomes)
                )

    def test_mono_and_discrete_line_boundaries(self) -> None:
        mono = mono_group()
        self.assertEqual(mono.ticket_cdf(mono.mono - 1), 0)
        self.assertEqual(mono.ticket_cdf(mono.mono), 2**32)
        lines = RuntimeGroup(
            0,
            "Gamma",
            2,
            Decimal(2),
            "DiscreteLines",
            0,
            0,
            (10, 20),
            (Decimal(1), Decimal(3)),
            (2, 8),
            8,
        )
        self.assertEqual([lines.ticket_cdf(x) for x in (9, 10, 19, 20)], [0, 2, 2, 8])

    def test_discrete_ecdf_handles_ties_and_missing_reference_jumps(self) -> None:
        distance = ecdf_distance([0, 0, 2, 2], lambda x: (x + 1) / 3, lambda x: x / 3)
        self.assertAlmostEqual(distance, 1 / 6)
        result = dkw_test([], lambda _: 0, lambda _: 0, 0.01)
        self.assertEqual(result["status"], "insufficient_samples")

    def test_poisson_checks_do_not_normalize_observed_counts(self) -> None:
        self.assertEqual(poisson_interval(1000, 1000, 0.01)["status"], "pass")
        self.assertEqual(poisson_interval(1500, 1000, 0.01)["status"], "fail")
        self.assertEqual(poisson_interval(0, 0, 0.01)["status"], "pass")
        self.assertEqual(
            poisson_dispersion([(1000, 1000.0)] * 100, 0.01)["status"], "fail"
        )


class ReferenceTests(unittest.TestCase):
    def test_selected_o15_values_and_raw_evidence(self) -> None:
        path = (
            Path(__file__).resolve().parents[1] / "data/O-15/reference/reference.json"
        )
        reference = Reference.load(path)
        self.assertEqual(reference.half_life, Decimal("122.266"))
        self.assertEqual(reference.groups[0].yield_per_decay, Decimal("0.999001"))
        self.assertEqual(reference.verify_raw_files(path), 20)
        spectrum = reference.groups[0].spectrum
        self.assertIsNotNone(spectrum)
        if spectrum is None:
            self.fail("Missing selected spectrum.")
        self.assertEqual(len(spectrum.energy), 348)
        self.assertAlmostEqual(spectrum.area, 0.998991212775914, places=14)
        self.assertEqual(spectrum.cdf(0), 0)
        self.assertEqual(spectrum.cdf(1732.18), 1)

    def test_reference_pdf_integration_is_not_linear_cdf_interpolation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "spectrum.csv"
            _ = path.write_text(
                "energy_keV,density_per_keV,standard_uncertainty_per_keV\n0,0,0\n2,2,0\n",
                encoding="utf-8",
            )
            spectrum = Spectrum.load(path)
            self.assertEqual(spectrum.cdf(1), 0.25)
            self.assertAlmostEqual(spectrum.mean, 4 / 3)

    def test_generic_mono_and_lines_reference_paths(self) -> None:
        mono = ReferenceGroup(
            0,
            "mono",
            "Gamma",
            Decimal(2),
            "Mono",
            {"energy": {"value": "0.01", "unit": "keV"}},
            None,
        )
        self.assertEqual(reference_cdf(mono, 10**9)(10_000_000), 1)
        lines = ReferenceGroup(
            0,
            "lines",
            "Gamma",
            Decimal(3),
            "DiscreteLines",
            {
                "lines": [
                    {"energy_keV": "1", "weight": "1"},
                    {"energy_keV": "2", "weight": "3"},
                ]
            },
            None,
        )
        self.assertEqual(reference_cdf(lines, 10**9)(10**9), 0.25)

    def test_audit_detects_wrong_yield_without_correcting_runtime(self) -> None:
        expected = ReferenceGroup(
            0,
            "mono",
            "Gamma",
            Decimal(2),
            "Mono",
            {"energy": {"value": "511", "unit": "keV"}},
            None,
        )
        reference = Reference("Synthetic", Decimal(100), (expected,), {})
        runtime = synthetic_runtime("100")
        result = definition_audit(reference, runtime, Path("."))
        self.assertEqual(result["nuclear_scalars_and_mapping"], "discrepancy")
        self.assertEqual(runtime.groups[0].yield_per_decay, Decimal(1))

    def test_malformed_reference_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "reference.json"
            _ = path.write_text(json.dumps({"schema_version": 999}), encoding="utf-8")
            with self.assertRaises(ValueError):
                _ = Reference.load(path)


if __name__ == "__main__":
    _ = unittest.main()
