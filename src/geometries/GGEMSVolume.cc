/*!
  \file GGEMSVolume.cc

  \brief Mother class handle volume

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday January 13, 2020
*/

#include "GGEMS/geometries/GGEMSVolume.hh"
#include "GGEMS/tools/GGEMSSystemOfUnits.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVolume::GGEMSVolume(void)
: label_value_(1.0f),
  positions_(GGfloat3{{0.0f, 0.0f, 0.0f}}),
  kernel_draw_volume_(nullptr)
{
  GGcout("GGEMSVolume", "GGEMSVolume", 3) << "Allocation of GGEMSVolume..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVolume::~GGEMSVolume(void)
{
  GGcout("GGEMSVolume", "~GGEMSVolume", 3) << "Deallocation of GGEMSVolume..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolume::SetLabelValue(GGfloat const& label_value)
{
  label_value_ = label_value;
}

void GGEMSVolume::SetMaterial(char const* material)
{
  // Get the volume creator manager
  GGEMSVolumeCreatorManager& volume_creator_manager = GGEMSVolumeCreatorManager::GetInstance();

  // Adding the material to phantom creator manager
  volume_creator_manager.AddLabelAndMaterial(label_value_, material);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolume::SetPosition(GGfloat const& pos_x, GGfloat const& pos_y, GGfloat const& pos_z, char const* unit)
{
  positions_.s[0] = GGEMSUnits::DistanceUnit(pos_x, unit);
  positions_.s[1] = GGEMSUnits::DistanceUnit(pos_y, unit);
  positions_.s[2] = GGEMSUnits::DistanceUnit(pos_z, unit);
}