import ggems

ggems.set_detail_level(4)
# ggems.start("term")
ggems.start("gui")  # GUI

try:
    # --------------------------------------------------------------------------
    opencl = ggems.opencl.GGEMSOpenCL()
    opencl.set_worker_count(1048576)
    opencl.select_devices("0")
    opencl.initialize()

    # --------------------------------------------------------------------------
    random = ggems.rndm.GGEMSRandom().set_engine("Philox").set_seed(7777777)

    # --------------------------------------------------------------------------
    cb_source = (
        ggems.source.GGEMSSource()
        .set_emission_point()
        # .set_position(0.0, 0.0, -150.0)
        # .set_direction(0.0, 0.0, 1.0)
        # .set_angular_isotropic(0.0, 2.5, 0.0, 360.0, "deg")
        .set_position(-500.0, 0.0, 0.0, "mm")
        .set_direction(1.0, 0.0, 0.0)
        .set_angular_isotropic(0.0, 20.0, 0.0, 360.0, "deg")
        .set_primary_count(1_000_000)
        .set_particle("aionino")
        .set_energy(125.0, "keV")
    )
    cb_source.verbose()

    # --------------------------------------------------------------------------
    observer = (
        ggems.observer.GGEMSTransportObserver().enable().capture_first_primaries(1024)
    )

    # --------------------------------------------------------------------------
    world = ggems.geometry.GGEMSWorld("Dosimetry World")
    world.set_size(1200, 1200, 2200, "mm")
    world.set_material("Air")

    # gantry = ggems.geometry.GGEMSTubs("PET Gantry")
    # gantry.set_radii(350.0, 328.0, "mm")
    # gantry.set_length(300.0, "mm")
    # gantry.set_material("Air")

    # block = ggems.geometry.GGEMSBox("PET Block")
    # block.set_size(20, 34, 290, "mm")
    # block.set_position(338.5, 0, 0, "mm")
    # block.set_material("Air")

    # module = ggems.geometry.GGEMSBox("PET Module")
    # module.set_size(20, 33.38016, 32.5, "mm")
    # module.set_material("Air")

    # crystal = ggems.geometry.GGEMSBox("PET Crystal")
    # crystal.set_size(19.8, 4.0, 3.9, "mm")
    # crystal.set_material("LSO")
    # crystal.set_color(ggems.colors.red_ruby)

    # module.linear_repeater(crystal, count=(1, 8, 8), pitch=(0.0, 4.15, 4.0), unit="mm")
    # block.linear_repeater(module, count=8, pitch=36.5625, unit="mm", axis="Z")
    # gantry.circular_repeater(block, count=56, axis="Z")

    # --------------------------------------------------------------------------
    # phantom = ggems.geometry.GGEMSTubs()
    # phantom.set_radii(150.0, 0.0, "mm")
    # phantom.set_length(500.0, "mm")
    # phantom.set_material("Water")
    # phantom.set_color(ggems.colors.blue_babyblue)

    # xcat = ggems.geometry.GGEMSVoxelizedVolume(
    #    "XCAT Phantom", "validation/tmp/xcat.mhd", "validation/tmp/xcat_materials.txt"
    # )
    # xcat.set_position(0.0, 0.0, 0.0, "mm")
    # xcat.set_rotation(0.0, 0.0, 0.0, "deg")
    # xcat.verbose()

    # --------------------------------------------------------------------------
    # mlc = ggems.geometry.GGEMSMesh("MLC", "validation/tmp/MyMlc.obj", unit="mm")
    # mlc.set_position(0.0, 0.0, 0.0, "mm")
    # mlc.set_rotation(0.0, 0.0, 0.0, "deg")
    # mlc.set_material("Tungsten")
    # mlc.set_color(ggems.colors.red_ruby)
    # mlc.verbose()

    collimator = ggems.geometry.GGEMSMesh(
        "Colli_SPECT", "validation/tmp/collimator_MELP.stl", unit="mm"
    )
    collimator.set_position(0.0, 0.0, 0.0, "mm")
    collimator.set_rotation(90.0, 0.0, 0.0, "deg")
    collimator.set_material("Tungsten")
    collimator.set_color(ggems.colors.red_ruby)
    collimator.verbose()

    # --------------------------------------------------------------------------
    # world.add_volume(phantom)
    # world.add_volume(xcat)
    # world.add_volume(gantry)
    # world.add_volume(mlc)
    world.add_volume(collimator)

    # --------------------------------------------------------------------------
    run = ggems.run.GGEMSRun()
    run.set_world(world)
    run.set_random(random)
    run.add_source(cb_source)
    run.set_observer(observer)  # GUI
    # run.set_time(0.0, 1.0, 1.0, "s")
    run.initialize()

    # Run 1
    run.run()

    # Move the A30 leaf by +5 mm along local X
    # mlc.set_component_translation("A30", 5.0, 0.0, 0.0, "mm")
    # Upload updated geometry to OpenCL
    # run.update_geometry()

    # Run 2: modified MLC geometry
    # run.run()

    # --------------------------------------------------------------------------

    gui = ggems.gui.GGEMSGuiApplication()
    gui.set_vulkan_device(0)
    gui.initialize()

    gui.submit_last_run_source_snapshot(run)
    gui.submit_particle_traces_from_observer(observer)

    gui.submit_run_geometry(run)
    gui.run()

finally:
    ggems.stop()
