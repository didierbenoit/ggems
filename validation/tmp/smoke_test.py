import ggems

ggems.set_detail_level(4)
ggems.start()

# ------------------------------------------------------------------------------

opencl = ggems.opencl.GGEMSOpenCL()
opencl.print_devices()

opencl.select_devices("1")
opencl.initialize()

opencl.print_contexts()

# ------------------------------------------------------------------------------

random = ggems.rndm.GGEMSRandom().set_engine("Philox").set_seed(7777777)
random.verbose()

# ------------------------------------------------------------------------------

print(ggems.radionuclide.available())

am241 = ggems.radionuclide.load("Am-241")
am241.verbose()

# ------------------------------------------------------------------------------

source_1 = (
    ggems.source.GGEMSSource()
    .set_emission_point()
    .set_position(-100.0, 0.0, 0.0, "mm")
    .set_angular_isotropic()
    .set_primary_count(1000)
    .set_particle("gamma")
    .set_energy(511.0, "keV")
)

# ---------------------------------------------------------------------------

source_2 = (
    ggems.source.GGEMSSource()
    .set_emission_point()
    .set_position(0.0, 0.0, 0.0, "mm")
    .set_angular_fixed()
    .set_direction(0.0, 0.0, 1.0)
    .set_primary_count(1000)
    .set_particle("electron")
    .set_energy(250.0, "keV")
)

# ---------------------------------------------------------------------------

source_3 = (
    ggems.source.GGEMSSource()
    .set_emission_point()
    .set_position(100.0, 0.0, 0.0, "mm")
    .set_angular_isotropic()
    .set_radionuclide(am241, 10.0, "MBq")
)

# ---------------------------------------------------------------------------

print(source_1)
print(source_2)
print(source_3)

source_1.verbose()
source_2.verbose()
source_3.verbose()

ggems.stop()
