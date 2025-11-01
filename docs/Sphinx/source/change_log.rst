**********
Change Log
**********

CMake
=====
* Deleting CMake for C++ examples

GGEMS
=====
* New kind of energy source, now a source can be defined using discrete peak energy
* Validation of GGEMS using OpenCL from CUDA 12.5 or 12.6
* Installation using setuptools
* New GGEMSMeshPhantom class for photon navigation through meshed navigator
* New GGEMSMeshedSolid and GGEMSMeshedSolidData class to store and handle geometric infos about meshed volume
* New GGEMSVoxelizedSource class

Fixed Bugs
==========
* Bug in Rayleigh scattering process

Features
========
* Meshed navigator
* Voxelized source

Examples
========
* New example 7_Mesh declaring a meshed navigator
* New example 8_Voxelized_Source declaring a voxelized source associated to a voxelized phantom
