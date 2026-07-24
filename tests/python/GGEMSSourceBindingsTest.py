import math
import unittest

import ggems


class GGEMSSourceBindingsTest(unittest.TestCase):
    def test_configuration_methods_are_fluent(self) -> None:
        source = ggems.source.GGEMSSource()

        self.assertIs(source.set_emission_point(), source)
        self.assertIs(source.set_emission_rectangle(40.0, 20.0, "mm"), source)
        self.assertIs(source.set_emission_ellipse(10.0, 5.0, "mm"), source)
        self.assertIs(source.set_emission_circle(10.0, "mm"), source)
        self.assertIs(source.set_angular_fixed(), source)
        self.assertIs(source.set_angular_isotropic(), source)
        self.assertIs(source.set_angular_focused(0.0, 0.0, 100.0, "mm"), source)

    def test_all_distance_units_are_accepted(self) -> None:
        for unit in ("pm", "nm", "um", "mm", "cm", "m"):
            with self.subTest(unit=unit):
                source = ggems.source.GGEMSSource()
                source.set_emission_rectangle(2.0, 1.0, unit)
                source.set_emission_ellipse(2.0, 1.0, unit)
                source.set_emission_circle(1.0, unit)
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

    def test_invalid_focus_is_reported(self) -> None:
        source = ggems.source.GGEMSSource()

        with self.assertRaisesRegex(RuntimeError, "must not target"):
            source.set_angular_focused(0.0, 0.0, 0.0, "pm")


if __name__ == "__main__":
    unittest.main()
