*****
World
*****

Outside the navigator, particles are not tracked. GGEMS includes a tool to record photons exiting the navigator. Particles are projected into the world using a DDA algorithm.

The world module is GGEMSWorld:

.. code-block:: python

  world = GGEMSWorld()

After creating a GGEMSWorld, the world dimensions and voxel size can be defined:

.. code-block:: python

  world.set_dimensions(200, 200, 200)
  world.set_element_sizes(10.0, 10.0, 10.0, 'mm')

For world output, the user can save various information such as: photon energy and squared energy through voxel, photon momentum, and fluence (photon tracking):

.. code-block:: python

  world.set_output_basename('data/world')
  world.energy_tracking(True)
  world.energy_squared_tracking(True)
  world.momentum(True)
  world.photon_tracking(True)
