import threading

import ggems


def main() -> None:
    ggems.core.set_output_mode("gui")
    ggems.core.start_output_runtime()

    try:
        opencl = ggems.opencl.GGEMSOpenCL()
        opencl.select_devices(["gpu"])
        opencl.initialise()

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(34567)
        observer = (
            ggems.observer.GGEMSTransportObserver()
            .set_capacity(4096)
            .capture_first_primaries(768)
        )

        fixed = (
            ggems.source.GGEMSSource()
            .set_primary_count(1)
            .set_particle("aionino")
            .set_position(-0.45, 0.0, 0.0, "m")
            .set_emission_point()
            .set_direction(0.0, 0.0, 1.0)
            .set_angular_fixed()
        )

        isotropic = (
            ggems.source.GGEMSSource()
            .set_primary_count(768)
            .set_particle("aionino")
            .set_position(0.0, 0.0, 0.0, "m")
            .set_emission_point()
            .set_angular_isotropic()
        )

        focused = (
            ggems.source.GGEMSSource()
            .set_primary_count(384)
            .set_particle("aionino")
            .set_position(0.45, 0.0, 0.0, "m")
            .set_emission_rectangle(200.0, 120.0, "mm")
            .set_angular_focused(0.45, 0.0, 0.35, "m")
        )

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.add_source(fixed)
        simulation.add_source(isotropic)
        simulation.add_source(focused)
        simulation.set_observer(observer)
        simulation.set_worker_count(256)

        application = ggems.gui.GGEMSGuiApplication("Aionino angular modes", 1600, 900)
        application.initialise()

        failures: list[BaseException] = []

        def simulate() -> None:
            try:
                simulation.initialise()
                simulation.run()
                application.submit_last_run_source_snapshot(simulation)
                application.submit_particle_traces_from_observer(observer)
            except BaseException as exception:
                failures.append(exception)

        worker = threading.Thread(target=simulate, name="ggems-simulation")
        worker.start()
        application.run()
        worker.join()

        if failures:
            raise failures[0]
    finally:
        ggems.core.stop_output_runtime()


if __name__ == "__main__":
    main()
