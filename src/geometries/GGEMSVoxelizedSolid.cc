/*!
  \file GGEMSVoxelizedSolid.cc

  \brief GGEMS class for voxelized solid

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Wednesday June 10, 2020
*/

#include "GGEMS/geometries/GGEMSVoxelizedSolid.hh"
#include "GGEMS/io/GGEMSMHDImage.hh"
#include "GGEMS/sources/GGEMSSourceManager.hh"
#include "GGEMS/physics/GGEMSParticles.hh"
#include "GGEMS/physics/GGEMSCrossSections.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVoxelizedSolid::GGEMSVoxelizedSolid(std::string const& volume_header_filename, std::string const& range_filename)
: GGEMSSolid(),
  volume_header_filename_(volume_header_filename),
  range_filename_(range_filename)
{
  GGcout("GGEMSVoxelizedSolid", "GGEMSVoxelizedSolid", 3) << "Allocation of GGEMSVoxelizedSolid..." << GGendl;

  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get the RAM manager
  GGEMSRAMManager& ram_manager = GGEMSRAMManager::GetInstance();

  // Allocation of memory on OpenCL device for header data
  solid_data_cl_ = opencl_manager.Allocate(nullptr, sizeof(GGEMSVoxelizedSolidData), CL_MEM_READ_WRITE);
  ram_manager.AddGeometryRAMMemory(sizeof(GGEMSVoxelizedSolidData));

  // Initializing kernels
  InitializeKernel();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVoxelizedSolid::~GGEMSVoxelizedSolid(void)
{
  GGcout("GGEMSVoxelizedSolid", "~GGEMSVoxelizedSolid", 3) << "Deallocation of GGEMSVoxelizedSolid..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::InitializeKernel(void)
{
  GGcout("GGEMSVoxelizedSolid", "InitializeKernel", 3) << "Initializing kernel for voxelized solid..." << GGendl;

  // Getting OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Getting the path to kernel
  std::string const kOpenCLKernelPath = OPENCL_KERNEL_PATH;
  std::string const kFilename1 = kOpenCLKernelPath + "/DistanceVoxelizedSolid.cl";
  std::string const kFilename2 = kOpenCLKernelPath + "/ProjectToVoxelizedSolid.cl";
  std::string const kFilename3 = kOpenCLKernelPath + "/TrackThroughVoxelizedSolid.cl";

  // Compiling the kernels
  kernel_distance_cl_ = opencl_manager.CompileKernel(kFilename1, "distance_voxelized_solid");
  kernel_project_to_cl_ = opencl_manager.CompileKernel(kFilename2, "project_to_voxelized_solid");
  kernel_track_through_cl_ = opencl_manager.CompileKernel(kFilename3, "track_through_voxelized_solid");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::Initialize(std::shared_ptr<GGEMSMaterials> materials)
{
  GGcout("GGEMSVoxelizedSolid", "Initialize", 3) << "Initializing voxelized solid..." << GGendl;

  LoadVolumeImage(materials);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::SetPosition(GGfloat3 const& position_xyz)
{
  GGcout("GGEMSVoxelizedSolid", "SetPosition", 3) << "Setting position of voxelized solid..." << GGendl;

  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get pointer on OpenCL device
  GGEMSVoxelizedSolidData* solid_data_device = opencl_manager.GetDeviceBuffer<GGEMSVoxelizedSolidData>(solid_data_cl_.get(), sizeof(GGEMSVoxelizedSolidData));

  for (GGuint i = 0; i < 3; ++i ) {
    // Offset
    solid_data_device->position_xyz_.s[i] = position_xyz.s[i];

    // Bounding box
    solid_data_device->border_min_xyz_.s[i] = -solid_data_device->position_xyz_.s[i];
    solid_data_device->border_max_xyz_.s[i] = solid_data_device->border_min_xyz_.s[i] + solid_data_device->number_of_voxels_xyz_.s[i] * solid_data_device->voxel_sizes_xyz_.s[i];
  }

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::SetGeometryTolerance(GGfloat const& tolerance)
{
  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get pointer on OpenCL device
  GGEMSVoxelizedSolidData* solid_data_device = opencl_manager.GetDeviceBuffer<GGEMSVoxelizedSolidData>(solid_data_cl_.get(), sizeof(GGEMSVoxelizedSolidData));

  // Storing the geometry tolerance
  solid_data_device->tolerance_ = tolerance;

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::SetNavigatorID(std::size_t const& navigator_id)
{
  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get pointer on OpenCL device
  GGEMSVoxelizedSolidData* solid_data_device = opencl_manager.GetDeviceBuffer<GGEMSVoxelizedSolidData>(solid_data_cl_.get(), sizeof(GGEMSVoxelizedSolidData));

  // Storing the geometry tolerance
  solid_data_device->navigator_id_ = static_cast<GGuchar>(navigator_id);

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::PrintInfos(void) const
{
  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get pointer on OpenCL device
  GGEMSVoxelizedSolidData* solid_data_device = opencl_manager.GetDeviceBuffer<GGEMSVoxelizedSolidData>(solid_data_cl_.get(), sizeof(GGEMSVoxelizedSolidData));

  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "GGEMSVoxelizedSolid Infos:" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "--------------------------" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "*Dimension: " << solid_data_device->number_of_voxels_xyz_.s[0] << " " << solid_data_device->number_of_voxels_xyz_.s[1] << " " << solid_data_device->number_of_voxels_xyz_.s[2] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "*Number of voxels: " << solid_data_device->number_of_voxels_ << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "*Size of voxels: (" << solid_data_device->voxel_sizes_xyz_.s[0] << "x" << solid_data_device->voxel_sizes_xyz_.s[1] << "x" << solid_data_device->voxel_sizes_xyz_.s[2] << ") mm3" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "*Position: (" << solid_data_device->position_xyz_.s[0] << "x" << solid_data_device->position_xyz_.s[1] << "x" << solid_data_device->position_xyz_.s[2] << ") mm3" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "*Bounding box:" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    - X: " << solid_data_device->border_min_xyz_.s[0] << " <-> " << solid_data_device->border_max_xyz_.s[0] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    - Y: " << solid_data_device->border_min_xyz_.s[1] << " <-> " << solid_data_device->border_max_xyz_.s[1] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    - Z: " << solid_data_device->border_min_xyz_.s[2] << " <-> " << solid_data_device->border_max_xyz_.s[2] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "*Geometry tolerance: " << solid_data_device->tolerance_ << " mm" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "*Navigator index: " << static_cast<GGint>(solid_data_device->navigator_id_) << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << GGendl;

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::Distance(void)
{
  // Getting the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();
  cl::CommandQueue* queue_cl = opencl_manager.GetCommandQueue();
  cl::Event* event_cl = opencl_manager.GetEvent();

  // Getting the buffer of primary particles from source
  GGEMSSourceManager& source_manager = GGEMSSourceManager::GetInstance();
  GGEMSParticles* particles = source_manager.GetParticles();
  cl::Buffer* primary_particles_cl = particles->GetPrimaryParticles();

  // Getting the number of particles
  GGulong const kNumberOfParticles = particles->GetNumberOfParticles();

  // Set parameters for kernel
  std::shared_ptr<cl::Kernel> kernel_cl = kernel_distance_cl_.lock();
  kernel_cl->setArg(0, *primary_particles_cl);
  kernel_cl->setArg(1, *solid_data_cl_);

  // Define the number of work-item to launch
  cl::NDRange global(kNumberOfParticles);
  cl::NDRange offset(0);

  // Launching kernel
  cl_int kernel_status = queue_cl->enqueueNDRangeKernel(*kernel_cl, offset, global, cl::NullRange, nullptr, event_cl);
  opencl_manager.CheckOpenCLError(kernel_status, "GGEMSVoxelizedSolid", "Distance");
  queue_cl->finish(); // Wait until the kernel status is finish
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::ProjectTo(void)
{
  // Getting the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();
  cl::CommandQueue* queue_cl = opencl_manager.GetCommandQueue();
  cl::Event* event_cl = opencl_manager.GetEvent();

  // Getting the buffer of primary particles from source
  GGEMSSourceManager& source_manager = GGEMSSourceManager::GetInstance();
  GGEMSParticles* particles = source_manager.GetParticles();
  cl::Buffer* primary_particles_cl = particles->GetPrimaryParticles();

  // Getting the number of particles
  GGulong const kNumberOfParticles = particles->GetNumberOfParticles();

  // Set parameters for kernel
  std::shared_ptr<cl::Kernel> kernel_cl = kernel_project_to_cl_.lock();
  kernel_cl->setArg(0, *primary_particles_cl);
  kernel_cl->setArg(1, *solid_data_cl_);

  // Define the number of work-item to launch
  cl::NDRange global(kNumberOfParticles);
  cl::NDRange offset(0);

  // Launching kernel
  cl_int kernel_status = queue_cl->enqueueNDRangeKernel(*kernel_cl, offset, global, cl::NullRange, nullptr, event_cl);
  opencl_manager.CheckOpenCLError(kernel_status, "GGEMSVoxelizedSolid", "ProjectTo");
  queue_cl->finish(); // Wait until the kernel status is finish
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::TrackThrough(std::weak_ptr<GGEMSCrossSections> cross_sections, std::weak_ptr<GGEMSMaterials> materials)
{
  // Getting the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();
  cl::CommandQueue* queue_cl = opencl_manager.GetCommandQueue();
  cl::Event* event_cl = opencl_manager.GetEvent();

  // Getting the buffer of primary particles from source
  GGEMSSourceManager& source_manager = GGEMSSourceManager::GetInstance();
  GGEMSParticles* particles = source_manager.GetParticles();
  cl::Buffer* primary_particles_cl = particles->GetPrimaryParticles();

  // Getting OpenCL buffer for cross section
  cl::Buffer* cross_sections_cl = cross_sections.lock()->GetCrossSections();

  // Getting OpenCL buffer for materials
  cl::Buffer* materials_cl = materials.lock()->GetMaterialTables().lock().get();

  // Getting the number of particles
  GGulong const kNumberOfParticles = particles->GetNumberOfParticles();

  // Get pointer on OpenCL device
  GGEMSMaterialTables* material_device = opencl_manager.GetDeviceBuffer<GGEMSMaterialTables>(materials_cl, sizeof(GGEMSMaterialTables));

  std::cout << "CHECK: " << (int)material_device->number_of_materials_ << std::endl;
  std::cout << "CHECK n elements: " << material_device->total_number_of_chemical_elements_ << std::endl;
  std::cout << "CHECK density: " << material_device->density_of_material_[0] << std::endl;

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(materials_cl, material_device);

  // Set parameters for kernel
  std::shared_ptr<cl::Kernel> kernel_cl = kernel_track_through_cl_.lock();
  kernel_cl->setArg(0, *primary_particles_cl);
  kernel_cl->setArg(1, *solid_data_cl_);
  kernel_cl->setArg(2, *label_data_cl_);
  kernel_cl->setArg(3, *cross_sections_cl);
  kernel_cl->setArg(4, *materials_cl);

  // Define the number of work-item to launch
  cl::NDRange global(kNumberOfParticles);
  cl::NDRange offset(0);

  // Launching kernel
  cl_int kernel_status = queue_cl->enqueueNDRangeKernel(*kernel_cl, offset, global, cl::NullRange, nullptr, event_cl);
  opencl_manager.CheckOpenCLError(kernel_status, "GGEMSVoxelizedSolid", "TrackThrough");
  queue_cl->finish(); // Wait until the kernel status is finish
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::LoadVolumeImage(std::weak_ptr<GGEMSMaterials> materials)
{
  GGcout("GGEMSVoxelizedSolid", "LoadVolumeImage", 3) << "Loading volume image from mhd file..." << GGendl;

  // Read MHD input file
  GGEMSMHDImage mhd_input_phantom;
  mhd_input_phantom.Read(volume_header_filename_, solid_data_cl_);

  // Get the name of raw file from mhd reader
  std::string const kRawFilename = mhd_input_phantom.GetRawMDHfilename();

  // Get the type
  std::string const kDataType = mhd_input_phantom.GetDataMHDType();

  // Convert raw data to material id data
  if (!kDataType.compare("MET_CHAR")) {
    ConvertImageToLabel<char>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_UCHAR")) {
    ConvertImageToLabel<unsigned char>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_SHORT")) {
    ConvertImageToLabel<GGshort>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_USHORT")) {
    ConvertImageToLabel<GGushort>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_INT")) {
    ConvertImageToLabel<GGint>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_UINT")) {
    ConvertImageToLabel<GGuint>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_FLOAT")) {
    ConvertImageToLabel<GGfloat>(kRawFilename, range_filename_, materials);
  }
}