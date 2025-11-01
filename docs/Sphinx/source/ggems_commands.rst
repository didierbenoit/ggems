**************
GGEMS Commands
**************

Useful commands to manage GGEMS and verbosity

.. code-block:: python

  # Create an GGEMS instance
  ggems = GGEMS()
  # Infos about OpenCL environment
  ggems.opencl_verbose(True)
  # Infos about whole material database
  ggems.material_database_verbose(True)
  # Infos about navigator (system/phantom)
  ggems.navigator_verbose(True)
  # Infos about source
  ggems.source_verbose(True)
  # Infos about memory usage
  ggems.memory_verbose(True)
  # Infos about activated processe(s)
  ggems.process_verbose(True)
  # Infos about range cuts
  ggems.range_cuts_verbose(True)
  # Print infos about first random state and initial seed
  ggems.random_verbose(True)
  # Infos about elapsed time in kernels
  ggems.profiling_verbose(True)
  # Infos about a specific photon index (here photon index 12)
  ggems.tracking_verbose(True, 12)

Important commands to initialize and run a GGEMS application:

.. code-block:: python

  # Initialization with a specific seed (here 9383) or let GGEMS get a random seed
  ggems.initialize(777)
  # ggems.initialize() # random seed
  # Running ggems
  ggems.run()
