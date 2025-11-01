************
Requirements
************

GGEMS is a multi architecture and multi platform application using `OpenCL`_.

.. _OpenCL: https://www.khronos.org/opencl

Operating Systems
=================
* Linux (Ubuntu 24.04 LTS, other system should work)
* Windows 10 & 11

.. warning::
   GGEMS has not been validated on macOS. Only minor modifications to the source code may be required.

C++ Compiler Version
====================
* Linux: GNU v13.3 and Clang v18.1.3
* Windows:

  * Visual C++ v19.44.35217 for x64 from Visual Studio Community 2022
  * Visual C++ v19.50.35503 for x64 from Visual Studio Community 2026

Python Version
==============
GGEMS supports the following versions

* Python 3.6+

OpenCL Version
==============
GGEMS validated using the following version

* OpenCL 3.0

.. important::
   GGEMS version 1.3 is only compatible with OpenCL 3.0. Ensure that this version is installed on your system. OpenCL 3.0 is compatible with `NVIDIA`_ and `Intel`_. 

.. _NVIDIA: https://developer.nvidia.com/blog/nvidia-is-now-opencl-3-0-conformant
.. _Intel: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-cpu-runtime-for-opencl-applications-guide.html#:~:text=Intel®%20CPU%20Runtime%20for%20OpenCL™%20Applications%20is%20an,standard%20specification%20and%20most%20extensions.

Hardware
========
GGEMS can be used with lots of different hardwares such as CPU, GPU and Intel HD Graphics

* Intel (CPU + HD Graphics)
* NVIDIA

.. warning::
   GGEMS has not been validated with AMD hardwares (CPU and GPU). Do not hesitate to contact the GGEMS developers if you have AMD hardware and encounter installation or operational issues with GGEMS.

