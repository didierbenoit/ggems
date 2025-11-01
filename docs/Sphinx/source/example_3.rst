***************************************
Examples 3: Voxelized Phantom Generator
***************************************

A voxelized phantom can be created using GGEMS. Only basic shapes are available such as tube, box and sphere. Output format is MHD, and a range material data is also generated.

.. code-block:: console

  $ python generate_volume.py [-h] [-d DEVICE] [-v VERBOSE]
  -h/--help           Printing help into the screen
  -d/--device         Setting OpenCL id
  -v/--verbose        Setting level of verbosity

Create a global volume storing all voxelized objets:

.. code-block:: python

  volume_creator_manager = GGEMSVolumeCreatorManager()

  volume_creator_manager.set_dimensions(450, 450, 450)
  volume_creator_manager.set_element_sizes(0.5, 0.5, 0.5, "mm")
  volume_creator_manager.set_output('data/volume')
  volume_creator_manager.set_range_output('data/range_volume')
  volume_creator_manager.set_material('Air')
  volume_creator_manager.set_data_type('MET_INT')
  volume_creator_manager.initialize()

Draw an object in previous global volume:

.. code-block:: python

  # Creating a box
  box = GGEMSBox(24.0, 36.0, 56.0, 'mm')
  box.set_position(-70.0, -30.0, 10.0, 'mm')
  box.set_label_value(1)
  box.set_material('Water')
  box.initialize()
  box.draw()
  box.delete()

  # Creating a tube
  tube = GGEMSTube(13.0, 8.0, 50.0, 'mm')
  tube.set_position(20.0, 10.0, -2.0, 'mm')
  tube.set_label_value(2)
  tube.set_material('Calcium')
  tube.initialize()
  tube.draw()
  tube.delete()

  # Creating a sphere
  sphere = GGEMSSphere(14.0, 'mm')
  sphere.set_position(30.0, -30.0, 8.0, 'mm')
  sphere.set_label_value(3)
  sphere.set_material('Lung')
  sphere.initialize()
  sphere.draw()
  sphere.delete()
