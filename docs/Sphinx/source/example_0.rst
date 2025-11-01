*************************************
Examples 0: Cross-Section Computation
*************************************

A cross-section computation for a specific material, energy and process

.. code-block:: console

  $ python cross_sections.py [-h] [-d DEVICE] [-m MATERIAL] -p [PROCESS]-e [ENERGY] [-v VERBOSE]
  -h/--help           Printing help into the screen
  -d/--device         Setting OpenCL id
  -m/--material       Setting one of material defined in GGEMS (Water, Air, ...)
  -p/--process        Setting photon physical process (Compton, Rayleigh, Photoelectric)
  -e/--energy         Setting photon energy in MeV
  -v/--verbose        Setting level of verbosity

Verbosity level is defined in the range [0;3]. For a silent GGEMS execution, the level is set to 0, otherwise 3 for lot of informations.

.. code-block:: python

  GGEMSVerbosity(verbosity_level)

  # Select a OpenCL device
  opencl_manager.set_device_index(device_id)

Create a GGEMSMaterial instance then add a material. The initialization step is mandatory and compute all physical tables, and store them on an OpenCL device:

.. code-block:: python

  materials = GGEMSMaterials()
  materials.add_material(material_name)
  materials.initialize()

Create a GGEMSCrossSection instance and activate a process:

.. code-block:: python

  cross_sections = GGEMSCrossSections(materials)
  cross_sections.add_process(process_name, 'gamma')
  cross_sections.initialize()

For attenuation informations, create a GGEMSAttenuations:

.. code-block:: python

  attenuations = GGEMSAttenuations(materials, cross_sections)
  attenuations.initialize();

Computing cross section value (in cm2.g-1) for a specific energy (in MeV):

.. code-block:: python

  cross_sections.get_cs(process_name, material_name, energy_MeV, 'MeV')

  # Get attenuation value in cm-1
  attenuations.get_mu(material_name, energy_MeV, 'MeV')
  # Get energy attenuation value
  attenuations.get_mu_en(material_name, energy_MeV, 'MeV')
