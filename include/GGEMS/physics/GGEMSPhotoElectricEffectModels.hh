#ifndef GUARD_GGEMS_PHYSICS_GGEMSPHOTOELECTRICEFFECTMODELS_HH
#define GUARD_GGEMS_PHYSICS_GGEMSPHOTOELECTRICEFFECTMODELS_HH

// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
  \file GGEMSPhotoElectricEffectModels.hh

  \brief Models for PhotoElectric effect, only for OpenCL kernel usage

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday September 17, 2020
*/

#ifdef OPENCL_COMPILER

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline void StandardPhotoElectricSampleSecondaries(__global GGEMSPrimaryParticles* primary_particle, GGint const index_particle)
  \param primary_particle - buffer of particles
  \param index_particle - index of the particle
  \brief Standard Photoelectric model
*/
inline void StandardPhotoElectricSampleSecondaries(
  __global GGEMSPrimaryParticles* primary_particle,
  GGint const index_particle
)
{
  primary_particle->status_[index_particle] = DEAD;
}

#endif

#endif // GUARD_GGEMS_PHYSICS_GGEMSCOMPTONSCATTERINGMODELS_HH
