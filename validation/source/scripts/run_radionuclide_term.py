import ggems


ACTIVITY_MBQ = 100.0
REFERENCE_TIME_S = 0.0

ACQUISITION_TIME_US = 1000000.0
WORKER_COUNT = 4_194_304


def main() -> None:
    ggems.logging.set_output_mode("term")
    ggems.logging.start_output_runtime()

    try:
        opencl = ggems.opencl.GGEMSOpenCL()
        opencl.select_devices(["1"])
        opencl.initialize()

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(120_015)

        radionuclide = ggems.radionuclide("Lu-177")
        # print(f"Radionuclide: {radionuclide.name}")
        # print(f"Half-life: {radionuclide.half_life_seconds} s")
        # print(f"Emission count: {radionuclide.emission_count}")

        source = (
            ggems.Source()
            .set_analytic()
            .set_emission_point()
            .set_position(0.0, 0.0, 0.0, "cm")
            .set_angular_isotropic()
            .set_radionuclide(
                radionuclide,
                activity=ACTIVITY_MBQ,
                activity_unit="MBq",
                reference_time=REFERENCE_TIME_S,
                time_unit="s",
            )
        )

        print(source)

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.add_source(source)
        simulation.set_worker_count(WORKER_COUNT)

        # ActivityDriven sources need a real, non-empty acquisition window.
        simulation.set_time(
            0.0,
            ACQUISITION_TIME_US,
            ACQUISITION_TIME_US,
            "us",
        )

        simulation.initialize()
        simulation.run()

    finally:
        ggems.logging.stop_output_runtime()


if __name__ == "__main__":
    main()
