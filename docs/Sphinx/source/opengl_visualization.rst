********************
OpenGL Visualization
********************

In GGEMS, user-defined volumes and sources can be visualized. OpenGL can be enabled through the GGEMS macro. Several colors are already predefined, such as:

  * black
  * blue
  * lime
  * cyan
  * red
  * magenta
  * yellow
  * white
  * gray
  * silver
  * maroon
  * olive
  * green
  * purple
  * teal
  * navy

.. IMPORTANT::

  OpenGL settings must be initialized at the beginning of the simulation. All volumes and all sources must be declared only after this initialization.

.. code-block:: python

  from ggems import *

  opengl_manager = GGEMSOpenGLManager()

  # Multisample anti-aliasing, can be 1, 2, 4 or 8
  opengl_manager.set_msaa(8)
  # Background color for OpenGL window
  opengl_manager.set_background_color('black')
  # Draw axis X, Y and Z
  opengl_manager.set_draw_axis(True)
  # Output folder storing OpenGL images you want to save (*.png format)
  opengl_manager.set_image_output('axis')
  # Number of maximum particles drawn in the OpenGL viewport (max: 65536)
  opengl_manager.set_displayed_particles(128)
  # Change the color of photons, either using a color defined in GGEMS or using RGB indices
  opengl_manager.set_particle_color('gamma', 152, 251, 152)
  # opengl_manager.set_particle_color('gamma', color_name='red')
  # Set the size of your world surrounding your simulation. It is important that this is large enough because the field of view of the OpenGL viewport depends on this value. 
  opengl_manager.set_world_size(3.0, 3.0, 3.0, 'm')
  # Size of OpenGL viewport
  opengl_manager.set_window_dimensions(500, 500)
  # Initialize OpenGL
  opengl_manager.initialize()
  # Show OpenGL window outside GGEMS simulation
  opengl_manager.display()

If you have defined a navigator (phantom or detector) it is also possible to change the color of the material either by using a defined color or by using RGB indices. It is also possible to disable the color.

.. code-block:: python

  # Supposing you defined a phantom before composed by air and water
  phantom.set_material_visible('Air', True) # or False if you do not want to draw the air voxels
  phantom.set_material_color('Water', color_name='blue')

  # Supposing you define a CBCT before composed by GOS
  cbct_detector.set_material_color('GOS', 255, 0, 0) # Custom color using RGB
  #cbct_detector.set_material_color('GOS', color_name='red') # Using registered color
