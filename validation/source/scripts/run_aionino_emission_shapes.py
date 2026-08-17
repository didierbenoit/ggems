import threading

import ggems


def _base_source(x_m: float, primary_count: int):
    return (
        ggems.source.GGEMSSource()
        .set_primary_count(primary_count)
        .set_particle("aionino")
        .set_position(x_m, 0.0, 0.0, "m")
        .set_direction(0.0, 0.0, 1.0)
        .set_angular_fixed()
    )


def main() -> None:
    ggems.logging.set_output_mode("gui")
    ggems.logging.start_output_runtime()

    try:
        opencl = ggems.opencl.GGEMSOpenCL()
        opencl.select_devices(["gpu"])
        opencl.initialize()

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(23456)
        observer = (
            ggems.observer.GGEMSTransportObserver()
            .set_capacity(4096)
            .capture_first_primaries(384)
        )

        point = _base_source(-0.45, 1).set_emission_point()
        rectangle = _base_source(-0.15, 384).set_emission_rectangle(120.0, 60.0, "mm")
        circle = _base_source(0.15, 384).set_emission_circle(80.0, "mm")
        ellipse = _base_source(0.45, 384).set_emission_ellipse(120.0, 60.0, "mm")

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.add_source(point)
        simulation.add_source(rectangle)
        simulation.add_source(circle)
        simulation.add_source(ellipse)
        simulation.set_observer(observer)
        simulation.set_worker_count(256)

        application = ggems.gui.GGEMSGuiApplication(
            "Aionino emission shapes", 1600, 900
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

        worker = threading.Thread(target=simulate, name="ggems-simulation")
        worker.start()
        application.run()
        worker.join()

        if failures:
            raise failures[0]
    finally:
        ggems.logging.stop_output_runtime()


if __name__ == "__main__":
    main()
