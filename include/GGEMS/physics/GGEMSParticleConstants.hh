#ifndef GUARD_GGEMS_PHYSICS_GGEMSPARTICLECONSTANTS_HH
#define GUARD_GGEMS_PHYSICS_GGEMSPARTICLECONSTANTS_HH

/*!
  \file GGEMSParticleConstants.hh

  \brief Storing particle states for GGEMS

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday April 13, 2020
*/

#include "GGEMS/tools/GGEMSSystemOfUnits.hh"

__constant GGuchar PRIMARY = 0; /*!< Primary particle */
__constant GGuchar GEOMETRY_BOUNDARY = 99; /*!< Particle on the boundary */
__constant GGuchar ALIVE = 0; /*!< Particle alive */
__constant GGuchar DEAD = 1; /*!< Particle dead */
__constant GGuchar FREEZE = 2; /*!< Particle freeze */
__constant GGfloat OUT_OF_WORLD = FLT_MAX; /*!< Particle out of world */

__constant GGuchar PHOTON = 0; /*!< Photon particle */
__constant GGuchar ELECTRON = 1; /*!< Electron particle */
__constant GGuchar POSITRON = 2; /*!< Positron particle */

#endif // End of GUARD_GGEMS_PHYSICS_GGEMSPARTICLECONSTANTS_HH