from pathlib import Path
import threading

import ggems

SPECTRUM_PATH = (
    Path(__file__).resolve().parents[1] / "data" / "spectrum_120kVp_2mmAl.dat"
)


def base_source(x_m: float):
    return (
        ggems.source.GGEMSSource()
        .set_primary_count(64)
        .set_particle("aionino")
        .set_position(x_m, 0.0, 0.0, "m")
        .set_direction(0.0, 0.0, 1.0)
        .set_emission_point()
        .set_angular_fixed()
    )


def main() -> None:
    ggems.logging.set_output_mode("gui")
    ggems.logging.start_output_runtime()

    try:
        opencl = ggems.opencl.GGEMSOpenCL()
        opencl.select_devices(["gpu"])
        opencl.initialize()

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(45_678)

        observer = (
            ggems.observer.GGEMSTransportObserver()
            .set_capacity(512)
            .capture_first_primaries(32)
        )

        mono = base_source(-0.4).set_energy(100.0, "keV")

        discrete_lines = base_source(0.0).set_discrete_energy_lines(
            [40.0, 80.0, 120.0],
            [1.0, 2.0, 1.0],
            "keV",
        )

        regular_spectrum = base_source(0.4).load_regular_energy_spectrum(
            SPECTRUM_PATH, "MeV"
        )

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.add_source(mono)
        simulation.add_source(discrete_lines)
        simulation.add_source(regular_spectrum)
        simulation.set_observer(observer)
        simulation.set_worker_count(64)

        application = ggems.gui.GGEMSGuiApplication(
            "Aionino energy distributions",
            1600,
            900,
        )
        application.initialize()

        failures: list[BaseException] = []

        def simulate() -> None:
            try:
                simulation.initialize()
                simulation.run()
                application.submit_last_run_source_snapshot(simulation)
                application.submit_particle_traces_from_observer(observer)
            except BaseException as exception:
                failures.append(exception)

        worker = threading.Thread(
            target=simulate,
            name="ggems-simulation",
        )
        worker.start()
        application.run()
        worker.join()

        if failures:
            raise failures[0]
    finally:
        ggems.logging.stop_output_runtime()


if __name__ == "__main__":
    main()
