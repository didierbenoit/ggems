*********
Dosimetry
*********

The dosimetry module can be activated to compute absorbed dose. Currently, only photons are simulated; electrons are ignored.

Dosimetry module is 'GGEMSDosimetryCalculator':

.. code-block:: python

  dosimetry = GGEMSDosimetryCalculator()

A navigator phantom must be attached to GGEMSDosimetryCalculator using:

.. code-block:: python

  dosimetry.attach_to_navigator('phantom')

Size of voxel (dosel) for dosimetry image could be set, if not, dosel size is the same than voxel phantom size:

.. code-block:: python

  dosimetry.set_dosel_size(0.5, 0.5, 0.5, 'mm')

Absorbed dose is computed in gray (Gy). By default, dose in computed using materials in phantom, otherwize the user can set water material everywhere in phantom.

.. code-block:: python

  dosimetry.water_reference(True)

A custom density threshold can be set. If density of phantom is below this threshold the dose value in 0 Gy.

.. code-block:: python

  dosimetry.minimum_density(0.1, 'g/cm3')

Since GGEMS v1.2, TLE method (Track Length Estimated) proposed in [Baldacci2014] can be activated to improve statistics.

.. code-block:: python

  dosimetry.set_tle(is_tle)

Many informations can be registered such as: uncertainty values of the dose, the deposited energy in dosel, the squared of deposited energy in dosel and the number of interactions (hits) in dosel:

.. code-block:: python

  dosimetry.set_output('dosimetry')
  dosimetry.uncertainty(True)
  dosimetry.edep(True)
  dosimetry.hit(True)
  dosimetry.edep_squared(True)

There is a special output named 'photon tracking'. This output registers the number of photons crossing a dosel. To use this option, the size of dosel has to be the same than the phantom voxel size, otherwize GGEMS will throw an error:

.. code-block:: python

  dosimetry.photon_tracking(True)


* F. Baldacci, A. Mittone, A. Bravin, P. Coan, F. Delaire, C. Ferrero, S. Gasilov, J. M. Létang, D. Sarrut, F. Smekens, et al., “A track length estimator method for dose calculations in low-energy x-ray irradiations: implementation, properties and performance”, Zeitschrift Fur Medizinische Physik, 2014.
