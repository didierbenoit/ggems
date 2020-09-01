#ifndef GUARD_GGEMS_PHYSICS_GGEMSPROCESSCONSTANTS_HH
#define GUARD_GGEMS_PHYSICS_GGEMSPROCESSCONSTANTS_HH

/*!
  \file GGEMSEMProcessConstants.hh

  \brief Storing some constant variables for process

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday April 13, 2020
*/

#include "GGEMS/tools/GGEMSSystemOfUnits.hh"

__constant GGuchar NUMBER_PROCESSES = 3; /*!< Maximum number of processes */

__constant GGuchar NO_PROCESS = 99; /*!< No process */
__constant GGuchar TRANSPORTATION = 99; /*!< Transportation process */

// PHOTON PROCESSES
#define NUMBER_PHOTON_PROCESSES 3 // Number of photon processes
__constant GGuchar COMPTON_SCATTERING = 0; /*!< Compton process */
__constant GGuchar PHOTOELECTRIC_EFFECT = 1; /*!< Photoelectric process */
__constant GGuchar RAYLEIGH_SCATTERING = 2; /*!< Rayleigh process */

//__constant GGuchar NUMBER_ELECTRON_PROCESSES = 3; /*!< Maximum number of electron processes */
//__constant GGuchar NUMBER_PARTICLES = 5; /*!< Maximum number of different particles for secondaries */
//__constant GGuchar PHOTON_BONDARY_VOXEL = 77; /*!< Photon on the boundaries */
//__constant GGuchar ELECTRON_IONISATION = 4; /*!< Electron ionisation process */
//__constant GGuchar ELECTRON_MSC = 5; /*!< Electron multiple scattering process */
//__constant GGuchar ELECTRON_BREMSSTRAHLUNG = 6; /*!< Bremsstralung electron process */

// CROSS SECTIONS
__constant GGfloat KINETIC_ENERGY_MIN = 1.e-6f; /*!< Min kinetic energy, 1eV */
__constant GGfloat CROSS_SECTION_TABLE_ENERGY_MIN = 990.0f*1.e-6f; /*!< Min energy in the cross section table, 990 eV */
__constant GGfloat CROSS_SECTION_TABLE_ENERGY_MAX = 250.0f*1.0f; /*!< Max energy in the cross section table, 250 MeV */
#define MAX_CROSS_SECTION_TABLE_NUMBER_BINS 2048
__constant GGushort CROSS_SECTION_TABLE_NUMBER_BINS = 220; /*!< Number of bins in the cross section table */

// CUTS
__constant GGfloat PHOTON_DISTANCE_CUT = 1.e-3f; /*!< Photon cut, 1 um */
__constant GGfloat ELECTRON_DISTANCE_CUT = 1.e-3f; /*!< Electron cut, 1 um */
__constant GGfloat POSITRON_DISTANCE_CUT = 1.e-3f; /*!< Positron cut, 1 um */

#endif // End of GUARD_GGEMS_PHYSICS_GGEMSEMPROCESSCONSTANTS_HH