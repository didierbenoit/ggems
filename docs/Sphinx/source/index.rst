.. GGEMS documentation master file, created by
   sphinx-quickstart on Tue Sep 30 09:41:35 2025.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the GGEMS Documentation
==================================
GGEMS (GPU Geant4-based Monte Carlo Simulations) is a Monte Carlo numerical simulation platform dedicated to medical applications such as CT/CBCT imaging and hadron therapy. The physical models from the `Geant4`_ platform, which is a toolkit for simulating the interaction of particles through matter, are implemented in GGEMS using the `OpenCL`_ library. This approach enables parallel computing on heterogeneous architectures such as CPUs and GPUs. The core of the GGEMS library is fully written in C++, including the components that integrate OpenCL and OpenGL (for visualisation). However, the library can only be used through commands written in Python.

.. _Geant4: https://geant4.web.cern.ch
.. _OpenCL: https://www.khronos.org/opencl

This documentation is mainly for users running CT/CBCT imaging simulations or photon dosimetry. It also provides guidance for those who want to understand how GGEMS works so they can integrate their own code.

This documentation is divided into three parts:

First, an introduction to GGEMS and the informations are given in order to install your GGEMS environment.

Second, informations about all GGEMS potentials are given. Examples and tools are also illustrated and explained. Command lines are listed using python instructions.

And finally, a more detailed description concerning the GGEMS implementation for advanced user. The goal is to provide sufficient information on the implementation of GGEMS so that any user who wishes can integrate their own code.

.. toctree::
   :maxdepth: 1
   :caption: Preamble

   introduction
   requirements
   building_and_installing

.. toctree::
   :maxdepth: 1
   :caption: User Documentation

   documentation
   multi_devices
   opengl_visualization
   world
   navigators
   ct_cbct_system
   dosimetry
   processes_cuts
   sources
   ggems_commands
   examples_and_tools

.. toctree::
   :maxdepth: 1
   :caption: Developer Documentation

   ggems_design
   executable_program
   release_notes
   change_log

.. toctree::
   :maxdepth: 1
   :caption: Printable Document

   ggems_pdf
