import ggems

ggems.set_detail_level(4)
ggems.start()

# ------------------------------------------------------------------------------

opencl = ggems.opencl.GGEMSOpenCL()
opencl.print_devices()

# opencl.set_worker_count(4096)
# opencl.set_worker_count(65_536)
# opencl.set_worker_count(4_194_304)
opencl.set_worker_count(1_048_576)
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
    # .set_primary_count(1_000_000)
    .set_primary_count(1_073_741_824)
    # .set_primary_count(1_000_000_000)
    # .set_primary_count(2_147_483_648)
    # .set_primary_count(50_000_000_000)
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
box.set_rotation(0.0, 0.0, 45.0, "deg")
box.set_material("Water")

# sphere = ggems.geometry.GGEMSSphere()
# sphere.set_radius(0.25, "m")
# sphere.set_position(0.0, 0.0, 0.0, "m")
# sphere.set_material("Water")

# tubs = ggems.geometry.GGEMSTubs()
# tubs.set_radii(0.20, 0.08, "m")
# tubs.set_length(0.40, "m")
# tubs.set_position(0.0, 0.0, 0.0, "m")
# tubs.set_rotation(0.0, 45.0, 0.0, "deg")
# tubs.set_material("Water")

# ellipsoid = ggems.geometry.GGEMSEllipsoid()
# ellipsoid.set_semi_axes(0.25, 0.15, 0.10, "m")
# ellipsoid.set_position(0.0, 0.0, 0.0, "m")
# ellipsoid.set_rotation(20.0, 35.0, 45.0, "deg")
# ellipsoid.set_material("Water")

# trd = ggems.geometry.GGEMSTrd()
# trd.set_negative_z_face(200.0, 140.0, "mm")
# trd.set_positive_z_face(100.0, 80.0, "mm")
# trd.set_length(300.0, "mm")
# trd.set_position(0.0, 0.0, 0.0, "mm")
# trd.set_rotation(0.0, 45.0, 0.0, "deg")
# trd.set_material("Water")

# trap = ggems.geometry.GGEMSTrap()
# trap.set_negative_z_face(140.0, 180.0, 120.0)
# trap.set_positive_z_face(140.0, 180.0, 120.0)
# trap.set_length(300.0, "mm")
# trap.set_inclination(15.0, 0.0, "deg")
# trap.set_position(0.0, 0.0, 0.0, "mm")
# trap.set_rotation(0.0, 30.0, 0.0, "deg")
# trap.set_material("Water")

# cons = ggems.geometry.GGEMSCons()
# cons.set_negative_z_radii(20.0, 100.0, "mm")
# cons.set_positive_z_radii(10.0, 60.0, "mm")
# cons.set_length(300.0, "mm")
# cons.set_position(0.0, 0.0, 0.0, "mm")
# cons.set_rotation(0.0, 45.0, 0.0, "deg")
# cons.set_material("Water")

# polyhedra = ggems.geometry.GGEMSPolyhedra()
# polyhedra.set_side_count(6)
# polyhedra.set_start_angle(30.0, "deg")
# polyhedra.set_z_profile(
#    [-150.0, 0.0, 150.0],
#    [20.0, 15.0, 20.0],
#    [100.0, 70.0, 100.0],
#    "mm",
# )
# polyhedra.set_position(0.0, 0.0, 0.0, "mm")
# polyhedra.set_rotation(0.0, 30.0, 0.0, "deg")
# polyhedra.set_material("Water")

world.add_volume(box)
# world.add_volume(sphere)
# world.add_volume(tubs)
# world.add_volume(ellipsoid)
# world.add_volume(trd)
# world.add_volume(trap)
# world.add_volume(cons)
# world.add_volume(polyhedra)

# ---------------------------------------------------------------------------

run = ggems.run.GGEMSRun()
run.set_random(random)
run.add_source(source_test)
run.set_world(world)

run.initialize()
run.run()

# ---------------------------------------------------------------------------

ggems.stop()
