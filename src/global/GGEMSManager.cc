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
  \file GGEMSManager.cc

  \brief GGEMS class managing the GGEMS simulation

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday September 30, 2019
*/

#include <algorithm>
#include <fcntl.h>

#ifdef _WIN32
#ifdef _MSC_VER
#define NOMINMAX
#endif
#include <windows.h>
#include <wincrypt.h>
#else
#include <unistd.h>
#endif

#include "GGEMS/global/GGEMSManager.hh"

#include "GGEMS/physics/GGEMSProcessesManager.hh"
#include "GGEMS/physics/GGEMSRangeCutsManager.hh"

#include "GGEMS/sources/GGEMSSourceManager.hh"

#include "GGEMS/navigators/GGEMSNavigatorManager.hh"

#include "GGEMS/tools/GGEMSChrono.hh"
#include "GGEMS/tools/GGEMSRAMManager.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSManager::GGEMSManager(void)
: seed_(0),
  is_opencl_verbose_(false),
  is_material_database_verbose_(false),
  is_source_verbose_(false),
  is_navigator_verbose_(false),
  is_memory_ram_verbose_(false),
  is_process_verbose_(false),
  is_range_cuts_verbose_(false),
  is_random_verbose_(false),
  is_tracking_verbose_(false),
  is_kernel_verbose_(false),
  particle_tracking_id_(0)
{
  GGcout("GGEMSManager", "GGEMSManager", 3) << "Allocation of GGEMS Manager..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSManager::~GGEMSManager(void)
{
  GGcout("GGEMSManager", "~GGEMSManager", 3) << "Deallocation of GGEMS Manager..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetSeed(uint32_t const& seed)
{
  seed_ = seed;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGuint GGEMSManager::GenerateSeed(void) const
{
  #ifdef _WIN32
  HCRYPTPROV seedWin32;
  if (CryptAcquireContext(&seedWin32, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) == FALSE) {
    std::ostringstream oss(std::ostringstream::out);
    char buffer_error[256];
    oss << "Error finding a seed: " << strerror_s(buffer_error, 256, errno) << std::endl;
    GGEMSMisc::ThrowException("GGEMSManager", "GenerateSeed", oss.str());
  }
  return static_cast<uint32_t>(seedWin32);
  #else
  // Open a system random file
  GGint file_descriptor = ::open("/dev/urandom", O_RDONLY | O_NONBLOCK);
  if (file_descriptor < 0) {
    std::ostringstream oss( std::ostringstream::out );
    oss << "Error opening the file '/dev/urandom': " << strerror(errno) << std::endl;
    GGEMSMisc::ThrowException("GGEMSManager", "GenerateSeed", oss.str());
  }

  // Buffer storing 8 characters
  char seedArray[sizeof(GGuint)];
  ::read(file_descriptor, reinterpret_cast<GGuint*>(seedArray), sizeof(GGuint));
  ::close(file_descriptor);
  GGuint *seedUInt = reinterpret_cast<GGuint*>(seedArray);
  return *seedUInt;
  #endif
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetOpenCLVerbose(bool const& is_opencl_verbose)
{
  is_opencl_verbose_ = is_opencl_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetMaterialDatabaseVerbose(bool const& is_material_database_verbose)
{
  is_material_database_verbose_ = is_material_database_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetSourceVerbose(bool const& is_source_verbose)
{
  is_source_verbose_ = is_source_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetNavigatorVerbose(bool const& is_navigator_verbose)
{
  is_navigator_verbose_ = is_navigator_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetMemoryRAMVerbose(bool const& is_memory_ram_verbose)
{
  is_memory_ram_verbose_ = is_memory_ram_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetProcessVerbose(bool const& is_process_verbose)
{
  is_process_verbose_ = is_process_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetKernelVerbose(bool const& is_kernel_verbose)
{
  is_kernel_verbose_ = is_kernel_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetRangeCutsVerbose(bool const& is_range_cuts_verbose)
{
  is_range_cuts_verbose_ = is_range_cuts_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetRandomVerbose(bool const& is_random_verbose)
{
  is_random_verbose_ = is_random_verbose;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetTrackingVerbose(bool const& is_tracking_verbose, GGint const& particle_tracking_id)
{
  is_tracking_verbose_ = is_tracking_verbose;
  particle_tracking_id_ = particle_tracking_id;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::CheckParameters(void)
{
  GGcout("GGEMSManager", "CheckParameters", 1) << "Checking the mandatory parameters..." << GGendl;

  // Checking the seed of the random generator
  if (seed_ == 0) seed_ = GenerateSeed();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::Initialize(void)
{
  GGcout("GGEMSManager", "Initialize", 1) << "Initialization of GGEMS Manager singleton..." << GGendl;

  // Getting the GGEMS singletons
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();
  GGEMSMaterialsDatabaseManager& material_database_manager = GGEMSMaterialsDatabaseManager::GetInstance();
  GGEMSSourceManager& source_manager = GGEMSSourceManager::GetInstance();
  GGEMSNavigatorManager& navigator_manager = GGEMSNavigatorManager::GetInstance();
  GGEMSProcessesManager& processes_manager = GGEMSProcessesManager::GetInstance();
  GGEMSRangeCutsManager& range_cuts_manager = GGEMSRangeCutsManager::GetInstance();
  GGEMSRAMManager& ram_manager = GGEMSRAMManager::GetInstance();

  // Checking if a context is activated
  if (!opencl_manager.IsReady()) {
    GGEMSMisc::ThrowException("GGEMSManager", "Initialize", "OpenCL Manager is not ready, you have to choose a context!!!");
  }

  // Get the start time
  ChronoTime start_time = GGEMSChrono::Now();

  // Printing the banner with the GGEMS version
  PrintBanner();

  // Checking the mandatory parameters
  CheckParameters();
  GGcout("GGEMSManager", "Initialize", 0) << "Parameters OK" << GGendl;

  // Initialize the pseudo random number generator
  srand(seed_);
  GGcout("GGEMSManager", "Initialize", 0) << "C++ Pseudo-random number generator seeded OK" << GGendl;

  // Checking if material manager is ready
  if (!material_database_manager.IsReady()) GGEMSMisc::ThrowException("GGEMSManager", "Initialize", "Materials are not loaded in GGEMS!!!");

  // Initialization of the source
  source_manager.Initialize();

  // Initialization of the navigators (phantom + system)
  navigator_manager.Initialize();

  // Printing infos about OpenCL
  if (is_opencl_verbose_) {
    opencl_manager.PrintPlatformInfos();
    opencl_manager.PrintDeviceInfos();
    opencl_manager.PrintContextInfos();
    opencl_manager.PrintCommandQueueInfos();
    opencl_manager.PrintActivatedContextInfos();
    opencl_manager.PrintBuildOptions();
  }

  // Printing infos about material database
  if (is_material_database_verbose_) material_database_manager.PrintAvailableMaterials();

  // Printing infos about source(s)
  if (is_source_verbose_) source_manager.PrintInfos();

  // Printing infos about navigator(s)
  if (is_navigator_verbose_) navigator_manager.PrintInfos();

  // Printing infos about processe(s)
  if (is_process_verbose_) {
    processes_manager.PrintAvailableProcesses();
    processes_manager.PrintInfos();
  }

  // Printing infos about range cuts
  if (is_range_cuts_verbose_) range_cuts_manager.PrintInfos();

  // Printing infos about random in GGEMS
  if (is_random_verbose_) GGcout("GGEMSManager", "Initialize", 0) << "GGEMS Seed: " << seed_ << GGendl;

  // Printing infos about RAM
  if (is_memory_ram_verbose_) ram_manager.PrintRAMStatus();

  // Get the end time
  ChronoTime end_time = GGEMSChrono::Now();

  GGcout("GGEMSManager", "Initialize", 0) << "GGEMS initialization succeeded" << GGendl;

  // Display the elapsed time in GGEMS
  GGEMSChrono::DisplayTime(end_time - start_time, "GGEMS initialization");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::Run()
{
  GGcout("GGEMSManager", "Run", 0) << "GGEMS simulation started" << GGendl;

  GGEMSSourceManager& source_manager = GGEMSSourceManager::GetInstance();
  GGEMSNavigatorManager& navigator_manager = GGEMSNavigatorManager::GetInstance();

  ChronoTime start_time = GGEMSChrono::Now();

  for (std::size_t j = 0; j < source_manager.GetNumberOfSources(); ++j) {
    GGcout("GGEMSManager", "Run", 0) << "## Source " << source_manager.GetNameOfSource(j) << GGendl;

    for (std::size_t i = 0; i < source_manager.GetNumberOfBatchs(j); ++i) {
      GGcout("GGEMSManager", "Run", 1) << "----> Launching batch " << i+1 << "/" << source_manager.GetNumberOfBatchs(j) << GGendl;

      GGlong number_of_particles = source_manager.GetNumberOfParticlesInBatch(j, i);

      // Step 1: Generating primaries from source
      GGcout("GGEMSManager", "Run", 2) << "      + Generating " << number_of_particles << " particles..." << GGendl;
      source_manager.GetPrimaries(j, number_of_particles);

      // Loop until ALL particles are dead
      do {
        // Step 2: Find closest navigator (phantom and detector) before track to in operation
        GGcout("GGEMSManager", "Run", 2) << "      + Finding closest solid..." << GGendl;
        navigator_manager.FindClosestSolid();

        // Step 3: Project particles to closest solid
        GGcout("GGEMSManager", "Run", 2) << "      + Project particles to closest solid..." << GGendl;
        navigator_manager.ProjectToClosestSolid();

        // Step 4: Track to out step, particles are tracked in navigator
        GGcout("GGEMSManager", "Run", 2) << "      + Tracking particles through closest solid..." << GGendl;
        navigator_manager.TrackThroughClosestSolid();

      } while (source_manager.IsAlive()); // Step 5: Checking if all particles are dead, otherwize go back to step 2
    }
  }

  // Printing elapsed time in kernels
  if (is_kernel_verbose_) {
    source_manager.PrintKernelElapsedTime();
    navigator_manager.PrintKernelElapsedTime();
  }

  ChronoTime end_time = GGEMSChrono::Now();

  GGcout("GGEMSManager", "Run", 0) << "GGEMS simulation succeeded" << GGendl;

  GGEMSChrono::DisplayTime(end_time - start_time, "GGEMS simulation");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::PrintBanner(void) const
{
  std::cout << std::endl;
  std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
  std::cout << "$  ___   ___   ___  __ __  ___          _  $" << std::endl;
  std::cout << "$ /  _> /  _> | __>|  \\  \\/ __>    _ _ / | $" << std::endl;
  std::cout << "$ | <_/\\| <_/\\| _> |     |\\__ \\   | | || | $" << std::endl;
  std::cout << "$ `____/`____/|___>|_|_|_|<___/   |__/ |_| $" << std::endl;
  std::cout << "$                                          $" << std::endl;
  std::cout << "$ Welcome to GGEMS v1.0   https://ggems.fr $" << std::endl;
  std::cout << "$ Copyright (c) GGEMS Team 2020            $" << std::endl;
  std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
  std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSManager* get_instance_ggems_manager(void)
{
  return &GGEMSManager::GetInstance();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_seed_ggems_manager(GGEMSManager* ggems_manager, GGuint const seed)
{
  ggems_manager->SetSeed(seed);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void initialize_ggems_manager(GGEMSManager* ggems_manager)
{
  ggems_manager->Initialize();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_opencl_verbose_ggems_manager(GGEMSManager* ggems_manager, bool const is_opencl_verbose)
{
  ggems_manager->SetOpenCLVerbose(is_opencl_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_material_database_verbose_ggems_manager(GGEMSManager* ggems_manager, bool const is_material_database_verbose)
{
  ggems_manager->SetMaterialDatabaseVerbose(is_material_database_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_source_ggems_manager(GGEMSManager* ggems_manager, bool const is_source_verbose)
{
  ggems_manager->SetSourceVerbose(is_source_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_navigator_ggems_manager(GGEMSManager* ggems_manager, bool const is_navigator_verbose)
{
  ggems_manager->SetNavigatorVerbose(is_navigator_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_memory_ram_ggems_manager(GGEMSManager* ggems_manager, bool const is_memory_ram_verbose)
{
  ggems_manager->SetMemoryRAMVerbose(is_memory_ram_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_process_ggems_manager(GGEMSManager* ggems_manager, bool const is_process_verbose)
{
  ggems_manager->SetProcessVerbose(is_process_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_range_cuts_ggems_manager(GGEMSManager* ggems_manager, bool const is_range_cuts_verbose)
{
  ggems_manager->SetRangeCutsVerbose(is_range_cuts_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_random_ggems_manager(GGEMSManager* ggems_manager, bool const is_random_verbose)
{
  ggems_manager->SetRandomVerbose(is_random_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_kernel_ggems_manager(GGEMSManager* ggems_manager, bool const is_kernel_verbose)
{
  ggems_manager->SetKernelVerbose(is_kernel_verbose);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_tracking_ggems_manager(GGEMSManager* ggems_manager, bool const is_tracking_verbose, GGint const particle_id_tracking)
{
  ggems_manager->SetTrackingVerbose(is_tracking_verbose, particle_id_tracking);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void run_ggems_manager(GGEMSManager* ggems_manager)
{
  ggems_manager->Run();
}
