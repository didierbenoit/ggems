import math
import unittest

import ggems


class GGEMSRunBindingsTest(unittest.TestCase):
    def test_default_static_time_state(self) -> None:
        simulation = ggems.run.GGEMSRun()

        self.assertFalse(simulation.has_time_configuration())
        self.assertTrue(simulation.has_next_time_step())
        self.assertEqual(simulation.get_current_time("ps"), 0.0)
        self.assertEqual(simulation.get_current_time_window("ps"), (0.0, 0.0))

    def test_time_setters_are_non_fluent_and_reset_is_available(self) -> None:
        simulation = ggems.run.GGEMSRun()

        self.assertIsNone(simulation.set_time(2.0, 5.0, 1.0, "ns"))
        self.assertTrue(simulation.has_time_configuration())
        self.assertTrue(simulation.has_next_time_step())
        self.assertEqual(simulation.get_current_time("ns"), 2.0)
        self.assertEqual(simulation.get_current_time_window("ns"), (2.0, 3.0))

        self.assertIsNone(simulation.reset_time())
        self.assertEqual(simulation.get_current_time("ns"), 2.0)

    def test_time_units_round_trip_through_run_binding(self) -> None:
        factors_to_ps = {
            "ps": 1,
            "ns": 1_000,
            "us": 1_000_000,
            "µs": 1_000_000,
            "μs": 1_000_000,
            "ms": 1_000_000_000,
            "s": 1_000_000_000_000,
            "min": 60_000_000_000_000,
            "h": 3_600_000_000_000_000,
        }

        for unit, factor_to_ps in factors_to_ps.items():
            with self.subTest(unit=unit):
                simulation = ggems.run.GGEMSRun()
                simulation.set_time(1.0, 3.0, 1.0, unit)

                self.assertEqual(simulation.get_current_time(unit), 1.0)
                self.assertEqual(simulation.get_current_time_window(unit), (1.0, 2.0))
                self.assertEqual(simulation.get_current_time("ps"), float(factor_to_ps))

    def test_default_python_unit_is_second(self) -> None:
        simulation = ggems.run.GGEMSRun()
        simulation.set_time(1.0, 3.0, 1.0)

        self.assertEqual(simulation.get_current_time(), 1.0)
        self.assertEqual(simulation.get_current_time_window(), (1.0, 2.0))

    def test_invalid_python_time_values_and_units_are_rejected(self) -> None:
        for value in (math.nan, math.inf, -math.inf):
            with self.subTest(value=value):
                simulation = ggems.run.GGEMSRun()
                with self.assertRaisesRegex(ValueError, r"^Run time must be finite\.$"):
                    simulation.set_time(value, 2.0, 1.0, "ps")

        simulation = ggems.run.GGEMSRun()
        with self.assertRaisesRegex(ValueError, "positive or zero"):
            simulation.set_time(-1.0, 2.0, 1.0, "ps")
        with self.assertRaisesRegex(ValueError, r"^Run time is too large\.$"):
            simulation.set_time(float(2**64), float(2**64), 1.0, "ps")
        unsupported_unit_message = r"^Unsupported Run time unit 'fortnight'\.$"
        with self.assertRaisesRegex(ValueError, unsupported_unit_message):
            simulation.set_time(0.0, 2.0, 1.0, "fortnight")
        with self.assertRaisesRegex(ValueError, unsupported_unit_message):
            simulation.get_current_time("fortnight")
        with self.assertRaisesRegex(ValueError, unsupported_unit_message):
            simulation.get_current_time_window("fortnight")

    def test_invalid_schedule_semantics_are_rejected(self) -> None:
        invalid_schedules = (
            (0.0, 0.0, 1.0),
            (2.0, 1.0, 1.0),
            (0.0, 2.0, 0.0),
            (0.0, 2.0, 0.49),
        )

        for start, stop, step in invalid_schedules:
            with self.subTest(start=start, stop=stop, step=step):
                simulation = ggems.run.GGEMSRun()
                with self.assertRaises(RuntimeError):
                    simulation.set_time(start, stop, step, "ps")

    def test_configured_schedule_iterates_and_resets_after_initialise(self) -> None:
        opencl = ggems.opencl.GGEMSOpenCL()
        simulation = None
        random = None
        source = None

        try:
            opencl.select_devices(["gpu"])
            opencl.initialise()

            random = ggems.rndm.GGEMSRandom()
            random.set_engine("philox")
            random.set_seed(12_345)

            source = ggems.source.GGEMSSource()
            source.set_primary_count(1)
            source.set_emission_point()
            source.set_angular_fixed()

            simulation = ggems.run.GGEMSRun()
            simulation.set_random(random)
            simulation.set_source(source)
            simulation.set_worker_count(64)
            simulation.set_time(0.0, 3.0, 2.0, "ps")
            simulation.initialise()

            with self.assertRaises(RuntimeError):
                simulation.set_time(0.0, 4.0, 1.0, "ps")

            self.assertEqual(simulation.get_current_time_window("ps"), (0.0, 2.0))
            simulation.run()
            self.assertEqual(simulation.get_current_time("ps"), 2.0)
            self.assertEqual(simulation.get_current_time_window("ps"), (2.0, 3.0))
            simulation.run()
            self.assertEqual(simulation.get_current_time("ps"), 3.0)
            self.assertFalse(simulation.has_next_time_step())

            with self.assertRaises(RuntimeError):
                simulation.run()
            self.assertEqual(simulation.get_current_time("ps"), 3.0)

            self.assertIsNone(simulation.reset_time())
            self.assertEqual(simulation.get_current_time("ps"), 0.0)
            self.assertTrue(simulation.has_next_time_step())
            simulation.run()
            self.assertEqual(simulation.get_current_time("ps"), 2.0)
        finally:
            simulation = None
            source = None
            random = None
            opencl.clean()


if __name__ == "__main__":
    unittest.main()
