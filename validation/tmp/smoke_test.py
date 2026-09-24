import ggems

ggems.set_detail_level(4)
ggems.start()

# ------------------------------------------------------------------------------

opencl = ggems.opencl.GGEMSOpenCL()
opencl.print_devices()

opencl.set_worker_count(256)
opencl.select_devices("2")
opencl.initialize()

opencl.print_contexts()

# ------------------------------------------------------------------------------

random = ggems.rndm.GGEMSRandom().set_engine("Philox").set_seed(7777777)
random.verbose()

# ------------------------------------------------------------------------------

ggems.radionuclide.available()

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

source_1.verbose()
source_2.verbose()
source_3.verbose()

# ---------------------------------------------------------------------------

ggems.materials.add(
    "Deuterium",
    0.00018,
    {
        "H": {
            "mass_fraction": 1.0,
            "isotopes": [
                {
                    "mass_number": 2,
                    "fraction": 1.0,
                }
            ],
        }
    },
    "g/cm3",
)

ggems.materials.add(
    "Concrete, Barite (TYPE BA)",
    3.350,
    {
        "H": 0.003585,
        "O": 0.311622,
        "Mg": 0.001195,
        "Al": 0.004183,
        "Si": 0.010457,
        "S": 0.107858,
        "Ca": 0.050194,
        "Fe": 0.047505,
        "Ba": 0.463400,
    },
    "g/cm3",
)

ggems.materials.load_json("validation/tmp/custom_material.json")

ggems.materials.available()
ggems.materials.registered()
ggems.materials.verbose("Copper")

# ------------------------------------------------------------------------------

ggems.cuts.verbose()
ggems.cuts.set_cut(gamma=5.0, electron=0.1, positron=2.5, proton=10.0, unit="mm")
ggems.cuts.verbose()

ggems.stop()
