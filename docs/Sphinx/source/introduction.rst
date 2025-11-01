************
Introduction
************

GGEMS is an advanced Monte Carlo simulation platform using the OpenCL library managing CPU and GPU architecture. GGEMS is fully developed in C++ and accessible via Python command line.

Well-validated `Geant4`_ physic models are used in GGEMS and implemented using OpenCL.

The aim of GGEMS is to provide a fast simulation platform for imaging application (CT/CBCT for moment) and particle therapy. To favor speed of computation, GGEMS is not a very generic platform as `Geant4`_ or `GATE`_. For very realistic simulation with lot of information results, Geant4 and GATE are still recommended.

GGEMS features:

* Photon particle tracking
* Multithreaded CPU
* GPU
* Multi devices (GPUs+CPU) approach
* Single or double float precision for dosimetry application
* External X-ray source
* Voxelized source
* Navigation in simple box volume, voxelized volume or meshed volume
* Flat or curved detector for CBCT/CT application
* Visualisation using OpenGL

GGEMS medical applications:

* CT/CBCT imaging (standard, dual-energy)
* External radiotherapy (IMRT and VMAT)
* Portal imaging from LINAC system

In the next GGEMS releases, the aim is to implement the following applications and features:

* Positron particle tracking
* Electron particle tracking
* PET imaging
* SPECT imaging
* Intra-operative radiotherapy (brachytherapy and intrabeam)

.. _Geant4: https://geant4.web.cern.ch
.. _GATE: http://www.opengatecollaboration.org
