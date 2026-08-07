import threading

import ggems


def _source(
    position: tuple[float, float, float], direction: tuple[float, float, float]
):
    return (
        ggems.source.GGEMSSource()
        .set_primary_count(1)
        .set_particle("aionino")
        .set_position(*position, "m")
        .set_direction(*direction)
        .set_emission_point()
        .set_angular_fixed()
    )


def main() -> None:
    ggems.core.set_output_mode("gui")
    ggems.core.start_output_runtime()

    try:
        opencl = ggems.opencl.GGEMSOpenCL()
        opencl.select_devices(["gpu"])
        opencl.initialize()

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(12345)
        observer = (
            ggems.observer.GGEMSTransportObserver()
            .set_capacity(64)
            .capture_first_primaries(1)
        )

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.set_observer(observer)
        simulation.set_worker_count(64)

        cases = (
            ((-1.5, 0.0, 0.0), (1.0, 0.0, 0.0)),
            ((1.5, 0.0, 0.0), (-1.0, 0.0, 0.0)),
            ((0.0, -1.5, 0.0), (0.0, 1.0, 0.0)),
            ((0.0, 1.5, 0.0), (0.0, -1.0, 0.0)),
            ((0.0, 0.0, -1.5), (0.0, 0.0, 1.0)),
            ((0.0, 0.0, 1.5), (0.0, 0.0, -1.0)),
            ((0.0, 0.0, 0.0), (1.0, 1.0, 1.0)),
        )

        for position, direction in cases:
            simulation.add_source(_source(position, direction))

        application = ggems.gui.GGEMSGuiApplication("Aionino source axes", 1600, 900)
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
