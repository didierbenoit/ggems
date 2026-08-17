import threading

import ggems


def base_source(x_m: float, primary_count: int):
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

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(56_789)
        observer = (
            ggems.observer.GGEMSTransportObserver()
            .set_capacity(8192)
            .capture_first_primaries(1024)
        )

        box = base_source(-0.5, 1024).set_emission_box(180.0, 120.0, 80.0, "mm")
        sphere = base_source(0.0, 1024).set_emission_sphere(160.0, "mm")
        cylinder = base_source(0.5, 1024).set_emission_cylinder(160.0, 180.0, "mm")

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.add_source(box)
        simulation.add_source(sphere)
        simulation.add_source(cylinder)
        simulation.set_observer(observer)
        simulation.set_worker_count(256)

        application = ggems.gui.GGEMSGuiApplication(
            "Aionino volume emission shapes", 1600, 900
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
