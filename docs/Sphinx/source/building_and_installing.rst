*********************
Building & Installing
*********************

Prerequisites
=============
GGEMS is based on the OpenCL library. For each platform (NVIDIA, Intel and AMD), you must install the corresponding drivers.

.. warning::
   GGEMS has not been validated with AMD. Useful AMD drivers could be found `here`_.

.. _here: https://www.amd.com/en/support/download/drivers.html

NVIDIA
------
To use GGEMS on an NVIDIA platform, simply install `CUDA`_ (version 12.6 is recommended) along with the corresponding driver.

.. _CUDA: https://developer.nvidia.com/cuda-12-6-2-download-archive

.. important::
  The CUDA library is not used by GGEMS. Only the OpenCL library provided with CUDA is used. It's recommended to install CUDA from NVIDIA website. For linux user, installing cuda using the 'apt' program for instance is not recommended. If the NVIDIA driver does not match the correct CUDA version, GGEMS may not work.

INTEL
-----
To use GGEMS on an Intel platform (CPU or GPU) you must install the driver. For this, it is recommended to install the driver provided by Intel `oneAPI`_ Base Toolkit. The library will be installed with the Intel compiler and the other libraries.

.. _oneAPI: https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit.html

OpenGL visualization
--------------------
Since GGEMS v1.2, the OpenGL library can be used to visualize a simulation in 3D space. OpenGL can be used on any OS. 3 libraries have to be installed to use OpenGL correctly:

* `GLFW`_ : an Open Source and multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. It provides a simple API for creating windows, contexts and surfaces, receiving input and events.

* `GLEW`_ : a cross-platform open-source C/C++ extension loading library.

* `GLM`_ : a C++ header for mathematics library based on the OpenGL Shading Language (GLSL) specifications.

.. _GLFW: https://www.glfw.org
.. _GLEW: http://glew.sourceforge.net
.. _GLM: https://github.com/g-truc/glm

.. important::
  For linux users, GLEW library must be installed from `source`_ (glew-XXX.zip). It's mandatory to link GGEMS and libGLEW.a static library.

.. _source: http://glew.sourceforge.net

.. important::
  For linux users, GLFW and GLM libraries can be installed using the 'apt' program for example.

.. WARNING::
  For Windows users the libraries should be downloaded from their respective website and installed if possible in the standard location C:\\Program Files (x86)

GGEMS Installation
==================
To install GGEMS, `CMake`_ and `setuptools`_ (a python project combined with CMake) are required.

.. _CMake: https://cmake.org
.. _setuptools: https://pypi.org/project/setuptools

In the following section, a recommended installation procedure is provided as an example. First clone the GGEMS project:

.. code-block:: console

  $ git clone https://github.com/GGEMS/ggems.git

Enter the directory ggems and launch the installation command (set opengl to OFF to deactivate it):

.. code-block:: console

  $ cd ggems
  $ python setup.py build_ext --opengl=ON install --user

.. NOTE::
  On Windows OS, multi-processor compilation is possible using `Ninja`_, and running the command:

  .. code-block:: console
    
    $ python setup.py build_ext --generator=Ninja --opengl=ON install --user

.. _Ninja: https://ninja-build.org

GGEMS is now installed on your machine. To check the installation, you can try the examples or launch GGEMS in a Python console.

.. code-block:: python

  from ggems import *
  opencl_manager = GGEMSOpenCLManager()
  opencl_manager.print_infos()
  exit()

