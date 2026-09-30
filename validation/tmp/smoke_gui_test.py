import ggems

ggems.set_detail_level(4)
ggems.start("gui")

try:
    # --------------------------------------------------------------------------

    opencl = ggems.opencl.GGEMSOpenCL()
    opencl.set_worker_count(256)
    opencl.select_devices("1")
    opencl.initialize()

    # --------------------------------------------------------------------------

    random = ggems.rndm.GGEMSRandom().set_engine("Philox").set_seed(7777777)

    # --------------------------------------------------------------------------

    am241 = ggems.radionuclide.load("Am-241")
    am241.verbose()

    # --------------------------------------------------------------------------

    source = (
        ggems.source.GGEMSSource()
        .set_emission_point()
        .set_position(0.0, 0.0, 0.0, "mm")
        .set_angular_isotropic()
        .set_primary_count(1000)
        .set_particle("gamma")
        .set_energy(59.5, "keV")
    )

    # --------------------------------------------------------------------------

    observer = (
        ggems.observer.GGEMSTransportObserver().enable().capture_first_primaries(64)
    )

    # --------------------------------------------------------------------------

    run = ggems.run.GGEMSRun()
    run.set_random(random)
    run.add_source(source)
    run.set_observer(observer)
    run.set_time(0.0, 1.0, 1.0, "s")
    run.initialize()
    run.run()

    # --------------------------------------------------------------------------

    gui = ggems.gui.GGEMSGuiApplication()
    gui.set_vulkan_device(1)
    gui.initialize()

    gui.submit_last_run_source_snapshot(run)
    gui.submit_particle_traces_from_observer(observer)

    gui.run()

finally:
    ggems.stop()
