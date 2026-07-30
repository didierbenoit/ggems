from pathlib import Path

import ggems


PRIMARY_COUNT = 4_000_000_000
# PRIMARY_COUNT = 4_290_772_991
WORKER_COUNT = 4_194_304
CONE_HALF_ANGLE_DEG = 15.0
SOURCE_DISTANCE_CM = 100.0
SPECTRUM_PATH = (
    Path(__file__).resolve().parents[1] / "data" / "spectrum_120kVp_2mmAl.dat"
)


def main() -> None:
    if not SPECTRUM_PATH.is_file():
        raise FileNotFoundError(f"GGEMS source spectrum was not found: {SPECTRUM_PATH}")

    ggems.core.set_output_mode("term")
    ggems.core.start_output_runtime()

    try:
        opencl = ggems.opencl.GGEMSOpenCL()
        opencl.select_devices(["1"])
        opencl.initialise()

        random = ggems.rndm.GGEMSRandom().set_engine("philox").set_seed(120_015)

        # The diagnostic transport currently emits one Source record and one
        # Terminal record per captured primary. Keep some capacity margin.
        #        observer = (
        #            ggems.observer.GGEMSTransportObserver()
        #            .set_capacity(4096)
        #            .capture_first_primaries(10)
        #        )

        source = (
            ggems.source.GGEMSSource()
            .set_analytic()
            .set_primary_count(PRIMARY_COUNT)
            .set_particle("gamma")
            # Source placed 100 cm before the origin on global -Z.
            .set_position(0.0, 0.0, -SOURCE_DISTANCE_CM, "cm")
            # The cone central axis points towards the origin along global +Z.
            .set_direction(0.0, 0.0, 1.0)
            .set_emission_point()
            # Circular cone: theta in [0, 15 deg], complete azimuth.
            .set_angular_isotropic(
                0.0,
                CONE_HALF_ANGLE_DEG,
                0.0,
                360.0,
                "deg",
            )
            .load_regular_energy_spectrum(SPECTRUM_PATH, "MeV")
        )

        simulation = ggems.run.GGEMSRun()
        simulation.set_random(random)
        simulation.add_source(source)
        #        simulation.set_observer(observer)
        simulation.set_worker_count(WORKER_COUNT)
        simulation.initialise()
        simulation.run()
    finally:
        ggems.core.stop_output_runtime()


if __name__ == "__main__":
    main()
