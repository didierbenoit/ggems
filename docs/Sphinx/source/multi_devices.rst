*************
Multi-Devices
*************

GGEMS can be used on any platform compatible with OpenCL 3.0. Each GPU or CPU is recognised as a device. There are different ways to activate a device.

* Using the device index (assumed to be known by the user):

.. code-block:: python

  from ggems import *

  opencl_manager = GGEMSOpenCLManager()
  opencl_manager.set_device_index(0) # Activate device id 0
  opencl_manager.set_device_index(2) # Activate device id 2, if it exists

  exit()

* Explicitly indicate the device(s):

.. code-block:: python

  from ggems import *

  opencl_manager = GGEMSOpenCLManager()
  opencl_manager.set_device_to_activate('gpu', 'nvidia') # Activate all NVIDIA GPU only
  opencl_manager.set_device_to_activate('gpu', 'intel') # Activate all Intel GPU only
  opencl_manager.set_device_to_activate('cpu') # Activate cpu only
  opencl_manager.set_device_to_activate('all') # Activate all found devices
  opencl_manager.set_device_to_activate('0;2') # Activate devices 0 and 2

  exit()
