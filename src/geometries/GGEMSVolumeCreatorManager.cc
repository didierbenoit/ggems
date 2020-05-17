/*!
  \file GGEMSVolumeCreatorManager.cc

  \brief Singleton class generating voxelized volume from analytical volume

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Thursday January 9, 2020
*/

#include <algorithm>

#include "GGEMS/geometries/GGEMSVolumeCreatorManager.hh"
#include "GGEMS/tools/GGEMSPrint.hh"
#include "GGEMS/tools/GGEMSTools.hh"
#include "GGEMS/tools/GGEMSSystemOfUnits.hh"
#include "GGEMS/io/GGEMSMHDImage.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVolumeCreatorManager::GGEMSVolumeCreatorManager(void)
: element_sizes_(GGfloat3{{0.0, 0.0, 0.0}}),
  volume_dimensions_(GGuint3{{0, 0, 0}}),
  number_elements_(0),
  data_type_("MET_FLOAT"),
  material_("Air"),
  output_image_filename_(""),
  output_range_to_material_filename_(""),
  voxelized_volume_(nullptr)
{
  GGcout("GGEMSVolumeCreatorManager", "GGEMSVolumeCreatorManager", 3) << "Allocation of Phantom Creator Manager singleton..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVolumeCreatorManager::~GGEMSVolumeCreatorManager(void)
{
  GGcout("GGEMSVolumeCreatorManager", "~GGEMSVolumeCreatorManager", 3) << "Deallocation of Phantom Creator Manager singleton..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::SetElementSizes(GGfloat const& voxel_width, GGfloat const& voxel_height, GGfloat const& voxel_depth, char const* unit)
{
  element_sizes_.s[0] = GGEMSUnits::DistanceUnit(voxel_width, unit);
  element_sizes_.s[1] = GGEMSUnits::DistanceUnit(voxel_height, unit);
  element_sizes_.s[2] = GGEMSUnits::DistanceUnit(voxel_depth, unit);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::SetVolumeDimensions(GGuint const& volume_width, GGuint const& volume_height, GGuint const& volume_depth)
{
  volume_dimensions_.s[0] = volume_width;
  volume_dimensions_.s[1] = volume_height;
  volume_dimensions_.s[2] = volume_depth;
  number_elements_ = volume_width * volume_height * volume_depth;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::SetMaterial(char const* material)
{
  material_ = material;

  // Store the material in map
  label_to_material_.insert(std::make_pair(0.0f, material_));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::SetDataType(std::string const& data_type)
{
  data_type_ = data_type;

  // Convert raw data to material id data
  if (data_type_.compare("MET_CHAR") && data_type_.compare("MET_UCHAR") && data_type_.compare("MET_SHORT") && data_type_.compare("MET_USHORT") && data_type_.compare("MET_INT") && data_type_.compare("MET_UINT") && data_type_.compare("MET_FLOAT")) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "Your type in not compatible. The type has to be:" << std::endl;
    oss << "    - MET_CHAR" << std::endl;
    oss << "    - MET_UCHAR" << std::endl;
    oss << "    - MET_SHORT" << std::endl;
    oss << "    - MET_USHORT" << std::endl;
    oss << "    - MET_INT" << std::endl;
    oss << "    - MET_UINT" << std::endl;
    oss << "    - MET_FLOAT" << std::endl;
    GGEMSMisc::ThrowException("GGEMSVolumeCreatorManager", "SetDataType", oss.str());
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::AddLabelAndMaterial(GGfloat const& label, std::string const& material)
{
  GGcout("GGEMSVolumeCreatorManager", "AddLabelAndMaterial", 3) << "Adding new material and label..." << GGendl;

  // Insert label and check if the label exists already
  auto const [iter, success] = label_to_material_.insert(std::make_pair(label, material));
  if (!success) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "The label: " << iter->first << " already exists...";
    GGEMSMisc::ThrowException("GGEMSVolumeCreatorManager", "AddLabelAndMaterial", oss.str());
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::SetOutputImageFilename(char const* output_image_filename)
{
  output_image_filename_ = output_image_filename;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::SetRangeToMaterialDataFilename(char const* output_range_to_material_filename)
{
  output_range_to_material_filename_ = output_range_to_material_filename;

  // Adding suffix
  output_range_to_material_filename_ += ".txt";
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::CheckParameters(void) const
{
  GGcout("GGEMSVolumeCreatorManager", "CheckParameters", 3) << "Checking parameters for phantom creator manager..." << GGendl;

  // Checking phantom dimensions
  if (volume_dimensions_.s[0] == 0 && volume_dimensions_.s[1] == 0 && volume_dimensions_.s[2] == 0) {
    GGEMSMisc::ThrowException("GGEMSVolumeCreatorManager", "CheckParameters", "Phantom dimensions have to be > 0!!!");
  }

  // Checking size of voxels
  if (GGEMSMisc::IsEqual(element_sizes_.s[0], 0.0f) && GGEMSMisc::IsEqual(element_sizes_.s[1], 0.0f) && GGEMSMisc::IsEqual(element_sizes_.s[2], 0.0f)) {
    GGEMSMisc::ThrowException("GGEMSVolumeCreatorManager", "CheckParameters", "Phantom voxel sizes have to be > 0.0!!!");
    }

  // Checking output name
  if (output_image_filename_.empty()) {
    GGEMSMisc::ThrowException("GGEMSVolumeCreatorManager", "CheckParameters", "A output image filename has to be done to phantom manager!!!");
  }

  // Checking range to material data name
  if (output_range_to_material_filename_.empty()) {
    GGEMSMisc::ThrowException("GGEMSVolumeCreatorManager", "CheckParameters", "A output range to material data filename has to be done to phantom manager!!!");
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::Initialize(void)
{
  GGcout("GGEMSVolumeCreatorManager", "Initialize", 3) << "Initializing phantom creator manager..." << GGendl;

  // Check mandatory parameters
  CheckParameters();

  if (!data_type_.compare("MET_CHAR")) AllocateImage<char>();
  else if (!data_type_.compare("MET_UCHAR")) AllocateImage<unsigned char>();
  else if (!data_type_.compare("MET_SHORT")) AllocateImage<GGshort>();
  else if (!data_type_.compare("MET_USHORT")) AllocateImage<GGushort>();
  else if (!data_type_.compare("MET_INT")) AllocateImage<GGint>();
  else if (!data_type_.compare("MET_UINT")) AllocateImage<GGuint>();
  else if (!data_type_.compare("MET_FLOAT")) AllocateImage<GGfloat>();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::Write(void)
{
  // Writing output image
  WriteMHDImage();

  // Writing the range to material file
  WriteRangeToMaterialFile();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::WriteRangeToMaterialFile(void)
{
  GGcout("GGEMSVolumeCreatorManager", "WriteRangeToMaterialFile", 3) << "Writing range to material text file..." << GGendl;

  GGcout("GGEMSVolumeCreatorManager", "WriteRangeToMaterialFile", 0) << "List of label and material:" << GGendl;
  for(auto&& i : label_to_material_) {
    GGcout("GGEMSVolumeCreatorManager", "WriteRangeToMaterialFile", 0) << "    * Material: " << i.second << ", label: " << i.first << GGendl;
  }

  // Write file
  std::ofstream range_to_data_stream(output_range_to_material_filename_, std::ios::out);
  for(auto&& i : label_to_material_) {
    range_to_data_stream << i.first << " " << i.first << " " << i.second << std::endl;
  }
  range_to_data_stream.close();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVolumeCreatorManager::WriteMHDImage(void) const
{
  GGcout("GGEMSVolumeCreatorManager", "WriteMHDImage", 3) << "Writing MHD output file..." << GGendl;

  // Write MHD file
  GGEMSMHDImage mhdImage;
  mhdImage.SetBaseName(output_image_filename_);
  mhdImage.SetDataType(data_type_);
  mhdImage.SetDimensions(volume_dimensions_);
  mhdImage.SetElementSizes(element_sizes_);
  mhdImage.Write(voxelized_volume_);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVolumeCreatorManager* get_instance_volume_creator_manager(void)
{
  return &GGEMSVolumeCreatorManager::GetInstance();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_volume_dimension_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager, GGuint const volume_width, GGuint const volume_height, GGuint const volume_depth)
{
  volume_creator_manager->SetVolumeDimensions(volume_width, volume_height, volume_depth);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_element_sizes_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager, GGfloat const voxel_width, GGfloat const voxel_height, GGfloat const voxel_depth, char const* unit)
{
  volume_creator_manager->SetElementSizes(voxel_width, voxel_height, voxel_depth, unit);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_output_image_filename_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager, char const* output_image_filename)
{
  volume_creator_manager->SetOutputImageFilename(output_image_filename);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_output_range_to_material_filename_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager,char const* output_range_to_material_filename)
{
  volume_creator_manager->SetRangeToMaterialDataFilename(output_range_to_material_filename);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void initialize_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager)
{
  volume_creator_manager->Initialize();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void write_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager)
{
  volume_creator_manager->Write();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_material_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager, char const* material)
{
  volume_creator_manager->SetMaterial(material);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_data_type_volume_creator_manager(GGEMSVolumeCreatorManager* volume_creator_manager, char const* data_type)
{
  volume_creator_manager->SetDataType(data_type);
}