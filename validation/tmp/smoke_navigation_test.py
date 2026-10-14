import ggems

ggems.set_detail_level(4)
ggems.start()

# ------------------------------------------------------------------------------

opencl = ggems.opencl.GGEMSOpenCL()
opencl.print_devices()

opencl.set_worker_count(65_536)
# opencl.set_worker_count(4_194_304)
opencl.select_devices("0")
opencl.initialize()

opencl.print_contexts()

# ------------------------------------------------------------------------------

random = ggems.rndm.GGEMSRandom().set_engine("Philox").set_seed(7_777_777)
random.verbose()

# ------------------------------------------------------------------------------

source_test = (
    ggems.source.GGEMSSource()
    .set_emission_point()
    .set_position(-500.0, 0.0, 0.0, "mm")
    .set_direction(1.0, 0.0, 0.0)
    .set_angular_isotropic(0.0, 2.5, 0.0, 360.0, "deg")
    # .set_angular_fixed()
    .set_primary_count(1_073_741_824)
    .set_particle("aionino")
    .set_energy(125.0, "keV")
)

source_test.verbose()

# ---------------------------------------------------------------------------

world = ggems.geometry.GGEMSWorld()
world.set_size(3.0, 3.0, 3.0, "m")
world.set_material("Air")

box = ggems.geometry.GGEMSBox()
box.set_size(0.2, 0.1, 0.1, "m")
box.set_position(0.0, 0.0, 0.0, "m")
box.set_rotation(0.0, 0.0, 0.0, "deg")
box.set_material("Water")

world.add_volume(box)

# ---------------------------------------------------------------------------

run = ggems.run.GGEMSRun()
run.set_random(random)
run.add_source(source_test)
run.set_world(world)

run.initialize()
run.run()

# ---------------------------------------------------------------------------

ggems.stop()
