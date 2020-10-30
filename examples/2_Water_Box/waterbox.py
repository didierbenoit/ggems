# ************************************************************************
# * This file is part of GGEMS.                                          *
# *                                                                      *
# * GGEMS is free software: you can redistribute it and/or modify        *
# * it under the terms of the GNU General Public License as published by *
# * the Free Software Foundation, either version 3 of the License, or    *
# * (at your option) any later version.                                  *
# *                                                                      *
# * GGEMS is distributed in the hope that it will be useful,             *
# * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
# * GNU General Public License for more details.                         *
# *                                                                      *
# * You should have received a copy of the GNU General Public License    *
# * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
# *                                                                      *
# ************************************************************************

from ggems import *

# ------------------------------------------------------------------------------
# STEP 0: Level of verbosity during computation
GGEMSVerbosity(3)

# ------------------------------------------------------------------------------
# STEP 1: Choosing an OpenCL context
opencl_manager.set_context_index(2)

# ------------------------------------------------------------------------------
# STEP 2: Visualization


# ------------------------------------------------------------------------------
# STEP 3: Setting GGEMS materials
materials_database_manager.set_materials('data/materials.txt')

# ------------------------------------------------------------------------------
# STEP 4: Phantoms and systems
phantom = GGEMSVoxelizedPhantom('water_box')
phantom.set_voxelized_phantom('data/waterbox.mhd', 'data/range_waterbox.txt')
phantom.set_position(10.0, -20.0, 0.0, 'mm')
phantom.set_rotation(0.0, 0.0, 0.0, 'deg')
phantom.set_local_axis(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)

# ct_detector = GGEMSCTSystem('SOMATOM_Definition_EDGE')
# ct_detector.set_ct_type('curved')
# # ct_detector.set_number_of_modules(1, 46)
# ct_detector.set_number_of_modules(1, 3)
# ct_detector.set_number_of_detection_elements(64, 16)
# ct_detector.set_size_of_detection_elements(0.6, 0.6, 0.6, 'mm')
# ct_detector.set_material('GOS')
# ct_detector.set_source_detector_distance(1085.6, 'mm')
# ct_detector.set_source_isocenter_distance(595.0, 'mm')

# ------------------------------------------------------------------------------
# STEP 5: Physics
processes_manager.add_process('Compton', 'gamma', 'all')
processes_manager.add_process('Photoelectric', 'gamma', 'all')
processes_manager.add_process('Rayleigh', 'gamma', 'all')

# Optional options, the following are by default
processes_manager.set_cross_section_table_number_of_bins(220)
processes_manager.set_cross_section_table_energy_min(1.0, 'keV')
processes_manager.set_cross_section_table_energy_max(1.0, 'MeV')

# # ------------------------------------------------------------------------------
# # STEP 6: Cuts, by default but are 1 um
range_cuts_manager.set_cut('gamma', 1.0, 'mm', 'all')

# # ------------------------------------------------------------------------------
# # STEP 7: Source
point_source = GGEMSXRaySource('point_source')
point_source.set_source_particle_type('gamma')
point_source.set_number_of_particles(1)
point_source.set_position(-595.0, 0.0, 0.0, 'mm')
point_source.set_rotation(0.0, 0.0, 0.0, 'deg')
point_source.set_beam_aperture(0.0, 'deg')
point_source.set_focal_spot_size(0.0, 0.0, 0.0, 'mm')
point_source.set_monoenergy(60.0, 'keV')

# # ------------------------------------------------------------------------------
# # STEP 8: GGEMS simulation parameters
ggems_manager.set_seed(777) # Optional, if not set, the seed is automatically computed

ggems_manager.opencl_verbose(False)
ggems_manager.material_database_verbose(False)
ggems_manager.navigator_verbose(True)
ggems_manager.source_verbose(True)
ggems_manager.memory_verbose(True)
ggems_manager.process_verbose(True)
ggems_manager.range_cuts_verbose(True)
ggems_manager.random_verbose(True)
ggems_manager.kernel_verbose(True)
ggems_manager.tracking_verbose(True, 0) # Track a specific particle a utiliser avec le singleton de façon plus efficace!!!

# # Initializing the GGEMS simulation
ggems_manager.initialize()

# # Start GGEMS simulation
ggems_manager.run()

# ------------------------------------------------------------------------------
# STEP 10: Exit GGEMS safely
opencl_manager.clean()
exit()