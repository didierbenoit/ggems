*******************************
Physical Processes & Range cuts
*******************************

Physical Processes
==================

The photon processes impletemented are:

  * Compton scattering
  * Photoelectric effect
  * Rayleigh scattering

Each of these processes are extracted from Geant4 version 10.6. For more informations about physics, please read the documentation on the Geant4 website.

Processes can be accessed with the following command:

.. code-block:: python

  processes_manager = GGEMSProcessesManager()

.. IMPORTANT::

  Secondary particles (photon and electron) are not simulated yet. For Photoelectric effect, the photon is killed during the interaction and the energy is locally deposited, fluorescence photon is not emitted.

Compton Scattering
------------------

The Geant4 model extracted is the 'G4KleinNishinaCompton' standard model. Compton scattering is activated for all the navigators, or for a specific navigator.

.. code-block:: python

  # Activating Compton process for all navigators
  processes_manager.add_process('Compton', 'gamma', 'all')

  # Activating Compton process for a specific navigator name 'my_phantom'
  processes_manager.add_process('Compton', 'gamma', 'my_phantom')

Photoelectric Effect
--------------------

The Geant4 model extracted is the 'G4PhotoElectricEffect' standard model using Sandia tables. Photoelectric effect is activated for all the navigators, or for a specific navigator.

.. code-block:: python

  # Activating Photoelectric process for all navigators
  processes_manager.add_process('Photoelectric', 'gamma', 'all')

  # Activating Photoelectric process for a specific navigator name 'my_phantom'
  processes_manager.add_process('Photoelectric', 'gamma', 'my_phantom')

Rayleigh Scattering
-------------------

The Geant4 model extracted is the 'G4LivermoreRayleighModel' livermore model. Rayleigh scattering is activated for all the navigators, or for a specific navigator.

.. code-block:: python

  # Activating Rayleigh process for all navigators
  processes_manager.add_process('Rayleigh', 'gamma', 'all')

  # Activating Rayleigh process for a specific navigator name 'my_phantom'
  processes_manager.add_process('Rayleigh', 'gamma', 'my_phantom')

Process Parameters Building
---------------------------

Cross sections are calculated during GGEMS initialization. The parameters for calculating the cross sections can be modified, but it is recommended to use the default ones. The parameters that can be modified are:

  * Minimum energy of cross-section table
  * Maximum energy of cross-section table
  * Number of bins in cross-section table

Default parameters are defined as following:

.. code-block:: python

  processes_manager.set_cross_section_table_number_of_bins(220)
  processes_manager.set_cross_section_table_energy_min(1.0, 'keV')
  processes_manager.set_cross_section_table_energy_max(1.0, 'MeV')

Process Verbosity
-----------------

Some informations about processes can be printed:

  * Available processes
  * Global informations about processes
  * Cross-section values

The list of commands are:

.. code-block:: python

  processes_manager.print_available_processes()
  processes_manager.print_infos()
  processes_manager.print_tables(True)

Range Cuts
==========

Cuts are defined for each particle in distance unit in all navigators or a specific navigator. During initialization cuts are converted in energy for each material in navigator. If the particle energy is below the cut, then this one is killed and the energy is locally deposited. By default cuts are 1 micron.

.. code-block:: python

  # For photon and all navigators
  range_cuts_manager.set_cut('gamma', 0.1, 'mm', 'all')

  # For photon and a specific navigator named 'my_phantom'
  range_cuts_manager.set_cut('gamma', 0.1, 'mm', 'my_phantom')
