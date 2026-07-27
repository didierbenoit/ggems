import math
from pathlib import Path
import tempfile
import unittest

import ggems


class GGEMSSourceBindingsTest(unittest.TestCase):
    def test_configuration_methods_are_fluent(self) -> None:
        source = ggems.source.GGEMSSource()

        self.assertIs(source.set_emission_point(), source)
        self.assertIs(source.set_emission_rectangle(40.0, 20.0, "mm"), source)
        self.assertIs(source.set_emission_ellipse(10.0, 5.0, "mm"), source)
        self.assertIs(source.set_emission_circle(10.0, "mm"), source)
        self.assertIs(source.set_emission_box(40.0, 20.0, 10.0, "mm"), source)
        self.assertIs(source.set_emission_sphere(10.0, "mm"), source)
        self.assertIs(source.set_emission_cylinder(10.0, 20.0, "mm"), source)
        self.assertIs(source.set_angular_fixed(), source)
        self.assertIs(source.set_angular_isotropic(), source)
        self.assertIs(
            source.set_angular_isotropic(10.0, 60.0, -45.0, 45.0, "deg"),
            source,
        )
        self.assertIs(source.set_angular_focused(0.0, 0.0, 100.0, "mm"), source)
        self.assertIs(source.set_energy(100.0, "keV"), source)

        self.assertIs(
            source.set_discrete_energy_lines(
                [40.0, 80.0, 120.0], (1.0, 2.0, 1.0), "keV"
            ),
            source,
        )
        self.assertIs(
            source.set_regular_energy_spectrum(
                (20.0, 22.0, 24.0), [1.0, 2.0, 1.0], "keV"
            ),
            source,
        )

    def test_time_configuration_is_owned_by_run(self) -> None:
        source = ggems.source.GGEMSSource()

        self.assertFalse(hasattr(source, "set_time_window"))
        self.assertFalse(hasattr(source, "set_time"))

    def test_all_distance_units_are_accepted(self) -> None:
        for unit in ("pm", "nm", "um", "mm", "cm", "m"):
            with self.subTest(unit=unit):
                source = ggems.source.GGEMSSource()
                source.set_emission_rectangle(2.0, 1.0, unit)
                source.set_emission_ellipse(2.0, 1.0, unit)
                source.set_emission_circle(1.0, unit)
                source.set_emission_box(2.0, 1.0, 3.0, unit)
                source.set_emission_sphere(1.0, unit)
                source.set_emission_cylinder(1.0, 2.0, unit)
                source.set_angular_focused(0.0, 0.0, 2.0, unit)

    def test_invalid_dimensions_have_distinct_diagnostics(self) -> None:
        source = ggems.source.GGEMSSource()

        with self.assertRaisesRegex(ValueError, "strictly positive"):
            source.set_emission_circle(0.0, "mm")

        with self.assertRaisesRegex(ValueError, "strictly positive"):
            source.set_emission_rectangle(-1.0, 2.0, "mm")

        with self.assertRaisesRegex(ValueError, "finite"):
            source.set_emission_ellipse(math.nan, 1.0, "mm")

        with self.assertRaisesRegex(ValueError, "Unsupported"):
            source.set_emission_circle(1.0, "km")

        with self.assertRaisesRegex(ValueError, "too large"):
            source.set_emission_circle(math.ldexp(1.0, 64), "pm")

        with self.assertRaisesRegex(ValueError, "strictly positive"):
            source.set_emission_box(1.0, 0.0, 1.0, "mm")

        with self.assertRaisesRegex(ValueError, "strictly positive"):
            source.set_emission_sphere(0.0, "mm")

        with self.assertRaisesRegex(ValueError, "strictly positive"):
            source.set_emission_cylinder(1.0, -1.0, "mm")

    def test_bounded_isotropic_accepts_degrees_radians_and_negative_phi(self) -> None:
        source = ggems.source.GGEMSSource()

        self.assertIs(source.set_angular_isotropic(0, 90, -45, 45, "deg"), source)
        self.assertIs(
            source.set_angular_isotropic(0.0, math.pi / 2.0, -1.0, 1.0, "rad"),
            source,
        )
        self.assertIn("angular_distribution=Isotropic", repr(source))

        with self.assertRaisesRegex(ValueError, "finite"):
            source.set_angular_isotropic(0.0, math.nan, 0.0, 1.0, "rad")
        with self.assertRaisesRegex(ValueError, "Unsupported"):
            source.set_angular_isotropic(0.0, 90.0, 0.0, 180.0, "grad")
        with self.assertRaises(RuntimeError):
            source.set_angular_isotropic(90.0, 90.0, 0.0, 180.0, "deg")
        with self.assertRaises(RuntimeError):
            source.set_angular_isotropic(0.0, 90.0, 0.0, 361.0, "deg")

    def test_volume_repr_and_absent_cube_api(self) -> None:
        source = ggems.source.GGEMSSource().set_emission_box(2, 3, 4, "mm")
        self.assertIn("emission_geometry=Box", repr(source))
        self.assertFalse(hasattr(source, "set_emission_cube"))
        self.assertFalse(hasattr(source, "set_cube_emission"))

    def test_invalid_focus_is_reported(self) -> None:
        source = ggems.source.GGEMSSource()

        with self.assertRaisesRegex(RuntimeError, "must not target"):
            source.set_angular_focused(0.0, 0.0, 0.0, "pm")

    def test_energy_sequences_accept_lists_and_tuples(self) -> None:
        source = ggems.source.GGEMSSource()

        self.assertIs(
            source.set_discrete_energy_lines(
                [40.0, 80.0, 120.0], (1.0, 2.0, 1.0), "keV"
            ),
            source,
        )
        self.assertIn("Discrete lines", repr(source))

        self.assertIs(
            source.set_regular_energy_spectrum(
                (20.0, 22.0, 24.0), [1.0, 2.0, 1.0], "keV"
            ),
            source,
        )
        self.assertIn("Regular spectrum", repr(source))

        self.assertIs(source.set_energy(511.0, "keV"), source)
        self.assertIn("Mono", repr(source))

    def test_energy_sequence_errors_are_reported(self) -> None:
        source = ggems.source.GGEMSSource()

        with self.assertRaises(RuntimeError):
            source.set_discrete_energy_lines([40.0, 80.0], [1.0], "keV")

        with self.assertRaises(RuntimeError):
            source.set_discrete_energy_lines([80.0, 40.0], [1.0, 1.0], "keV")

        with self.assertRaises(RuntimeError):
            source.set_regular_energy_spectrum(
                [20.0, 22.0, 25.0], [1.0, 2.0, 1.0], "keV"
            )

        with self.assertRaises(TypeError):
            source.set_discrete_energy_lines([40.0, "not-a-number"], [1.0, 1.0], "keV")

        with self.assertRaises(RuntimeError):
            source.set_regular_energy_spectrum([20.0, 22.0], [1.0, 1.0], "invalid-unit")

        invalid_operations = (
            (
                "empty discrete table",
                RuntimeError,
                lambda: source.set_discrete_energy_lines([], [], "keV"),
            ),
            (
                "single discrete line",
                RuntimeError,
                lambda: source.set_discrete_energy_lines([40.0], [1.0], "keV"),
            ),
            (
                "zero discrete weight sum",
                RuntimeError,
                lambda: source.set_discrete_energy_lines(
                    [40.0, 80.0], [0.0, 0.0], "keV"
                ),
            ),
            (
                "regular length mismatch",
                RuntimeError,
                lambda: source.set_regular_energy_spectrum([20.0, 22.0], [1.0], "keV"),
            ),
            (
                "non-positive regular center",
                RuntimeError,
                lambda: source.set_regular_energy_spectrum(
                    [0.0, 2.0], [1.0, 1.0], "keV"
                ),
            ),
            (
                "non-finite regular weight",
                RuntimeError,
                lambda: source.set_regular_energy_spectrum(
                    [20.0, 22.0], [1.0, float("inf")], "keV"
                ),
            ),
            (
                "negative Mono energy",
                ValueError,
                lambda: source.set_energy(-1.0, "keV"),
            ),
            (
                "non-finite Mono energy",
                ValueError,
                lambda: source.set_energy(float("nan"), "keV"),
            ),
            (
                "overflowing Mono energy",
                ValueError,
                lambda: source.set_energy(float(2**64), "meV"),
            ),
        )

        for case, exception_type, operation in invalid_operations:
            with self.subTest(case=case):
                with self.assertRaises(exception_type):
                    operation()

        source.set_discrete_energy_lines([40.0, 80.0, 120.0], [1.0, 2.0, 1.0], "keV")
        previous_description = repr(source)

        with self.assertRaises(RuntimeError):
            source.set_discrete_energy_lines(
                [40.0, 40.0, 120.0], [1.0, 2.0, 1.0], "keV"
            )

        self.assertEqual(repr(source), previous_description)

    def test_regular_spectrum_loader_uses_pathlike_and_reports_line(self) -> None:
        source = ggems.source.GGEMSSource()

        with tempfile.TemporaryDirectory() as directory:
            filename = Path(directory) / "spectrum.dat"
            filename.write_text(
                "# center weight\n0.011 1.0\nnot-a-number 2.0\n",
                encoding="utf-8",
                newline="\n",
            )

            with self.assertRaisesRegex(RuntimeError, r"line 3:"):
                source.load_regular_energy_spectrum(filename, "MeV")

            filename.write_text(
                "0.011 1.0\n0.012 2.0\n",
                encoding="utf-8",
                newline="\n",
            )
            self.assertIs(source.load_regular_energy_spectrum(filename, "MeV"), source)


if __name__ == "__main__":
    unittest.main()
