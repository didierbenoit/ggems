import threading

import ggems


def point_source(x_m: float, primary_count: int):
    return (
        ggems.source.GGEMSSource()
        .set_primary_count(primary_count)
        .set_particle("aionino")
        .set_position(x_m, 0.0, 0.0, "m")
        .set_emission_point()
    )


def main() -> None:
    ggems.core.set_output_mode("gui")
    ggems.core.start_output_runtime()

    try:
        opencl = ggems.opencl.GGEMSOpenCL()
        opencl.select_devices(["gpu"])
        opencl.initialize()

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(67_890)
        observer = (
            ggems.observer.GGEMSTransportObserver()
            .set_capacity(8192)
            .capture_first_primaries(768)
        )

        full_sphere = (
            point_source(-0.5, 768).set_direction(1.0, 0.0, 0.0).set_angular_isotropic()
        )

        circular_theta_sector = (
            point_source(0.0, 768)
            .set_direction(0.0, 0.0, 1.0)
            .set_angular_isotropic(0.0, 25.0, 0.0, 360.0, "deg")
        )

        theta_phi_sector = (
            point_source(0.5, 768)
            .set_direction(0.6, 0.0, 0.8)
            .set_angular_isotropic(35.0, 70.0, -45.0, 45.0, "deg")
        )

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.add_source(full_sphere)
        simulation.add_source(circular_theta_sector)
        simulation.add_source(theta_phi_sector)
        simulation.set_observer(observer)
        simulation.set_worker_count(256)

        application = ggems.gui.GGEMSGuiApplication(
            "Aionino bounded isotropic domains", 1600, 900
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
        ggems.core.stop_output_runtime()


if __name__ == "__main__":
    main()
