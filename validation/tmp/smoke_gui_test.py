import ggems

ggems.set_detail_level(4)
ggems.start("term")
# ggems.start("gui")

try:
    # --------------------------------------------------------------------------
    opencl = ggems.opencl.GGEMSOpenCL()
    opencl.set_worker_count(1048576)
    opencl.select_devices("0")
    opencl.initialize()

    # --------------------------------------------------------------------------
    random = ggems.rndm.GGEMSRandom().set_engine("Philox").set_seed(7777777)

    # --------------------------------------------------------------------------
    source = (
        ggems.source.GGEMSSource()
        .set_particle("Aionino")
        .set_energy(511.0, "keV")
        .set_primary_count(1_000_000)
        .set_emission_cylinder(2.0 * 150.0, 500.0, "mm")
        .set_position(0.0, 0.0, 0.0, "mm")
        .set_angular_isotropic()
    )

    # --------------------------------------------------------------------------
    # observer = (
    #    ggems.observer.GGEMSTransportObserver().enable().capture_first_primaries(512)
    # )

    # --------------------------------------------------------------------------
    world = ggems.geometry.GGEMSWorld()
    world.set_size(1200, 1200, 1200, "mm")
    world.set_material("Air")

    gantry = ggems.geometry.GGEMSTubs()
    gantry.set_radii(350.0, 328.0, "mm")
    gantry.set_length(300.0, "mm")
    gantry.set_material("Air")

    block = ggems.geometry.GGEMSBox()
    block.set_size(20, 34, 290, "mm")
    block.set_position(338.5, 0, 0, "mm")
    block.set_material("Air")

    module = ggems.geometry.GGEMSBox()
    module.set_size(20, 33.38016, 32.5, "mm")
    module.set_material("Air")

    crystal = ggems.geometry.GGEMSBox()
    crystal.set_size(19.8, 4.0, 3.9, "mm")
    crystal.set_material("LSO")
    crystal.set_color(ggems.colors.red_ruby)

    module.linear_repeater(crystal, count=(1, 8, 8), pitch=(0.0, 4.15, 4.0), unit="mm")
    block.linear_repeater(module, count=8, pitch=36.5625, unit="mm", axis="Z")
    gantry.circular_repeater(block, count=56, axis="Z")

    # --------------------------------------------------------------------------
    phantom = ggems.geometry.GGEMSTubs()
    phantom.set_radii(150.0, 0.0, "mm")
    phantom.set_length(500.0, "mm")
    phantom.set_material("Water")
    phantom.set_color(ggems.colors.blue_babyblue)

    # --------------------------------------------------------------------------
    world.add_volume(phantom)
    world.add_volume(gantry)

    # --------------------------------------------------------------------------
    run = ggems.run.GGEMSRun()
    run.set_world(world)
    run.set_random(random)
    run.add_source(source)
    # run.set_observer(observer)
    # run.set_time(0.0, 1.0, 1.0, "s")
    run.initialize()
    run.run()

    # --------------------------------------------------------------------------

    # gui = ggems.gui.GGEMSGuiApplication()
    # gui.set_vulkan_device(0)
    # gui.initialize()

    # gui.submit_last_run_source_snapshot(run)
    # gui.submit_particle_traces_from_observer(observer)

    # gui.submit_run_geometry(run)
    # gui.run()

finally:
    ggems.stop()
